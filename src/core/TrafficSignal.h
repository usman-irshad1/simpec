#pragma once
#include "../simulation/CityPlanner.h"

enum SignalColor {
    SIGNAL_RED = 0,
    SIGNAL_YELLOW = 1,
    SIGNAL_GREEN = 2
};

enum SignalControlStrategy {
    STRATEGY_FIXED_WEBSTER = 0,
    STRATEGY_GA_OPTIMIZED = 1,
    STRATEGY_ACTUATED_GAPOUT = 2,
    STRATEGY_COORDINATED_GREENWAVE = 3,
    STRATEGY_Q_LEARNING_ADAPTIVE = 4
};

struct TrafficSignal {
    float greentimer;
    float minGreenTime;
    float queueThreshold;
    float maxGreenTime;
    float redtimer;
    float yellowtimer;
    float yellowDuration;
    float pa, pb;
    SignalColor color;
    bool emergencyPreempted;
    SignalControlStrategy strategy;
    float passageTime;
    float timeSinceLastDetection;
    float coordinatedOffset;
    float plannedGreenDuration;

    TrafficSignal() {
        greentimer = 0;
        minGreenTime = 2.0f; 
        queueThreshold = 3.0f;
        maxGreenTime = 8.0f;
        pa = 10.0f;
        pb = 7.0f; 
        redtimer = 0.0f;
        yellowtimer = 0.0f;
        yellowDuration = 1.0f; // Clearance duration
        color = SIGNAL_GREEN;
        emergencyPreempted = false;
        strategy = STRATEGY_GA_OPTIMIZED;
        passageTime = 2.5f;
        timeSinceLastDetection = 0.0f;
        coordinatedOffset = 0.0f;
        plannedGreenDuration = 6.0f;
        initQLearning();
    }

    void applyTimingPlan(const CitySignalTimingPlan& plan, int phaseIdx = 0) {
        minGreenTime = plan.minGreen;
        maxGreenTime = plan.maxGreen;
        yellowDuration = plan.yellowDuration;
        passageTime = plan.passageTime;
        coordinatedOffset = plan.coordinatedOffset;
        plannedGreenDuration = plan.greenSplits[phaseIdx % 4];
    }

    // =========================================================================
    // FEATURE 2: Q-LEARNING & MAX-PRESSURE ADAPTIVE CONTROLLER
    // =========================================================================
    float qTable[4][2]; // 4 queue states x 2 actions (0: Keep, 1: Switch)
    float learningRate;
    float discountFactor;
    float epsilon;
    int lastState;
    int lastAction;
    int lastQueue;

    void initQLearning() {
        learningRate = 0.12f;
        discountFactor = 0.90f;
        epsilon = 0.08f;
        lastState = 0;
        lastAction = 0;
        lastQueue = 0;
        for (int s = 0; s < 4; s++) {
            qTable[s][0] = 0.0f;
            qTable[s][1] = 0.0f;
        }
    }

    int discretizeQueueState(int q) const {
        if (q < 5) return 0;   // Low
        if (q < 15) return 1;  // Moderate
        if (q < 30) return 2;  // High
        return 3;              // Saturated
    }

    int selectQAction(int state, bool explore = true) {
        if (explore && ((float)(rand() % 1000) / 1000.0f) < epsilon) {
            return rand() % 2;
        }
        return (qTable[state][1] > qTable[state][0]) ? 1 : 0;
    }

    void updateQValue(int s, int a, float reward, int nextS) {
        float maxNext = (qTable[nextS][0] > qTable[nextS][1]) ? qTable[nextS][0] : qTable[nextS][1];
        float target = reward + discountFactor * maxNext;
        qTable[s][a] += learningRate * (target - qTable[s][a]);
    }

    bool shouldSwitchQAdaptive(int currentQueue, int maxOtherQueue, float downstreamPressure) {
        if (emergencyPreempted) return false;
        if (greentimer < minGreenTime) return false;
        if (mustSwitch()) return true;

        int state = discretizeQueueState(currentQueue);
        int action = selectQAction(state);

        float reward = (float)(lastQueue - currentQueue); // positive when queue clears
        if (downstreamPressure > 15.0f) reward -= 3.0f;  // penalty if downstream blocked
        updateQValue(lastState, lastAction, reward, state);

        lastState = state;
        lastAction = action;
        lastQueue = currentQueue;

        if (action == 1 && (currentQueue < maxOtherQueue || downstreamPressure < 0.0f || starvationCost() > 12.0f)) {
            return true;
        }

        return canSwitch(currentQueue, maxOtherQueue);
    }

    bool isGreen() const { return color == SIGNAL_GREEN; }
    bool isYellow() const { return color == SIGNAL_YELLOW; }
    bool isRed() const { return color == SIGNAL_RED; }

    void Timer(bool& state, float time) {
        if (color == SIGNAL_GREEN) {
            greentimer += time;
            redtimer = 0;
            yellowtimer = 0;
            state = true;
        }
        else if (color == SIGNAL_YELLOW) {
            yellowtimer += time;
            state = false;
            if (yellowtimer >= yellowDuration) {
                turnRed(state);
            }
        }
        else { // SIGNAL_RED
            redtimer += time;
            greentimer = 0;
            yellowtimer = 0;
            state = false;
        }
    }

    float starvationCost() const { return redtimer * 3.5f; }

    void turnGreen(bool& state) {
        color = SIGNAL_GREEN;
        state = true;
        greentimer = 0;
        yellowtimer = 0;
        emergencyPreempted = false;
    }

    void turnYellow(bool& state) {
        color = SIGNAL_YELLOW;
        state = false;
        yellowtimer = 0;
    }

    void turnRed(bool& state) {
        color = SIGNAL_RED;
        state = false;
        redtimer = 0;
        yellowtimer = 0;
        emergencyPreempted = false;
    }

    void triggerEmergencyPreemption(bool& state) {
        turnGreen(state);
        emergencyPreempted = true;
    }

    bool canSwitch(int currentQueue, int maxOtherQueue) const {
        if (emergencyPreempted)
            return false;

        if (greentimer < minGreenTime)
            return false;

        if (mustSwitch()) {
            return true;
        }

        if (maxOtherQueue <= 0) {
            // No competing vehicles waiting on other approaches; keep green if current approach has queue
            return (currentQueue == 0) && (starvationCost() > 12.0f);
        }

        float bottleneckRatio = (float)currentQueue / (float)maxOtherQueue;
        if (bottleneckRatio > 3.0f) {  
            return false;  
        }

        return (currentQueue < maxOtherQueue) || (starvationCost() > 12.0f);
    }

    bool mustSwitch() const { return greentimer >= maxGreenTime; }
};