#pragma once
#include <cmath>
#include <iostream>
#include <string>
#include "TrafficSignal.h"

using namespace std;

enum WeatherCondition {
    WEATHER_CLEAR = 0,
    WEATHER_RAIN = 1,       // Monsoon downpours (slick pavement, hydroplaning)
    WEATHER_SMOG = 2,       // Lahore/Punjab winter smog (AQI 500+, reduced visibility)
    WEATHER_DENSE_FOG = 3   // M-2/M-3 Dense winter fog (visibility < 50m)
};

struct RoadDetails {
    float length, max_speed, capacity;
    int currentVehicles;
    int queueCount;
    bool signalState;
    float NonIdealWeight;
    float a, b;
    TrafficSignal light;
    int dischargeCapcity;

    // Phase 3 Realism: Incidents, Road Blocks & Weather
    bool isBlocked;
    float incidentPenalty;
    float capacityFactor; // 1.0 = normal, 0.5 = adverse weather/lane restriction, 0.0 = full block

    // Feature 5: Multi-Lane Motorway Configuration
    int numLanes;

    // Feature 7: M-Tag Electronic Toll Plazas
    bool hasTollPlaza;
    float tollRatePKR;
    int mTagVehiclesServed;
    int cashVehiclesServed;
    float totalTollCollectedPKR;

    // Feature 9: Pakistani Seasonal Weather Presets
    WeatherCondition weather;
    float frictionGrip;     // Grip coefficient: 1.0 (clear), 0.72 (rain), 0.85 (fog/smog)
    float weatherSpeedCap;  // Enforced advisory speed ceiling during adverse weather

    int getNumLanes() const { return numLanes; }
    void setNumLanes(int lanes) { numLanes = (lanes < 1) ? 1 : lanes; }

    void setTollPlaza(bool enabled, float rate = 120.0f) {
        hasTollPlaza = enabled;
        tollRatePKR = rate;
    }

    float processToll(bool hasMTag) {
        if (!hasTollPlaza) return 0.0f;
        totalTollCollectedPKR += tollRatePKR;
        if (hasMTag) {
            mTagVehiclesServed++;
            return 1.5f; // M-Tag RFID high-speed electronic barrier
        } else {
            cashVehiclesServed++;
            return 18.0f; // Manual cash collection & ticket change delay
        }
    }

    void setWeather(WeatherCondition w) {
        weather = w;
        if (w == WEATHER_CLEAR) {
            frictionGrip = 1.0f;
            capacityFactor = 1.0f;
            weatherSpeedCap = max_speed;
        } else if (w == WEATHER_RAIN) {
            frictionGrip = 0.72f;
            capacityFactor = 0.85f;
            weatherSpeedCap = max_speed * 0.80f;
        } else if (w == WEATHER_SMOG) {
            frictionGrip = 0.90f;
            capacityFactor = 0.75f;
            weatherSpeedCap = max_speed * 0.65f;
        } else if (w == WEATHER_DENSE_FOG) {
            frictionGrip = 0.85f;
            capacityFactor = 0.50f;
            weatherSpeedCap = max_speed * 0.45f;
        }
        NonIdealtime();
    }

    float getEffectiveMaxSpeed() const {
        if (weatherSpeedCap > 0.0f && weatherSpeedCap < max_speed) return weatherSpeedCap;
        return (max_speed > 0.0f) ? max_speed : 1.0f;
    }

    bool operator<(const RoadDetails& other) const {
        return this->calculateWeight() < other.calculateWeight();
    }

    void blockRoad() {
        isBlocked = true;
        capacityFactor = 0.0f;
    }

    void unblockRoad() {
        isBlocked = false;
        capacityFactor = 1.0f;
        incidentPenalty = 0.0f;
    }

    void setIncident(float penalty = 50.0f) {
        incidentPenalty = penalty;
    }

    void clearIncident() {
        incidentPenalty = 0.0f;
    }

    void setCapacityFactor(float factor) {
        capacityFactor = (factor < 0.0f) ? 0.0f : factor;
    }

    float calculateWeight() const {
        if (isBlocked) return 1e8f; // Heavy penalty diverts all dynamic rerouting
        float effSpeed = getEffectiveMaxSpeed();
        if (effSpeed <= 0) return 1000000;
        float effectiveCap = capacity * capacityFactor;
        if (effectiveCap <= 0) return 1e8f;
        float best = length / effSpeed;
        return best * (1.0f + a * pow((float)currentVehicles / effectiveCap, b)) + incidentPenalty;
    }

