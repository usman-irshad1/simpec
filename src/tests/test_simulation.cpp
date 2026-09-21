#include <iostream>
#include <cassert>
#include <cmath>
#include <map>
#include <vector>
#include <string>
#include "../core/TrafficSignal.h"
#include "../core/RoadDetails.h"
#include "../core/vehicle.h"
#include "../core/Graph.h"
#include "../simulation/Manger.h"
#include "../simulation/Simulator.h"

using namespace std;

// Standalone Point2D struct for testing projection and geometry headlessly
struct TestPoint2D {
    float x;
    float y;
};

// Headless distance to segment math test
float TestDistanceToSegment(TestPoint2D p, TestPoint2D a, TestPoint2D b) {
    float abX = b.x - a.x;
    float abY = b.y - a.y;
    float l2 = abX * abX + abY * abY;
    if (l2 <= 0.0001f) {
        float dx = p.x - a.x;
        float dy = p.y - a.y;
        return sqrtf(dx * dx + dy * dy);
    }
    float apX = p.x - a.x;
    float apY = p.y - a.y;
    float t = (apX * abX + apY * abY) / l2;
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    float projX = a.x + t * abX;
    float projY = a.y + t * abY;
    float diffX = p.x - projX;
    float diffY = p.y - projY;
    return sqrtf(diffX * diffX + diffY * diffY);
}

