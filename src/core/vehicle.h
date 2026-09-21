#pragma once
#include <list>
#include <vector>
#include <string>
#include <utility>

using namespace std;

enum VehicleState {
    VEHICLE_STATE_UNINITIALIZED = -1,
    VEHICLE_STATE_QUEUED = 0,
    VEHICLE_STATE_EN_ROUTE = 1,
    VEHICLE_STATE_ARRIVED = 2
};

enum VehicleType {
    VEHICLE_CAR = 0,
    VEHICLE_TRUCK = 1,
    VEHICLE_BUS = 2,
    VEHICLE_EMERGENCY = 3,
    VEHICLE_EV = 4,         // Feature 8: Electric Vehicle Fleet
    VEHICLE_MOTORCYCLE = 5  // Agile 2-Wheeler / Scooter
};

template <class t> struct vehicle {
    int id;
    t dest;
    t source;
    t current;
    list<t> path;
    pair<t, t> currentRoad;
    int state;
    bool inQueue; 
    vector<t> selected_path;
    float timespent, timeRemaining;
    float initialRoadTravelTime; // Used for smooth forward-only progress interpolation
    VehicleType type;
    float pcu;                  // Passenger Car Unit (space occupied on road)
    float speedMultiplier;      // Speed adjustment factor

    // =========================================================================
    // FEATURE 4: MICROSCOPIC IDM CAR-FOLLOWING & SHOCKWAVE PHYSICS
    // =========================================================================
    float currentSpeed;         // Instantaneous speed in m/s (e.g. 30 m/s ~ 108 km/h)
    float acceleration;         // Current acceleration in m/s^2
    float positionOnRoad;       // Longitudinal position on road in meters
    float desiredSpeed;         // Target cruising speed v_0 in m/s
    float idmTimeHeadway;       // Safe time headway T in seconds (default 1.5s)
    float idmMinDistance;       // Minimum bumper-to-bumper jam distance s_0 (2.5m)
    float idmMaxAccel;          // Comfortable acceleration a_max (1.5 m/s^2)
    float idmComfortDecel;      // Comfortable braking deceleration b (2.0 m/s^2)

    // =========================================================================
    // FEATURE 5: MULTI-LANE MOTORWAYS & MOBIL OVERTAKING MODEL
    // =========================================================================
    int currentLane;            // 0 = slow/truck lane, 1 = cruising, 2 = overtaking

    // =========================================================================
    // FEATURE 6: CARBON EMISSIONS & FUEL BURN TRACKER
    // =========================================================================
    float fuelConsumedLiters;
    float co2EmittedKg;
    float fuelWastedInJamLiters;

    // =========================================================================
    // FEATURE 7: M-TAG ELECTRONIC TOLL PLAZAS
    // =========================================================================
    bool hasMTag;
    float tollDelayPaid;

    // =========================================================================
    // FEATURE 8: ELECTRIC VEHICLE (EV) FLEET & BATTERY SoC
    // =========================================================================
    float batteryCapacityKWh;
    float batterySoCPercent;
    float electricConsumptionRateKWhPerKm;

    vehicle() {
        id = 0;
        dest = t();
        source = t();
        current = t();
        state = VEHICLE_STATE_UNINITIALIZED;
        inQueue = false; 
        timespent = timeRemaining = 0;
        initialRoadTravelTime = 0;
        currentRoad.first = currentRoad.second = t();
        type = VEHICLE_CAR;
        pcu = 1.0f;
        speedMultiplier = 1.0f;
        initMicroscopicPhysics();
        initEnergyAndToll();
    }
    vehicle(int i, t s, t d, VehicleType vType = VEHICLE_CAR) {
        id = i;
        dest = d;
        source = s;
        current = s;
        state = VEHICLE_STATE_QUEUED;
        inQueue = false; 
        timespent = timeRemaining = 0;
        initialRoadTravelTime = 0;
        currentRoad.first = currentRoad.second = t();
        initMicroscopicPhysics();
        initEnergyAndToll();
        setType(vType);
    }

    void initEnergyAndToll() {
        fuelConsumedLiters = 0.0f;
        co2EmittedKg = 0.0f;
        fuelWastedInJamLiters = 0.0f;
        hasMTag = ((rand() % 100) < 70); // 70% M-Tag penetration
        tollDelayPaid = 0.0f;
        batteryCapacityKWh = 65.0f;
        batterySoCPercent = 100.0f;
        electricConsumptionRateKWhPerKm = 0.18f;
    }

    void updateFuelAndEmissions(float dtSeconds, bool isIdleOrQueued, float speedMps = 0.0f) {
        if (type == VEHICLE_EV) {
            if (!isIdleOrQueued && speedMps > 0.1f) {
                float distKm = (speedMps * dtSeconds) / 1000.0f;
                float energyKWh = distKm * electricConsumptionRateKWhPerKm;
                batterySoCPercent -= (energyKWh / batteryCapacityKWh) * 100.0f;
                if (batterySoCPercent < 0.0f) batterySoCPercent = 0.0f;
            }
            return;
        }

        float idleRateLps = 0.00025f;  // ~0.9 L/h for cars
        float cruiseLpKm = 0.070f;     // 7 L / 100 km
        float co2Factor = 2.31f;       // Gasoline: 2.31 kg CO2/L

        if (type == VEHICLE_TRUCK) {
            idleRateLps = 0.000694f;   // ~2.5 L/h
            cruiseLpKm = 0.280f;       // 28 L / 100 km
            co2Factor = 2.68f;         // Diesel: 2.68 kg CO2/L
        } else if (type == VEHICLE_BUS) {
            idleRateLps = 0.000556f;   // ~2.0 L/h
            cruiseLpKm = 0.220f;       // 22 L / 100 km
            co2Factor = 2.68f;
        }

        float burned = 0.0f;
        if (isIdleOrQueued || speedMps < 0.5f) {
            burned = idleRateLps * dtSeconds;
            fuelWastedInJamLiters += burned;
        } else {
            float distKm = (speedMps * dtSeconds) / 1000.0f;
            burned = distKm * cruiseLpKm;
        }

        fuelConsumedLiters += burned;
        co2EmittedKg += burned * co2Factor;
    }

    void initMicroscopicPhysics() {
        currentSpeed = 25.0f;      // 90 km/h
        acceleration = 0.0f;
        positionOnRoad = 0.0f;
        desiredSpeed = 33.33f;     // 120 km/h free-flow speed
        idmTimeHeadway = 1.5f;     // 1.5s time gap
        idmMinDistance = 2.5f;     // 2.5m standstill distance
        idmMaxAccel = 1.5f;        // 1.5 m/s^2
        idmComfortDecel = 2.0f;    // 2.0 m/s^2
        currentLane = 1;           // Middle lane default
    }

    // Intelligent Driver Model (IDM) acceleration formula:
    // a(t) = a_max * [ 1 - (v / v_0)^4 - (s*(v, delta_v) / s)^2 ]
    float calculateIDMAcceleration(float distanceToLead, float leadSpeed) {
        float v = (currentSpeed < 0.0f) ? 0.0f : currentSpeed;
        float v0 = desiredSpeed * speedMultiplier;
        if (v0 < 0.1f) v0 = 0.1f;
        float deltaV = v - leadSpeed;

        float sStar = idmMinDistance + v * idmTimeHeadway + (v * deltaV) / (2.0f * sqrtf(idmMaxAccel * idmComfortDecel));
        if (sStar < idmMinDistance) sStar = idmMinDistance;

        float s = (distanceToLead < 0.5f) ? 0.5f : distanceToLead;

        float freeTerm = 1.0f - powf(v / v0, 4.0f);
        float interactionTerm = powf(sStar / s, 2.0f);
        float a = idmMaxAccel * (freeTerm - interactionTerm);
        if (a < -9.0f) a = -9.0f; // Physical emergency braking clamp
        return a;
    }

    // MOBIL (Minimizing Overall Braking Induced by Lane Changes) evaluation
    bool shouldChangeLaneMOBIL(float accelCurLane, float accelTargetLane, float accelNewFollower, float politeness = 0.3f, float threshold = 0.2f) {
        // Safety constraint: new follower must not be forced into emergency braking
        if (accelNewFollower < -4.0f) return false;
        // Incentive criterion
        float driverAdvantage = accelTargetLane - accelCurLane;
        if (driverAdvantage + politeness * accelNewFollower > threshold) {
            return true;
        }
        return false;
    }

    void setType(VehicleType vType) {
        type = vType;
        if (type == VEHICLE_TRUCK) {
            pcu = 2.5f;
            speedMultiplier = 0.8f;
        } else if (type == VEHICLE_BUS) {
            pcu = 2.0f;
            speedMultiplier = 0.9f;
        } else if (type == VEHICLE_EMERGENCY) {
            pcu = 1.2f;
            speedMultiplier = 1.35f;
        } else if (type == VEHICLE_EV) {
            pcu = 1.0f;
            speedMultiplier = 1.05f;
        } else if (type == VEHICLE_MOTORCYCLE) {
            pcu = 0.5f;
            speedMultiplier = 1.15f;
        } else {
            pcu = 1.0f;
            speedMultiplier = 1.0f;
        }
    }

    bool isQueued() const { return state == VEHICLE_STATE_QUEUED; }
    bool isEnRoute() const { return state == VEHICLE_STATE_EN_ROUTE; }
    bool hasArrived() const { return state == VEHICLE_STATE_ARRIVED; }
    bool isEmergency() const { return type == VEHICLE_EMERGENCY; }
    bool isTruck() const { return type == VEHICLE_TRUCK; }
    bool isBus() const { return type == VEHICLE_BUS; }
    bool isEV() const { return type == VEHICLE_EV; }
    bool isMotorcycle() const { return type == VEHICLE_MOTORCYCLE; }
    bool isLowBattery() const { return (type == VEHICLE_EV && batterySoCPercent < 25.0f); }
    void rechargeBattery(float amountPercent = 75.0f) {
        batterySoCPercent += amountPercent;
        if (batterySoCPercent > 100.0f) batterySoCPercent = 100.0f;
    }
    void payToll(float delay, float fee = 0.0f) {
        tollDelayPaid += delay;
        timespent += delay;
    }
    void setState(VehicleState s) { state = static_cast<int>(s); }

    string getTakenPath() {
        string path = "";
        if (selected_path.empty())
            return path;

        for (typename vector<t>::size_type i = 0; i < selected_path.size(); i++) {
            path = path + selected_path[i];

            if (i < selected_path.size() - 1) {
                path = path + " -> ";
            }
        }
        return path;
    }

    string getstart_and_end() { return source + "->" + dest; }
};