    RoadDetails(float ab = 100, float bc = 100, float c = 50, float aplha = 0.5, float beta = 4,
        int discharge = 5)
        : a(aplha), b(beta), dischargeCapcity(discharge) {
        currentVehicles = 0;
        length = ab;
        max_speed = bc;
        capacity = c;
        queueCount = 0;
        signalState = true;
        NonIdealWeight = 0;
        isBlocked = false;
        incidentPenalty = 0.0f;
        capacityFactor = 1.0f;
        numLanes = (c >= 80.0f) ? 3 : 2;
        hasTollPlaza = false;
        tollRatePKR = 120.0f;
        mTagVehiclesServed = 0;
        cashVehiclesServed = 0;
        totalTollCollectedPKR = 0.0f;
        weather = WEATHER_CLEAR;
        frictionGrip = 1.0f;
        weatherSpeedCap = bc;
    }

    float DischargeAllowed(int capacity_ofnext_road, int pop_of_next_road) {
        if (isBlocked || capacityFactor <= 0.0f) return 0.0f;

        float allowed = queueCount;
        float avail = capacity_ofnext_road - pop_of_next_road;
        if (avail > allowed) {
            if (dischargeCapcity <= allowed) {
                return signalState * dischargeCapcity;
            }
            else {
                return signalState * queueCount;
            }
        }
        else if (dischargeCapcity > avail) {
            return avail * signalState;
        }
        else {
            return dischargeCapcity * signalState;
        }
    }

    float TimeCal() {
        if (isBlocked) return 1e8f;
        if (capacity <= 0) {
            return bestTime();
        }
        NonIdealWeight = NonIdealtime();
        return NonIdealWeight;
    }

    float bestTime() const { 
        float effSpeed = getEffectiveMaxSpeed();
        if (effSpeed <= 0) return 1000000.0f;
        // Scale travel time to simulation seconds so vehicles take a visually pleasing, realistic duration:
        // A standard city block (e.g. 3km at 50km/h) takes ~22 simulation seconds (~3.6s real-time at 60 FPS)
        float travelTimeSec = (length / effSpeed) * 360.0f;
        if (travelTimeSec < 14.0f) travelTimeSec = 14.0f; // Minimum 14 simulation seconds (~2.3s at 60 FPS)
        if (travelTimeSec > 60.0f) travelTimeSec = 60.0f; // Maximum 60 simulation seconds
        return travelTimeSec;
    }

    float NonIdealtime() {
        if (isBlocked) return 1e8f;
        if (capacity <= 0) {
            return bestTime();
        }
        float effectiveCap = capacity * capacityFactor;
        if (effectiveCap <= 0) return 1e8f;
        NonIdealWeight = bestTime();
        NonIdealWeight =
            NonIdealWeight * (1.0f + a * pow((float)currentVehicles / effectiveCap, b)) + incidentPenalty;
        return NonIdealWeight;
    }

    void vehicleEntersRoad() {
        if (currentVehicles < capacity) {
            currentVehicles++;
            NonIdealtime();
        }
    }

    void vehicleExitsRoad() {
        if (currentVehicles > 0) {
            currentVehicles--;
            NonIdealtime();
        }
    }

    void updateVehicles() {
        if (queueCount < capacity) 
            queueCount++;
    }

    void vehicleJoinsQueue() {
        queueCount++;
        NonIdealtime();
    }

    void vehicleExitsQueue() {
        if (queueCount > 0) {
            queueCount--;
        }
    }

    void decVehicles() {
        if (queueCount > 0) 
            queueCount--;
    }

    float Congestion() const {
        if (isBlocked) return 1.0f;
        float effectiveCap = capacity * capacityFactor;
        if (effectiveCap <= 0) {
            return 1.0f;
        }
        return (float)currentVehicles / effectiveCap;
    }

    void Time(float time) { light.Timer(signalState, time); }
    void increment(float time) { light.Timer(signalState, time); }

    void changeState() {
        if (signalState) {
            light.turnRed(signalState);
        }
        else {
            light.turnGreen(signalState);
        }
    }

    void change_to_red() { light.turnRed(signalState); }
    void change_to_yellow() { light.turnYellow(signalState); }
    void change_to_green() { light.turnGreen(signalState); }

    float choosing() {
        float currentCost =
            (light.pa * queueCount) + (light.pb * pow(Congestion(), 2));
        if (isBlocked) currentCost += 10000.0f;
        return currentCost;
    }
};