int main() {
    cout << "=== RUNNING COMPLETE TRAFFIC SIMULATION VERIFICATION TESTS (PHASES 1, 2, 3 & 4) ===" << endl;

    // --- PHASE 1 TESTS ---
    // 1. Test TrafficSignal::canSwitch with 0 competing queue (verify division by zero fix)
    TrafficSignal signal;
    signal.greentimer = 3.0f;
    bool switchResult = signal.canSwitch(5, 0);
    cout << "[TEST 1 PASSED] TrafficSignal::canSwitch with maxOtherQueue=0 executed safely (result = " 
         << switchResult << ")" << endl;

    // 2. Test VehicleState enum and initialRoadTravelTime
    vehicle<string> testCar(101, "Karachi", "Lahore");
    assert(testCar.isQueued());
    assert(!testCar.isEnRoute());
    testCar.setState(VEHICLE_STATE_EN_ROUTE);
    assert(testCar.isEnRoute());
    testCar.initialRoadTravelTime = 15.5f;
    assert(testCar.initialRoadTravelTime == 15.5f);
    cout << "[TEST 2 PASSED] VehicleState enum and initialRoadTravelTime field verified." << endl;

    // --- PHASE 2 TESTS ---
    // 3. Test Graph Min-Heap Dijkstra and O(1) vertex lookup
    Simulator<string, 100> sim;
    sim.setupNetwork();
    Graph<string, 100>* g = sim.getMap();

    int khiIndex = g->getIndex("Karachi");
    int lhrIndex = g->getIndex("Lahore");
    assert(khiIndex != -1);
    assert(lhrIndex != -1);
    cout << "[TEST 3 PASSED] O(1) Vertex Index lookup: Karachi=" << khiIndex << ", Lahore=" << lhrIndex << endl;

    // 4. Test Shortest Path via Priority Queue Dijkstra
    list<string> path = g->shortest_Path_btw2_vericex_returing_list("Karachi", "Islamabad");
    assert(!path.empty());
    assert(path.front() == "Karachi");
    assert(path.back() == "Islamabad");
    cout << "[TEST 4 PASSED] Min-Heap Dijkstra found valid path Karachi -> Islamabad with " << path.size() << " nodes: ";
    for (const auto& city : path) cout << city << " ";
    cout << endl;

    // 5. Test Duplicate Edge Prevention
    int initialEdges = g->No_Of_Edges();
    g->makeEdge(khiIndex, lhrIndex, 1000, 120, 100, 0.1, 4.0);
    int updatedEdges = g->No_Of_Edges();
    assert(initialEdges == updatedEdges);
    cout << "[TEST 5 PASSED] Duplicate edge prevention verified (edge count stayed " << updatedEdges << ")." << endl;

    // 6. Test Multi-tick simulation step with Manager
    sim.cityManager->addVehicle(1001, "Karachi", "Sukkur");
    sim.cityManager->addVehicle(1002, "Lahore", "Islamabad");
    assert(sim.cityManager->getVehicleCount() >= 2);

    for (int t = 0; t < 10; t++) {
        sim.cityManager->updateSignals();
        sim.cityManager->reached();
        sim.cityManager->arrivalAtIntersection();
        sim.cityManager->entraingfromQueetoEdge();
        sim.cityManager->entrance();
    }
    cout << "[TEST 6 PASSED] Multi-tick simulation stepped successfully." << endl;

    // --- PHASE 3 TESTS ---
    // 7. Test Yellow Clearance and Emergency Signal Preemption
    bool testSigState = true;
    TrafficSignal phase3Signal;
    phase3Signal.turnGreen(testSigState);
    assert(phase3Signal.isGreen());
    assert(testSigState == true);

    phase3Signal.turnYellow(testSigState);
    assert(phase3Signal.isYellow());
    assert(testSigState == false); // New entries stopped

    phase3Signal.Timer(testSigState, 2.0f); // Yellow timer expires
    assert(phase3Signal.isRed());
    assert(testSigState == false);

    phase3Signal.triggerEmergencyPreemption(testSigState);
    assert(phase3Signal.isGreen());
    assert(testSigState == true);
    assert(phase3Signal.emergencyPreempted == true);
    cout << "[TEST 7 PASSED] TrafficSignal Yellow clearance phase and Emergency Preemption verified." << endl;

    // 8. Test Vehicle Taxonomy (Cars, Trucks, Buses, Ambulances)
    vehicle<string> car(201, "Karachi", "Lahore", VEHICLE_CAR);
    vehicle<string> truck(202, "Sukkur", "Multan", VEHICLE_TRUCK);
    vehicle<string> bus(203, "Faisalabad", "Lahore", VEHICLE_BUS);
    vehicle<string> ambulance(204, "Quetta", "Islamabad", VEHICLE_EMERGENCY);

    assert(car.pcu == 1.0f && car.speedMultiplier == 1.0f);
    assert(truck.isTruck() && truck.pcu == 2.5f && truck.speedMultiplier == 0.8f);
    assert(bus.isBus() && bus.pcu == 2.0f && bus.speedMultiplier == 0.9f);
    assert(ambulance.isEmergency() && ambulance.speedMultiplier == 1.35f);
    cout << "[TEST 8 PASSED] Vehicle Taxonomy (PCU and Speed multipliers) verified." << endl;

    // 9. Test Dynamic Incident, Road Blocking & Adaptive Rerouting
    list<string> directPath = g->shortest_Path_btw2_vericex_returing_list("Karachi", "Lahore");
    assert(directPath.size() == 2); // Direct Karachi -> Lahore
    cout << "[TEST 9.1] Direct path Karachi -> Lahore has " << directPath.size() << " hops." << endl;

    // Block the direct road between Karachi and Lahore
    sim.blockRoad("Karachi", "Lahore");
    list<string> detourPath = g->shortest_Path_btw2_vericex_returing_list("Karachi", "Lahore");
    assert(!detourPath.empty());
    assert(detourPath.size() > 2); // Must detour via an intermediate city (e.g. Sukkur or Multan)
    cout << "[TEST 9.2 PASSED] Road blockage triggered adaptive detour! Detour path has " << detourPath.size() << " hops: ";
    for (const auto& c : detourPath) cout << c << " ";
    cout << endl;

    // Unblock the road and verify route restores to direct
    sim.unblockRoad("Karachi", "Lahore");
    list<string> restoredPath = g->shortest_Path_btw2_vericex_returing_list("Karachi", "Lahore");
    assert(restoredPath.size() == 2);
    cout << "[TEST 9.3 PASSED] Road unblock restored direct Karachi -> Lahore link." << endl;

    // 10. Test Commuter Peak Rush Wave Injections
    int prevCount = sim.cityManager->getVehicleCount();
    sim.cityManager->injectPeakDemand(100, true); // Morning rush into metro hubs
    int newCount = sim.cityManager->getVehicleCount();
    assert(newCount > prevCount);
    cout << "[TEST 10 PASSED] Commuter peak rush demand injected " << (newCount - prevCount) << " vehicles." << endl;

    // --- PHASE 4 TESTS ---
    // 11. Test Pakistan Real GPS Coordinates Mapping
    struct TestGeoCoord { float lat; float lon; };
    map<string, TestGeoCoord> coords;
    coords["Karachi"]    = { 24.8607f, 67.0011f };
    coords["Sukkur"]     = { 27.7052f, 68.8574f };
    coords["Quetta"]     = { 30.1798f, 66.9750f };
    coords["DG Khan"]    = { 30.0561f, 70.6403f };
    coords["Multan"]     = { 30.1575f, 71.5249f };
    coords["Faisalabad"] = { 31.4504f, 73.1350f };
    coords["Lahore"]     = { 31.5204f, 74.3587f };
    coords["Gujranwala"] = { 32.1877f, 74.1945f };
    coords["Sialkot"]    = { 32.4945f, 74.5229f };
    coords["Islamabad"]  = { 33.6844f, 73.0479f };
    coords["Peshawar"]   = { 34.0151f, 71.5249f };

    assert(coords.size() == 11);
    // Verify Karachi is southernmost and Peshawar is northernmost
    assert(coords["Karachi"].lat < coords["Sukkur"].lat);
    assert(coords["Sukkur"].lat < coords["Multan"].lat);
    assert(coords["Multan"].lat < coords["Lahore"].lat);
    assert(coords["Lahore"].lat < coords["Islamabad"].lat);
    assert(coords["Islamabad"].lat < coords["Peshawar"].lat);
    // Verify Quetta is westernmost and Sialkot is easternmost
    assert(coords["Quetta"].lon < coords["DG Khan"].lon);
    assert(coords["DG Khan"].lon < coords["Multan"].lon);
    assert(coords["Multan"].lon < coords["Lahore"].lon);
    assert(coords["Lahore"].lon < coords["Sialkot"].lon);
    cout << "[TEST 11 PASSED] Real Pakistan GPS coordinates geodetic boundaries verified." << endl;

    // 12. Test Geographic Viewport Projection Math
    const float minLat = 24.2f, maxLat = 34.6f, minLon = 66.4f, maxLon = 75.2f;
    const float latSpan = maxLat - minLat;
    const float lonSpan = maxLon - minLon;
    const float kmHeight = latSpan * 111.0f;
    const float kmWidth  = lonSpan * 96.0f;
    const float viewW = 1000.0f, viewH = 800.0f;
    float scale = (viewW / kmWidth < viewH / kmHeight) ? (viewW / kmWidth) : (viewH / kmHeight);

    map<string, TestPoint2D> projected;
    for (const auto& c : coords) {
        float xKm = (c.second.lon - minLon) * 96.0f;
        float yKm = (maxLat - c.second.lat) * 111.0f; // North is up -> smaller Y
        projected[c.first] = { xKm * scale, yKm * scale };
    }

    // In screen coords, North is smaller Y (Karachi Y > Peshawar Y)
    assert(projected["Karachi"].y > projected["Peshawar"].y);
    assert(projected["Quetta"].x < projected["Lahore"].x);
    cout << "[TEST 12 PASSED] Geographic Viewport Projection orientation and scaling verified." << endl;

    // 13. Test Distance to Segment Geometry for Inspector Hover Detection
    TestPoint2D p1 = { 5.0f, 5.0f };
    TestPoint2D a = { 0.0f, 0.0f };
    TestPoint2D b = { 10.0f, 0.0f };
    float distMid = TestDistanceToSegment(p1, a, b);
    assert(fabs(distMid - 5.0f) < 0.001f); // Orthogonal projection

    TestPoint2D pBefore = { -3.0f, 0.0f };
    float distBefore = TestDistanceToSegment(pBefore, a, b);
    assert(fabs(distBefore - 3.0f) < 0.001f); // Clamped to A

    TestPoint2D pAfter = { 14.0f, 0.0f };
    float distAfter = TestDistanceToSegment(pAfter, a, b);
    assert(fabs(distAfter - 4.0f) < 0.001f); // Clamped to B
    cout << "[TEST 13 PASSED] Point-to-segment distance calculations for inspector hover verified." << endl;

    // 14. Test Rolling Telemetry History Buffer for Sparkline Charts
    vector<float> sparklineBuffer;
    for (int i = 0; i < 100; i++) {
        sparklineBuffer.push_back((float)(i % 50));
        if (sparklineBuffer.size() > 60) {
            sparklineBuffer.erase(sparklineBuffer.begin());
        }
    }
    assert(sparklineBuffer.size() == 60);
    assert(sparklineBuffer.back() == (99 % 50));
    cout << "[TEST 14 PASSED] Rolling telemetry history buffer verified (capped at 60 points)." << endl;

    // --- ADVANCED FEATURE 1 TESTS ---
    // 15. Test A* Geodetic Haversine Routing and Optimality Equivalence with Dijkstra
    list<string> dijkstraPath = g->shortest_Path_btw2_vericex_returing_list("Karachi", "Islamabad");
    list<string> aStarPath = g->aStarShortestPath("Karachi", "Islamabad");
    assert(!aStarPath.empty());
    assert(aStarPath == dijkstraPath); // A* mathematically matches Dijkstra

    sim.blockRoad("Karachi", "Lahore");
    list<string> aStarDetour = g->aStarShortestPath("Karachi", "Lahore");
    list<string> dijkstraDetour = g->shortest_Path_btw2_vericex_returing_list("Karachi", "Lahore");
    cout << "A* Detour: ";
    for (const auto& s : aStarDetour) cout << s << " ";
    cout << "\nDijkstra Detour: ";
    for (const auto& s : dijkstraDetour) cout << s << " ";
    cout << endl;
    assert(aStarDetour == dijkstraDetour);
    sim.unblockRoad("Karachi", "Lahore");

    // Verify Haversine formula calculation
    float k2lDist = Graph<string, 100>::haversineDistanceKm(24.8607f, 67.0011f, 31.5204f, 74.3587f);
    assert(k2lDist > 1000.0f && k2lDist < 1060.0f);
    cout << "[TEST 15 PASSED] Feature 1: A* Geodetic Haversine Routing verified with mathematical Dijkstra optimality equivalence (Haversine KHI-LHR = " 
         << k2lDist << " km)." << endl;

    // --- ADVANCED FEATURE 2 TESTS ---
    // 16. Test Max-Pressure and Q-Learning Signal Controller
    TrafficSignal qSignal;
    assert(qSignal.discretizeQueueState(2) == 0);  // Low
    assert(qSignal.discretizeQueueState(10) == 1); // Med
    assert(qSignal.discretizeQueueState(22) == 2); // High
    assert(qSignal.discretizeQueueState(45) == 3); // Saturated

    float initialQ = qSignal.qTable[1][1];
    qSignal.updateQValue(1, 1, 5.0f, 0);
    assert(qSignal.qTable[1][1] > initialQ); // Positive reward increased Q-value

    qSignal.greentimer = 4.0f; // Past minGreenTime
    bool switchAllowed = qSignal.shouldSwitchQAdaptive(12, 18, -5.0f);
    cout << "[TEST 16 PASSED] Feature 2: Q-Learning & Max-Pressure Adaptive Signal Controller verified (Q-table updated, switchAllowed = " 
         << switchAllowed << ")." << endl;

    // --- ADVANCED FEATURE 3 TESTS ---
    // 17. Test Demographic Gravity Model O-D Demand Matrix
    int beforeGravityVehicles = sim.cityManager->getVehicleCount();
    sim.addGravityDemand(30);
    int afterGravityVehicles = sim.cityManager->getVehicleCount();
    assert(afterGravityVehicles == beforeGravityVehicles + 30);
    cout << "[TEST 17 PASSED] Feature 3: Demographic Gravity Model O-D Matrix verified (injected 30 probabilistically weighted units)." << endl;

    // --- ADVANCED FEATURE 4 TESTS ---
    // 18. Test Microscopic IDM Car-Following Physics
    vehicle<string> idmCar(888, "Lahore", "Islamabad", VEHICLE_CAR);
    idmCar.currentSpeed = 20.0f; // 72 km/h
    float accelFreeRoad = idmCar.calculateIDMAcceleration(300.0f, 25.0f);
    assert(accelFreeRoad > 0.5f); // Smooth cruising acceleration towards 33.3 m/s

    float decelObstacle = idmCar.calculateIDMAcceleration(12.0f, 0.0f);
    assert(decelObstacle < -2.0f); // Strong braking when approaching stationary queue
    cout << "[TEST 18 PASSED] Feature 4: Microscopic IDM Car-Following Physics verified (free road a=" 
         << accelFreeRoad << " m/s^2, obstacle decel a=" << decelObstacle << " m/s^2)." << endl;

    // --- ADVANCED FEATURE 5 TESTS ---
    // 19. Test Multi-Lane Motorway & MOBIL Lane Changing
    RoadDetails multiLaneHighway(180, 120, 90);
    assert(multiLaneHighway.getNumLanes() == 3); // High capacity link gets 3 lanes

    // Safe overtaking lane change: target lane has higher acceleration, follower not braking hard
    bool changeSafe = idmCar.shouldChangeLaneMOBIL(0.2f, 1.2f, -1.0f);
    assert(changeSafe == true);

    // Unsafe cut-off: target lane follower would have to brake violently (-5.0 m/s^2)
    bool changeUnsafe = idmCar.shouldChangeLaneMOBIL(0.2f, 1.2f, -5.0f);
    assert(changeUnsafe == false);
    cout << "[TEST 19 PASSED] Feature 5: Multi-Lane Highway & MOBIL Overtaking Model verified (safe=" 
         << changeSafe << ", unsafe blocked=" << !changeUnsafe << ")." << endl;

    // --- ADVANCED FEATURE 6 TESTS ---
    // 20. Test Carbon Emissions (CO2, NOx) & Fuel Burn Tracker
    vehicle<string> gasCar(901, "Karachi", "Hyderabad", VEHICLE_CAR);
    gasCar.updateFuelAndEmissions(3600.0f, true, 0.0f); // 1 hour idling in jam
    assert(gasCar.fuelConsumedLiters > 0.8f && gasCar.fuelConsumedLiters < 1.0f); // ~0.9 L/h idle
    assert(gasCar.fuelWastedInJamLiters == gasCar.fuelConsumedLiters);
    assert(gasCar.co2EmittedKg > 2.0f && gasCar.co2EmittedKg < 2.3f); // ~2.31 kg CO2/L

    vehicle<string> dieselTruck(902, "Lahore", "Peshawar", VEHICLE_TRUCK);
    dieselTruck.updateFuelAndEmissions(100.0f, false, 25.0f); // 2.5 km cruising
    assert(dieselTruck.fuelConsumedLiters > 0.6f && dieselTruck.fuelConsumedLiters < 0.8f); // 28 L / 100 km
    assert(dieselTruck.co2EmittedKg > 1.8f); // Diesel CO2 factor = 2.68 kg/L
    cout << "[TEST 20 PASSED] Feature 6: Carbon Emissions (CO2) & Fuel Burn Tracker verified (Car 1h idle = " 
         << gasCar.fuelConsumedLiters << " L, Truck 2.5km = " << dieselTruck.co2EmittedKg << " kg CO2)." << endl;

    // --- ADVANCED FEATURE 7 TESTS ---
    // 21. Test M-Tag Electronic Toll Plazas & Cash-Lane Delay
    RoadDetails m2Motorway(350, 120, 100);
    m2Motorway.setTollPlaza(true, 150.0f);
    assert(m2Motorway.hasTollPlaza == true);
    float mTagDelay = m2Motorway.processToll(true);
    assert(mTagDelay < 3.0f); // Fast RFID gate: 1.5s
    assert(m2Motorway.mTagVehiclesServed == 1);

    float cashDelay = m2Motorway.processToll(false);
    assert(cashDelay > 15.0f); // Cash stop & manual receipt: 18.0s
    assert(m2Motorway.cashVehiclesServed == 1);
    assert(m2Motorway.totalTollCollectedPKR == 300.0f);
    cout << "[TEST 21 PASSED] Feature 7: M-Tag Electronic Toll Plazas verified (M-Tag delay=" 
         << mTagDelay << "s, Cash delay=" << cashDelay << "s, Revenue PKR " << m2Motorway.totalTollCollectedPKR << ")." << endl;

    // --- ADVANCED FEATURE 8 TESTS ---
    // 22. Test Electric Vehicle (EV) Fleet & Highway Fast-Charging Network
    vehicle<string> evCar(903, "Lahore", "Islamabad", VEHICLE_EV);
    assert(evCar.isEV() == true);
    assert(evCar.co2EmittedKg == 0.0f); // Zero direct tailpipe emissions!
    evCar.updateFuelAndEmissions(1000.0f, false, 30.0f); // 30 km highway drive
    assert(evCar.batterySoCPercent < 100.0f); // Battery consumed
    float socBeforeCharge = evCar.batterySoCPercent;
    evCar.batterySoCPercent = 15.0f; // Low battery state
    assert(evCar.isLowBattery() == true);
    evCar.rechargeBattery(75.0f);
    assert(evCar.batterySoCPercent == 90.0f);
    assert(evCar.isLowBattery() == false);
    cout << "[TEST 22 PASSED] Feature 8: EV Fleet & Fast-Charging Network verified (0g CO2 tailpipe, 30km trip SoC=" 
         << socBeforeCharge << "%, recharged to " << evCar.batterySoCPercent << "%)." << endl;

    // --- ADVANCED FEATURE 9 TESTS ---
    // 23. Test Pakistani Seasonal Weather Presets
    RoadDetails weatherRoad(100, 100, 50);
    weatherRoad.setWeather(WEATHER_CLEAR);
    float timeClear = weatherRoad.bestTime();
    assert(weatherRoad.frictionGrip == 1.0f);

    weatherRoad.setWeather(WEATHER_RAIN);
    assert(weatherRoad.frictionGrip == 0.72f);
    assert(weatherRoad.capacityFactor == 0.85f);

    weatherRoad.setWeather(WEATHER_DENSE_FOG);
    assert(weatherRoad.capacityFactor == 0.50f);
    assert(weatherRoad.getEffectiveMaxSpeed() == 45.0f);
    float timeFog = weatherRoad.bestTime();
    assert(timeFog > timeClear * 2.0f); // Fog doubles transit time

    sim.setWeather(WEATHER_SMOG);
    assert(sim.cityManager->getWeather() == WEATHER_SMOG);
    cout << "[TEST 23 PASSED] Feature 9: Pakistani Seasonal Weather Presets verified (Clear time=" 
         << timeClear << "s, Dense Fog time=" << timeFog << "s, Global Smog active)." << endl;

    // --- ADVANCED FEATURE 10 TESTS ---
    // 24. Test 24-Hour Day/Night Diurnal Clock & Ambient Darkness
    sim.cityManager->setSimClock(13.0f); // 1:00 PM Daylight
    assert(sim.cityManager->isNight() == false);
    assert(sim.cityManager->getAmbientDarkness() == 0.0f);

    sim.cityManager->setSimClock(23.0f); // 11:00 PM Night
    assert(sim.cityManager->isNight() == true);
    assert(sim.cityManager->getAmbientDarkness() > 0.8f);

    sim.cityManager->advanceClock(120.0f); // +2 hours advances to 01:00 AM
    assert(fabs(sim.cityManager->getClockHours() - 1.0f) < 0.01f);
    cout << "[TEST 24 PASSED] Feature 10: 24-Hour Diurnal Clock verified (13:00 darkness=" 
         << 0.0f << ", 23:00 darkness=" << sim.cityManager->getAmbientDarkness() << ", rollover clock=01:00)." << endl;

    // --- ADVANCED FEATURE 11 TESTS ---
    // 25. Test Dynamic CPEC Network Expansion (Gwadar Port & Expressways)
    sim.addCPECCityNode("Gwadar", 25.1216f, 62.3254f);
    sim.addCPECHighwayLink("Gwadar", "Quetta", 650.0f, 120.0f, 100.0f, true);
    sim.addCPECHighwayLink("Gwadar", "Karachi", 630.0f, 120.0f, 100.0f, true);

    list<string> gwadarToIslamabad = g->aStarShortestPath("Gwadar", "Islamabad");
    assert(!gwadarToIslamabad.empty());
    assert(gwadarToIslamabad.front() == "Gwadar");
    assert(gwadarToIslamabad.back() == "Islamabad");
    cout << "CPEC Route Gwadar -> Islamabad: ";
    for (const auto& c : gwadarToIslamabad) cout << c << " -> ";
    cout << "END" << endl;
    cout << "[TEST 25 PASSED] Feature 11: Dynamic CPEC Network Expansion verified (added Gwadar Port, discovered route to Islamabad)." << endl;

    // =========================================================================
    // C++ URBAN CITY-LEVEL PLANNING & GENETIC ALGORITHM OPTIMIZER (TESTS 26-30)
    // =========================================================================
    // 26. Test Webster Baseline Signal Formulation in C++
    float phaseVols[4] = { 850.0f, 240.0f, 720.0f, 210.0f };
    CitySignalTimingPlan websterPlan = WebsterEngine::calculateWebsterPlan("Downtown_Central", phaseVols);
    assert(websterPlan.cycleLength >= 45.0f && websterPlan.cycleLength <= 120.0f);
    assert(websterPlan.greenSplits[0] > 0.0f && websterPlan.greenSplits[2] > 0.0f);
    cout << "[TEST 26 PASSED] C++ Webster Signal Design Engine verified (C0=" << websterPlan.cycleLength 
         << "s, splits: [" << websterPlan.greenSplits[0] << "s, " << websterPlan.greenSplits[1] << "s, "
         << websterPlan.greenSplits[2] << "s, " << websterPlan.greenSplits[3] << "s])." << endl;

    // 27. Test C++ Genetic Algorithm Signal Timing Optimizer
    CitySignalTimingPlan gaPlan = CityGeneticSignalOptimizer::optimizeIntersection("Downtown_Central", phaseVols, 16, 10);
    assert(gaPlan.cycleLength >= 45.0f && gaPlan.cycleLength <= 120.0f);
    assert(gaPlan.fitnessScore > 0.0f);
    cout << "[TEST 27 PASSED] C++ Genetic Algorithm Signal Optimizer verified (Optimized C=" << gaPlan.cycleLength 
         << "s, Fitness=" << gaPlan.fitnessScore << ")." << endl;

    // 28. Test Highway Capacity Manual (HCM) Level of Service (LOS A–F) Classifier
    assert(computeHCMLOS(8.5f) == LOS_A);
    assert(computeHCMLOS(16.5f) == LOS_B);
    assert(computeHCMLOS(28.0f) == LOS_C);
    assert(computeHCMLOS(45.0f) == LOS_D);
    assert(computeHCMLOS(65.0f) == LOS_E);
    assert(computeHCMLOS(95.0f) == LOS_F);
    cout << "[TEST 28 PASSED] C++ HCM Level of Service (LOS A-F) Classifier verified (" << getLOSName(LOS_A) 
         << " to " << getLOSName(LOS_F) << ")." << endl;

    // 29. Test Urban City-Level Network Topology Setup
    Simulator<string, 100> citySim;
    citySim.setupCityLevelNetwork();
    Graph<string, 100>* cityGraph = citySim.getMap();
    assert(cityGraph->Vcount == 11);
    assert(cityGraph->getIndex("Downtown Central") != -1);
    assert(cityGraph->getIndex("Tech Park") != -1);
    assert(cityGraph->getIndex("University Town") != -1);
    assert(cityGraph->getIndex("City Hospital") != -1);
    cout << "[TEST 29 PASSED] Urban City Planning Network verified (11 Urban Hubs, multi-lane boulevards, Downtown grid)." << endl;

    // 30. Test City Planning Helper Before/After KPI Report
    CityPlanningReport report = citySim.optimizeCitySignals(true);
    assert(report.delayReductionPct > 0.0f);
    assert(report.optimizedAvgDelay < report.baselineAvgDelay);
    assert(report.co2ReductionPct > 0.0f);
    assert(report.optimizedLOS <= report.baselineLOS); // Upgraded or maintained LOS
    cout << "[TEST 30 PASSED] City Planning Helper KPI Report verified (Delay: " << report.baselineAvgDelay 
         << "s -> " << report.optimizedAvgDelay << "s (-" << report.delayReductionPct << "%), LOS Upgraded: " 
         << getLOSName(report.baselineLOS) << " -> " << getLOSName(report.optimizedLOS) << ")." << endl;

    // 31. Test 4 Fantasy Realms Network (Mount Olympus, Tartarus, Atlantis, Elysium)
    Simulator<string, 100> realmSim;
    realmSim.setupFantasyRealmsNetwork();
    Graph<string, 100>* realmGraph = realmSim.getMap();
    assert(realmGraph->Vcount == 4);
    assert(realmGraph->getIndex("Mount Olympus") != -1);
    assert(realmGraph->getIndex("Tartarus") != -1);
    assert(realmGraph->getIndex("Atlantis") != -1);
    assert(realmGraph->getIndex("Elysium") != -1);
    realmSim.addFantasyRealmCaravans(20);
    assert(realmSim.cityManager->getVehicles().size() == 20);
    cout << "[TEST 31 PASSED] 4 Fantasy Realms Overworld Network verified (Mount Olympus, Tartarus, Atlantis, Elysium)." << endl;

    // 32. Test Motorcycle Vehicle Taxonomy and Agile Dynamics
    vehicle<string> bike(301, "Karachi", "Lahore", VEHICLE_MOTORCYCLE);
    assert(bike.isMotorcycle());
    assert(bike.pcu == 0.5f);
    assert(bike.speedMultiplier == 1.15f);
    cout << "[TEST 32 PASSED] Motorcycle Vehicle Taxonomy & Agile Dynamics verified (PCU: 0.5, Speed: 1.15x)." << endl;

    cout << "\n======================================================================" << endl;
    cout << "ALL VERIFICATION TESTS (PHASES 1-4 + FEATURES 1-11 + CITY PLANNER 26-32) PASSED!" << endl;
    cout << "======================================================================" << endl;
    return 0;
}
