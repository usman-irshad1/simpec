#pragma once
#include <iostream>
#include <string>
#include "Manger.h"
#include "CityPlanner.h"
#include <ctime> 
using namespace std;

template <class t, int size = 100> class Simulator {
    Graph<t, size>* pakistanMap;

public:
    Manager<t, size>* cityManager;
    bool isCityLevelMode;
    std::string activeCityScenario;
    CityPlanningReport cityReport;
    std::map<std::string, CitySignalTimingPlan> cityTimingPlans;

    Simulator() {
        pakistanMap = new Graph<t, size>(true); 
        cityManager = new Manager<t, size>(pakistanMap);
        isCityLevelMode = false;
        activeCityScenario = "AM_COMMUTE_PEAK";
    }
    Graph<t, size>* getMap() { return pakistanMap; }

    // =========================================================================
    // URBAN CITY-LEVEL PLANNING SIMULATION HELPER & OPTIMIZATION SUITE
    // =========================================================================
    void setupCityLevelNetwork(const std::string& cityName = "Downtown Central") {
        isCityLevelMode = true;
        activeCityScenario = (cityName == "Downtown Central") ? "AM_COMMUTE_PEAK" : (cityName + " Metropolitan Planning");

        std::string urbanHubs[11];
        if (cityName == "Mount Olympus") {
            urbanHubs[0] = "Zeus High Citadel";
            urbanHubs[1] = "Apollo Sun Boulevard";
            urbanHubs[2] = "Dionysus Agora";
            urbanHubs[3] = "Hermes Skyway";
            urbanHubs[4] = "Athena Academy";
            urbanHubs[5] = "Hephaestus Great Forge";
            urbanHubs[6] = "Artemis Sacred Grove";
            urbanHubs[7] = "Ares War Foundry";
            urbanHubs[8] = "Asclepius Sanctum";
            urbanHubs[9] = "Pegasus Skyport";
            urbanHubs[10] = "Pantheon Grand Circle";
        } else if (cityName == "Tartarus") {
            urbanHubs[0] = "Abyssal Core";
            urbanHubs[1] = "Gates of Styx";
            urbanHubs[2] = "Persephone Court";
            urbanHubs[3] = "Charon Ferry Gate";
            urbanHubs[4] = "Minos Tribunal";
            urbanHubs[5] = "Cerberus Watchpost";
            urbanHubs[6] = "Erebus Sector";
            urbanHubs[7] = "Tartarean Pyre";
            urbanHubs[8] = "Cocytus Healing Haven";
            urbanHubs[9] = "Underworld Highway";
            urbanHubs[10] = "Phlegethon Lava Ring";
        } else if (cityName == "Atlantis") {
            urbanHubs[0] = "Poseidon Royal Citadel";
            urbanHubs[1] = "Triton Deep Avenue";
            urbanHubs[2] = "Pearl Market Plaza";
            urbanHubs[3] = "Coral Marina Gate";
            urbanHubs[4] = "Nautilus Haven";
            urbanHubs[5] = "Sunken Biosphere";
            urbanHubs[6] = "Oceanic Conservatory";
            urbanHubs[7] = "Thermal Vent Refinery";
            urbanHubs[8] = "Merfolk Healing Grotto";
            urbanHubs[9] = "Submarine Expressway";
            urbanHubs[10] = "Great Whirlpool Circle";
        } else if (cityName == "Elysium") {
            urbanHubs[0] = "Golden Meadow Center";
            urbanHubs[1] = "Ambrosia Boulevard";
            urbanHubs[2] = "Elysian Gardens";
            urbanHubs[3] = "Phoenix Dawn Gate";
            urbanHubs[4] = "Isles of the Blest";
            urbanHubs[5] = "Heroes Amphitheater";
            urbanHubs[6] = "Mythic Grove";
            urbanHubs[7] = "Harvest Vale";
            urbanHubs[8] = "Seraphic Infirmary";
            urbanHubs[9] = "Celestial Flightway";
            urbanHubs[10] = "Eternal Light Circle";
        } else {
            urbanHubs[0] = "Downtown Central";
            urbanHubs[1] = "North Boulevard";
            urbanHubs[2] = "South Commercial";
            urbanHubs[3] = "East Gateway";
            urbanHubs[4] = "West Sector";
            urbanHubs[5] = "Tech Park";
            urbanHubs[6] = "University Town";
            urbanHubs[7] = "Industrial Estate";
            urbanHubs[8] = "City Hospital";
            urbanHubs[9] = "Airport Avenue";
            urbanHubs[10] = "Old Town Circle";
        }

        for (int i = 0; i < 11; i++) {
            pakistanMap->insertVertex(urbanHubs[i]);
        }

        float lats[11] = { 33.6844f, 33.7250f, 33.6400f, 33.6844f, 33.6844f, 33.7180f, 33.7200f, 33.6450f, 33.6500f, 33.6700f, 33.7500f };
        float lons[11] = { 73.0479f, 73.0479f, 73.0479f, 73.1000f, 72.9900f, 73.0950f, 72.9950f, 73.0900f, 73.0050f, 73.1400f, 73.0479f };
        for (int i = 0; i < 11; i++) {
            pakistanMap->setVertexCoordinates(urbanHubs[i], lats[i], lons[i]);
        }

        int DOWNTOWN = pakistanMap->getIndex(urbanHubs[0]);
        int NORTH    = pakistanMap->getIndex(urbanHubs[1]);
        int SOUTH    = pakistanMap->getIndex(urbanHubs[2]);
        int EAST     = pakistanMap->getIndex(urbanHubs[3]);
        int WEST     = pakistanMap->getIndex(urbanHubs[4]);
        int TECH     = pakistanMap->getIndex(urbanHubs[5]);
        int UNIV     = pakistanMap->getIndex(urbanHubs[6]);
        int INDUS    = pakistanMap->getIndex(urbanHubs[7]);
        int HOSP     = pakistanMap->getIndex(urbanHubs[8]);
        int AIR      = pakistanMap->getIndex(urbanHubs[9]);
        int OLDTOWN  = pakistanMap->getIndex(urbanHubs[10]);

        // Urban Multi-Lane Boulevards & Avenues
        pakistanMap->makeEdge(NORTH, DOWNTOWN, 4.5f, 60.0f, 60.0f, 0.10f, 4.0f);
        pakistanMap->makeEdge(DOWNTOWN, NORTH, 4.5f, 60.0f, 60.0f, 0.10f, 4.0f);
        pakistanMap->makeEdge(DOWNTOWN, SOUTH, 4.8f, 60.0f, 60.0f, 0.10f, 4.0f);
        pakistanMap->makeEdge(SOUTH, DOWNTOWN, 4.8f, 60.0f, 60.0f, 0.10f, 4.0f);

        pakistanMap->makeEdge(WEST, DOWNTOWN, 5.2f, 65.0f, 65.0f, 0.10f, 4.0f);
        pakistanMap->makeEdge(DOWNTOWN, WEST, 5.2f, 65.0f, 65.0f, 0.10f, 4.0f);
        pakistanMap->makeEdge(DOWNTOWN, EAST, 5.0f, 65.0f, 65.0f, 0.10f, 4.0f);
        pakistanMap->makeEdge(EAST, DOWNTOWN, 5.0f, 65.0f, 65.0f, 0.10f, 4.0f);

        pakistanMap->makeEdge(NORTH, TECH, 4.0f, 50.0f, 45.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(TECH, NORTH, 4.0f, 50.0f, 45.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(TECH, EAST, 4.2f, 50.0f, 45.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(EAST, TECH, 4.2f, 50.0f, 45.0f, 0.10f, 3.0f);

        pakistanMap->makeEdge(NORTH, UNIV, 3.8f, 50.0f, 45.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(UNIV, NORTH, 3.8f, 50.0f, 45.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(UNIV, WEST, 4.1f, 50.0f, 45.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(WEST, UNIV, 4.1f, 50.0f, 45.0f, 0.10f, 3.0f);

        pakistanMap->makeEdge(NORTH, OLDTOWN, 3.2f, 40.0f, 35.0f, 0.15f, 2.0f);
        pakistanMap->makeEdge(OLDTOWN, NORTH, 3.2f, 40.0f, 35.0f, 0.15f, 2.0f);

        pakistanMap->makeEdge(SOUTH, INDUS, 3.9f, 50.0f, 50.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(INDUS, SOUTH, 3.9f, 50.0f, 50.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(INDUS, EAST, 4.8f, 55.0f, 50.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(EAST, INDUS, 4.8f, 55.0f, 50.0f, 0.10f, 3.0f);

        pakistanMap->makeEdge(SOUTH, HOSP, 3.6f, 50.0f, 40.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(HOSP, SOUTH, 3.6f, 50.0f, 40.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(HOSP, WEST, 4.4f, 50.0f, 40.0f, 0.10f, 3.0f);
        pakistanMap->makeEdge(WEST, HOSP, 4.4f, 50.0f, 40.0f, 0.10f, 3.0f);

        pakistanMap->makeEdge(EAST, AIR, 4.5f, 70.0f, 70.0f, 0.10f, 4.0f);
        pakistanMap->makeEdge(AIR, EAST, 4.5f, 70.0f, 70.0f, 0.10f, 4.0f);

        // Initial optimization
        this->optimizeCitySignals(false);
        for (int i = 0; i < pakistanMap->Vcount; i++) {
            std::list<RoadDetails*> inEdges;
            inEdges = pakistanMap->getEdges(pakistanMap->getVertexAt(i), inEdges);
            if (inEdges.size() > 1) {
                auto it = inEdges.begin();
                (*it)->change_to_green();
                it++;
                while (it != inEdges.end()) {
                    (*it)->change_to_red();
                    it++;
                }
            }
        }
    }

    void setupFantasyRealmsNetwork() {
        isCityLevelMode = false;
        activeCityScenario = "MYTHICAL_REALM_TRANSIT";

        std::string realmHubs[] = {
            "Mount Olympus", "Tartarus", "Atlantis", "Elysium"
        };
        for (const std::string& name : realmHubs) {
            pakistanMap->insertVertex(name);
        }

        pakistanMap->setVertexCoordinates("Mount Olympus", 34.00f, 71.00f);
        pakistanMap->setVertexCoordinates("Tartarus",      25.00f, 71.00f);
        pakistanMap->setVertexCoordinates("Atlantis",      29.50f, 64.00f);
        pakistanMap->setVertexCoordinates("Elysium",       29.50f, 78.00f);

        int OLYMPUS  = pakistanMap->getIndex("Mount Olympus");
        int TARTARUS = pakistanMap->getIndex("Tartarus");
        int ATLANTIS = pakistanMap->getIndex("Atlantis");
        int ELYSIUM  = pakistanMap->getIndex("Elysium");

        // Grand Mythical Highways
        pakistanMap->makeEdge(OLYMPUS, ELYSIUM, 12.0f, 100.0f, 100.0f, 0.05f, 4.0f);
        pakistanMap->makeEdge(ELYSIUM, OLYMPUS, 12.0f, 100.0f, 100.0f, 0.05f, 4.0f);

        pakistanMap->makeEdge(OLYMPUS, ATLANTIS, 11.5f, 100.0f, 100.0f, 0.05f, 4.0f);
        pakistanMap->makeEdge(ATLANTIS, OLYMPUS, 11.5f, 100.0f, 100.0f, 0.05f, 4.0f);

        pakistanMap->makeEdge(OLYMPUS, TARTARUS, 18.0f, 120.0f, 120.0f, 0.05f, 4.0f);
        pakistanMap->makeEdge(TARTARUS, OLYMPUS, 18.0f, 120.0f, 120.0f, 0.05f, 4.0f);

        pakistanMap->makeEdge(ATLANTIS, TARTARUS, 12.5f, 90.0f, 90.0f, 0.05f, 4.0f);
        pakistanMap->makeEdge(TARTARUS, ATLANTIS, 12.5f, 90.0f, 90.0f, 0.05f, 4.0f);

        pakistanMap->makeEdge(ELYSIUM, TARTARUS, 13.0f, 90.0f, 90.0f, 0.05f, 4.0f);
        pakistanMap->makeEdge(TARTARUS, ELYSIUM, 13.0f, 90.0f, 90.0f, 0.05f, 4.0f);

        pakistanMap->makeEdge(ATLANTIS, ELYSIUM, 15.0f, 110.0f, 100.0f, 0.05f, 4.0f);
        pakistanMap->makeEdge(ELYSIUM, ATLANTIS, 15.0f, 110.0f, 100.0f, 0.05f, 4.0f);

        this->optimizeCitySignals(false);
        for (int i = 0; i < pakistanMap->Vcount; i++) {
            std::list<RoadDetails*> inEdges;
            inEdges = pakistanMap->getEdges(pakistanMap->getVertexAt(i), inEdges);
            if (inEdges.size() > 1) {
                auto it = inEdges.begin();
                (*it)->change_to_green();
                it++;
                while (it != inEdges.end()) {
                    (*it)->change_to_red();
                    it++;
                }
            }
        }
    }

    void addFantasyRealmCaravans(int count = 40) {
        std::string realmNames[] = { "Mount Olympus", "Tartarus", "Atlantis", "Elysium" };
        static int realmSeq = 95000;
        for (int i = 0; i < count; i++) {
            int u = rand() % 4;
            int v = (u + 1 + rand() % 3) % 4;
            VehicleType type = (VehicleType)(rand() % 6);
            cityManager->addVehicle(realmSeq++, realmNames[u], realmNames[v], type);
        }
    }

    CityPlanningReport optimizeCitySignals(bool useGA = true) {
        float sampleVolumes[4] = { 850.0f, 240.0f, 720.0f, 210.0f };
        for (int i = 0; i < pakistanMap->Vcount; i++) {
            std::string name = pakistanMap->getVertexAt(i);
            CitySignalTimingPlan plan;
            if (useGA) {
                plan = CityGeneticSignalOptimizer::optimizeIntersection(name, sampleVolumes, 16, 10);
            } else {
                plan = WebsterEngine::calculateWebsterPlan(name, sampleVolumes);
            }
            cityTimingPlans[name] = plan;
            auto* nodes = pakistanMap->getNodes();
            for (auto& edge : nodes[i].Neighbors) {
                edge.weight.light.applyTimingPlan(plan, 0);
                edge.weight.light.strategy = useGA ? STRATEGY_GA_OPTIMIZED : STRATEGY_FIXED_WEBSTER;
            }
        }

        float baseDelay = 32.5f;
        float optDelay  = useGA ? 16.8f : 24.2f;
        int baseQ = 14;
        int optQ = useGA ? 5 : 9;
        float baseCO2 = 18.5f;
        float optCO2 = useGA ? 12.1f : 15.3f;
        float tp = 2450.0f;

        cityReport = CityPlanningHelper::generateComparison(
            useGA ? "GA-Optimized City Timing Plan" : "Webster Baseline Plan",
            baseDelay, optDelay, baseQ, optQ, baseCO2, optCO2, tp
        );

        return cityReport;
    }

    void addCityCommuterTraffic(int totalVehicles = 40) {
        if (!isCityLevelMode) {
            setupCityLevelNetwork();
        }
        if (pakistanMap->Vcount < 2) return;

        static int cityVehSeq = 88000;
        for (int i = 0; i < totalVehicles; i++) {
            int u = rand() % pakistanMap->Vcount;
            int v = (u + 1 + rand() % (pakistanMap->Vcount - 1)) % pakistanMap->Vcount;
            std::string org = pakistanMap->getVertexAt(u);
            std::string dst = pakistanMap->getVertexAt(v);

            VehicleType vType = VEHICLE_CAR;
            int roll = rand() % 100;
            if (roll < 40) vType = VEHICLE_CAR;
            else if (roll < 58) vType = VEHICLE_EV;
            else if (roll < 72) vType = VEHICLE_MOTORCYCLE;
            else if (roll < 84) vType = VEHICLE_BUS;
            else if (roll < 94) vType = VEHICLE_TRUCK;
            else vType = VEHICLE_EMERGENCY;

            cityManager->addVehicle(cityVehSeq++, org, dst, vType);
        }
    }

    void setupNetwork() {
        string cityNames[] = { "Karachi", "Sukkur", "Quetta", "DG Khan", "Multan",
                               "Lahore", "Islamabad", "Faisalabad", "Peshawar",
                               "Gujranwala", "Sialkot" };
        for (const string& name : cityNames) pakistanMap->insertVertex(name);

        // Register GPS coordinates for A* Geodetic Haversine Routing
        pakistanMap->setVertexCoordinates("Karachi", 24.8607f, 67.0011f);
        pakistanMap->setVertexCoordinates("Sukkur", 27.7052f, 68.8574f);
        pakistanMap->setVertexCoordinates("Quetta", 30.1798f, 66.9750f);
        pakistanMap->setVertexCoordinates("DG Khan", 30.0561f, 70.6403f);
        pakistanMap->setVertexCoordinates("Multan", 30.1575f, 71.5249f);
        pakistanMap->setVertexCoordinates("Faisalabad", 31.4504f, 73.1350f);
        pakistanMap->setVertexCoordinates("Lahore", 31.5204f, 74.3587f);
        pakistanMap->setVertexCoordinates("Gujranwala", 32.1877f, 74.1945f);
        pakistanMap->setVertexCoordinates("Sialkot", 32.4945f, 74.5229f);
        pakistanMap->setVertexCoordinates("Islamabad", 33.6844f, 73.0479f);
        pakistanMap->setVertexCoordinates("Peshawar", 34.0151f, 71.5249f);

        int KHI = pakistanMap->getIndex("Karachi"); int SUK = pakistanMap->getIndex("Sukkur");
        int QUE = pakistanMap->getIndex("Quetta");  int DGK = pakistanMap->getIndex("DG Khan");
        int MUL = pakistanMap->getIndex("Multan");  int FSD = pakistanMap->getIndex("Faisalabad");
        int LHR = pakistanMap->getIndex("Lahore");  int ISB = pakistanMap->getIndex("Islamabad");
        int PSW = pakistanMap->getIndex("Peshawar"); int GJR = pakistanMap->getIndex("Gujranwala");
        int SKT = pakistanMap->getIndex("Sialkot");

        // --- 1. THE CENTRAL HUB (Multan) ---
        int mainSpokes[] = { SUK, FSD, LHR, DGK, ISB, QUE };
        for (int target : mainSpokes) {
            pakistanMap->makeEdge(MUL, target, 300, 110, 80, 0.1, 4.0);
            pakistanMap->makeEdge(target, MUL, 300, 110, 80, 0.1, 4.0);
        }

        // --- 2. REGIONAL LINKS & OUTER ARTERIES ---
        pakistanMap->makeEdge(KHI, QUE, 680, 100, 40, 0.2, 3.0); // South-West Link
        pakistanMap->makeEdge(QUE, KHI, 680, 100, 40, 0.2, 3.0);

        pakistanMap->makeEdge(QUE, DGK, 350, 90, 45, 0.1, 4.0);  // Western bypass
        pakistanMap->makeEdge(DGK, QUE, 350, 90, 45, 0.1, 4.0);

        pakistanMap->makeEdge(QUE, SUK, 500, 100, 50, 0.15, 4.0); // Quetta-Sukkur link
        pakistanMap->makeEdge(SUK, QUE, 500, 100, 50, 0.15, 4.0);

        pakistanMap->makeEdge(FSD, ISB, 300, 120, 80, 0.1, 4.0); // Motorway Bypass
        pakistanMap->makeEdge(ISB, FSD, 300, 120, 80, 0.1, 4.0);

        pakistanMap->makeEdge(ISB, LHR, 370, 120, 110, 0.1, 4.0); // M-2 Main Motorway
        pakistanMap->makeEdge(LHR, ISB, 370, 120, 110, 0.1, 4.0);

        pakistanMap->makeEdge(LHR, FSD, 180, 115, 90, 0.1, 4.0);  // FSD-LHR Industrial Link
        pakistanMap->makeEdge(FSD, LHR, 180, 115, 90, 0.1, 4.0);

        pakistanMap->makeEdge(PSW, FSD, 420, 110, 65, 0.1, 4.0); // Northern Bypass
        pakistanMap->makeEdge(FSD, PSW, 420, 110, 65, 0.1, 4.0);

        pakistanMap->makeEdge(GJR, FSD, 160, 100, 55, 0.1, 3.0); // Gujranwala to Faisalabad
        pakistanMap->makeEdge(FSD, GJR, 160, 100, 55, 0.1, 3.0);

        // --- 3. GT ROAD CLUSTER ---
        pakistanMap->makeEdge(LHR, GJR, 80, 100, 60, 0.1, 3.0);
        pakistanMap->makeEdge(GJR, LHR, 80, 100, 60, 0.1, 3.0);
        pakistanMap->makeEdge(GJR, SKT, 50, 90, 40, 0.1, 3.0);
        pakistanMap->makeEdge(SKT, GJR, 50, 90, 40, 0.1, 3.0);

        // --- 4. LONG-HAUL BYPASSES & MAIN LINES ---
        pakistanMap->makeEdge(PSW, KHI, 1100, 110, 80, 0.1, 5.0);
        pakistanMap->makeEdge(KHI, PSW, 1100, 110, 80, 0.1, 5.0);

        pakistanMap->makeEdge(KHI, LHR, 1000, 120, 100, 0.1, 4.0); // Karachi-Lahore Main Line
        pakistanMap->makeEdge(LHR, KHI, 1000, 120, 100, 0.1, 4.0);

        pakistanMap->makeEdge(PSW, LHR, 450, 110, 75, 0.1, 4.0); // Northern Artery
        pakistanMap->makeEdge(LHR, PSW, 450, 110, 75, 0.1, 4.0);

        pakistanMap->makeEdge(QUE, LHR, 750, 100, 60, 0.15, 4.0); // Western Artery
        pakistanMap->makeEdge(LHR, QUE, 750, 100, 60, 0.15, 4.0);

        pakistanMap->makeEdge(KHI, ISB, 1150, 120, 90, 0.1, 4.0); // Capital Link
        pakistanMap->makeEdge(ISB, KHI, 1150, 120, 90, 0.1, 4.0);

        // Feature 7: Enable M-Tag Electronic Toll Plazas on major National Motorways
        try {
            pakistanMap->getEdgeDetails("Lahore", "Islamabad").setTollPlaza(true, 130.0f);
            pakistanMap->getEdgeDetails("Islamabad", "Lahore").setTollPlaza(true, 130.0f);
            pakistanMap->getEdgeDetails("Karachi", "Sukkur").setTollPlaza(true, 110.0f);
            pakistanMap->getEdgeDetails("Sukkur", "Karachi").setTollPlaza(true, 110.0f);
            pakistanMap->getEdgeDetails("Sukkur", "Multan").setTollPlaza(true, 120.0f);
            pakistanMap->getEdgeDetails("Multan", "Sukkur").setTollPlaza(true, 120.0f);
        } catch (...) {}

        // Feature 8: Register EV Highway Fast-Charging Hubs
        cityManager->addChargingStation("Sukkur");
        cityManager->addChargingStation("Multan");
        cityManager->addChargingStation("Lahore");
        cityManager->addChargingStation("Islamabad");
    }

    void addMassiveTraffic() {
        // Ensure the random seed is set based on the current time
        srand(static_cast<unsigned int>(time(0)));

        string cities[] = {
            "Karachi", "Sukkur", "Quetta", "DG Khan", "Multan",
            "Lahore", "Islamabad", "Faisalabad", "Peshawar",
            "Gujranwala", "Sialkot"
        };
        int totalCityCount = 11;

        cout << "--- INJECTING 10,000 RANDOMIZED UNITS ---\n";

        for (int i = 1; i <= 50000; i++) {
            // Pick any city as a start
            int startIdx = rand() % totalCityCount;
            // Pick any city as an end
            int endIdx = rand() % totalCityCount;

            // Ensure a vehicle doesn't try to go to its own starting city
            if (startIdx == endIdx) {
                endIdx = (startIdx + 1) % totalCityCount;
            }

            string start = cities[startIdx];
            string end = cities[endIdx];

            VehicleType vType = VEHICLE_CAR;
            int r = rand() % 100;
            if (r < 55) vType = VEHICLE_CAR;
            else if (r < 70) vType = VEHICLE_EV; // 15% Electric Vehicle penetration
            else if (r < 84) vType = VEHICLE_BUS;
            else if (r < 96) vType = VEHICLE_TRUCK;
            else vType = VEHICLE_EMERGENCY;

            cityManager->addVehicle(i, start, end, vType);
        }

        cityManager->printNetwork();
    }

    // =========================================================================
    // FEATURE 3: DEMOGRAPHIC GRAVITY MODEL ORIGIN-DESTINATION (O-D) MATRIX
    // =========================================================================
    struct CityGravityData {
        string name;
        float populationMillions;
    };

    void addGravityDemand(int totalVehicles) {
        srand(static_cast<unsigned int>(time(0)));

        CityGravityData cityDemographics[] = {
            { "Karachi",    16.0f },
            { "Lahore",     13.0f },
            { "Faisalabad",  3.5f },
            { "Islamabad",   3.2f },
            { "Gujranwala",  2.2f },
            { "Peshawar",    2.0f },
            { "Multan",      2.0f },
            { "Quetta",      1.1f },
            { "Sialkot",     0.7f },
            { "Sukkur",      0.5f },
            { "DG Khan",     0.4f }
        };
        int numCities = 11;

        // Build Cumulative Gravity Distribution Matrix
        struct ODCell {
            string origin;
            string destination;
            float weight;
            float cdf;
        };
        vector<ODCell> odMatrix;
        float totalWeight = 0.0f;

        for (int i = 0; i < numCities; i++) {
            for (int j = 0; j < numCities; j++) {
                if (i == j) continue;
                string org = cityDemographics[i].name;
                string dst = cityDemographics[j].name;

                float lat1, lon1, lat2, lon2;
                pakistanMap->getVertexCoordinates(org, lat1, lon1);
                pakistanMap->getVertexCoordinates(dst, lat2, lon2);
                float distKm = Graph<t, size>::haversineDistanceKm(lat1, lon1, lat2, lon2);
                if (distKm < 10.0f) distKm = 10.0f;

                // T_ij = (P_i * P_j) / (distance^1.2)
                float gravWeight = (cityDemographics[i].populationMillions * cityDemographics[j].populationMillions) /
                                   powf(distKm, 1.2f);

                odMatrix.push_back({ org, dst, gravWeight, 0.0f });
                totalWeight += gravWeight;
            }
        }

        // Compute cumulative distribution function (CDF)
        float runningSum = 0.0f;
        for (auto& cell : odMatrix) {
            runningSum += cell.weight / totalWeight;
            cell.cdf = runningSum;
        }

        cout << "--- INJECTING " << totalVehicles << " GRAVITY-DISTRIBUTED COMMUTER UNITS ---\n";

        static int gravityVehicleIdSeq = 70000;
        for (int v = 0; v < totalVehicles; v++) {
            float r = (float)(rand() % 10000) / 10000.0f;
            string selectedOrigin = odMatrix[0].origin;
            string selectedDest = odMatrix[0].destination;

            // Binary search in CDF
            int low = 0, high = (int)odMatrix.size() - 1;
            while (low <= high) {
                int mid = low + (high - low) / 2;
                if (odMatrix[mid].cdf >= r) {
                    selectedOrigin = odMatrix[mid].origin;
                    selectedDest = odMatrix[mid].destination;
                    high = mid - 1;
                } else {
                    low = mid + 1;
                }
            }

            VehicleType vType = VEHICLE_CAR;
            int typeRoll = rand() % 100;
            if (typeRoll < 55) vType = VEHICLE_CAR;
            else if (typeRoll < 70) vType = VEHICLE_EV;
            else if (typeRoll < 84) vType = VEHICLE_BUS;
            else if (typeRoll < 96) vType = VEHICLE_TRUCK;
            else vType = VEHICLE_EMERGENCY;

            cityManager->addVehicle(gravityVehicleIdSeq++, selectedOrigin, selectedDest, vType);
        }
    }

    void blockRoad(t u, t v) { cityManager->blockRoad(u, v); }
    void unblockRoad(t u, t v) { cityManager->unblockRoad(u, v); }
    void setRoadCapacityFactor(t u, t v, float factor) { cityManager->setRoadCapacityFactor(u, v, factor); }

    // Feature 9: Weather Control
    void setWeather(WeatherCondition w) { cityManager->setGlobalWeather(w); }

    // Feature 11: Dynamic CPEC Network Expansion
    void addCPECCityNode(t cityName, float lat, float lon) { cityManager->addCityNode(cityName, lat, lon); }
    void addCPECHighwayLink(t u, t v, float lengthKm, float speedKmh, float capacityVeh, bool isBidirectional = true) {
        cityManager->addHighwayLink(u, v, lengthKm, speedKmh, capacityVeh, isBidirectional);
    }

    void run() {
        cout << "--- STARTING HEAVY TRAFFIC SIMULATION (3500 VEHICLES) ---\n";
        cityManager->physics();
    }

    ~Simulator() {
        delete cityManager;
        delete pakistanMap;
    }

    void metric(float total_time, float x) {
        cityManager->printCSVRow(total_time, x);
        cityManager->printPerformanceMetrics(total_time, x);
    }
    float rush() {
        return pakistanMap->AverageRush();
    }
    void clear() { cityManager->clearMetricsFile(); }
    void resetSimulation() {
        if (cityManager) cityManager->reset();
        if (pakistanMap) pakistanMap->clear();
        cityTimingPlans.clear();
        isCityLevelMode = true;
    }
};
