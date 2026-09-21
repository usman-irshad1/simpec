#pragma once
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <vector>
#include <deque>
#include <cmath>
#include "../core/Graph.h"
#include "../core/vehicle.h"

using namespace std;

template <class t, int size = 100> class Manager {
    float totalArrivedCount;
    float totalTime;
    list<vehicle<t>> array_of_vehicles;
    Graph<t, size>* map;
    deque<float> recentTravelTimes;
    float minTravelTime;
    float maxTravelTime;
    float totalFreeFlowTime;
    int windowSize;
    int totalReroutes;
    bool useAStarRouting;

    // Feature 6: Carbon Footprint & Fuel Analytics
    float totalFuelConsumedLiters;
    float totalCO2EmittedKg;
    float totalFuelWastedInJamLiters;

    // Feature 7: M-Tag Toll Plazas
    int totalTollCrossings;
    float totalTollRevenuePKR;

    // Feature 8: EV Fast Charging Stations
    vector<t> chargingStations;

    // Feature 9: Weather State
    WeatherCondition currentWeather;

    // Feature 10: 24-Hour Diurnal Clock
    float simClockHours; // 0.0f to 24.0f

public:
    Manager(Graph<t, size>* m) : map(m) {
        totalArrivedCount = 0;
        totalTime = 0;
        minTravelTime = 999999.0f;
        maxTravelTime = 0;
        totalFreeFlowTime = 0;
        windowSize = 100;
        totalReroutes = 0;
        useAStarRouting = true; // Default to A* Geodetic Haversine routing
        totalFuelConsumedLiters = 0.0f;
        totalCO2EmittedKg = 0.0f;
        totalFuelWastedInJamLiters = 0.0f;
        totalTollCrossings = 0;
        totalTollRevenuePKR = 0.0f;
        currentWeather = WEATHER_CLEAR;
        simClockHours = 8.0f; // Start at 08:00 AM (Morning rush)
    }

    void reset() {
        array_of_vehicles.clear();
        recentTravelTimes.clear();
        totalArrivedCount = 0;
        totalTime = 0;
        minTravelTime = 999999.0f;
        maxTravelTime = 0;
        totalFreeFlowTime = 0;
        totalReroutes = 0;
        totalFuelConsumedLiters = 0.0f;
        totalCO2EmittedKg = 0.0f;
        totalFuelWastedInJamLiters = 0.0f;
        totalTollCrossings = 0;
        totalTollRevenuePKR = 0.0f;
        chargingStations.clear();
        currentWeather = WEATHER_CLEAR;
        simClockHours = 8.0f;
    }

    void setUseAStarRouting(bool enable) { useAStarRouting = enable; }
    bool isUsingAStarRouting() const { return useAStarRouting; }

    list<t> computeRoute(t source, t destination) {
        if (useAStarRouting) {
            return map->aStarShortestPath(source, destination);
        }
        return map->shortest_Path_btw2_vericex_returing_list(source, destination);
    }

    int getVehicleCount() { return array_of_vehicles.size(); }
    float getArrivedCount() { return totalArrivedCount; }

    float getAvgTravelTime() {
        if (totalArrivedCount > 0) {
            return totalTime / totalArrivedCount;
        }
        else {
            return 0;
        }
    }
    float getWindowedAvgTravelTime() {
        if (recentTravelTimes.empty()) return 0;
        float wsum = 0;
        for (size_t i = 0; i < recentTravelTimes.size(); i++) wsum += recentTravelTimes[i];
        return wsum / recentTravelTimes.size();
    }
    float getMinTravelTime() {
        if (totalArrivedCount > 0) {
            return minTravelTime;
        }
        else {
            return 0;
        }
    }
    float getMaxTravelTime() { return maxTravelTime; }
    float getThroughput(float tick) {
        if (totalArrivedCount > 0 && tick > 0) {
            return totalArrivedCount / tick;
        }
        else {
            return 0;
        }
    }
    float getTotalDelay() { return totalTime - totalFreeFlowTime; }
    float getSystemCost() { return map->total_System_Cost(); }
    int getRerouteCount() { return totalReroutes; }

    // Feature 6 Getters
    float getTotalFuelConsumed() const { return totalFuelConsumedLiters; }
    float getTotalCO2Emitted() const { return totalCO2EmittedKg; }
    float getTotalFuelWasted() const { return totalFuelWastedInJamLiters; }

    // Feature 7 Getters
    int getTotalTollCrossings() const { return totalTollCrossings; }
    float getTotalTollRevenue() const { return totalTollRevenuePKR; }

    // Feature 8 Methods
    void addChargingStation(t node) {
        for (const auto& c : chargingStations) {
            if (c == node) return;
        }
        chargingStations.push_back(node);
    }
    bool hasChargingStation(t node) const {
        for (const auto& c : chargingStations) {
            if (c == node) return true;
        }
        return false;
    }

    // Feature 9 Methods
    WeatherCondition getWeather() const { return currentWeather; }
    void setGlobalWeather(WeatherCondition w) {
        currentWeather = w;
        for (int i = 0; i < size && i < map->Vcount; i++) {
            list<RoadDetails*> edges;
            edges = map->getEdges(map->getVertexAt(i), edges);
            for (auto* r : edges) {
                if (r) r->setWeather(w);
            }
        }
        ShortestPath();
    }

    // Feature 10 Methods
    float getClockHours() const { return simClockHours; }
    void setSimClock(float hour) {
        simClockHours = fmodf(hour, 24.0f);
        if (simClockHours < 0.0f) simClockHours += 24.0f;
    }
    void advanceClock(float dtMinutes) {
        simClockHours = fmodf(simClockHours + (dtMinutes / 60.0f), 24.0f);
        if (simClockHours < 0.0f) simClockHours += 24.0f;
    }
    bool isNight() const {
        return (simClockHours < 6.0f || simClockHours > 19.5f);
    }
    float getAmbientDarkness() const {
        if (simClockHours >= 7.0f && simClockHours <= 18.0f) return 0.0f;
        if (simClockHours > 18.0f && simClockHours <= 20.0f) return (simClockHours - 18.0f) / 2.0f * 0.85f;
        if (simClockHours >= 5.0f && simClockHours < 7.0f) return (7.0f - simClockHours) / 2.0f * 0.85f;
        return 0.85f;
    }

    // Feature 11 Methods: Dynamic CPEC Network Expansion
    void addCityNode(t cityName, float lat, float lon) {
        if (map->getIndex(cityName) == -1) {
            map->insertVertex(cityName);
            map->setVertexCoordinates(cityName, lat, lon);
        }
    }
    void addHighwayLink(t u, t v, float lengthKm, float speedKmh, float capacityVeh, bool isBidirectional = true) {
        int uIdx = map->getIndex(u);
        int vIdx = map->getIndex(v);
        if (uIdx == -1 || vIdx == -1) return;
        map->makeEdge(uIdx, vIdx, lengthKm, speedKmh, capacityVeh);
        if (isBidirectional) {
            map->makeEdge(vIdx, uIdx, lengthKm, speedKmh, capacityVeh);
        }
        ShortestPath();
    }
    void entrance() {
        int mov[size];
        for (int i = 0; i < size; i++) {
            mov[i] = 0;
        }
        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        while (car != array_of_vehicles.end()) {
            if (car->state != 0) {
                car++;
                continue;
            }

            if (car->path.size() < 2) {
                car->state = 2;
                car++;
                continue;
            }
            t u = car->path.front();
            auto it_p = car->path.begin();
            advance(it_p, 1);
            t v = *it_p;
            int index = map->getIndex(u);
            RoadDetails& road = map->getEdgeDetails(u, v);

            float discharge =
                road.DischargeAllowed(road.capacity, road.currentVehicles);
            if (road.signalState && mov[index] < discharge) {
                mov[index]++;
                if (car->inQueue) {
                    road.vehicleExitsQueue();
                    car->inQueue = false;
                }
                road.vehicleEntersRoad();
                car->timeRemaining = road.NonIdealtime();
                car->initialRoadTravelTime = car->timeRemaining;
                if (road.hasTollPlaza) {
                    float delay = road.processToll(car->hasMTag);
                    car->payToll(delay, road.tollRatePKR);
                    totalTollCrossings++;
                    totalTollRevenuePKR += road.tollRatePKR;
                }
                car->current = u;
                car->state = 1;
                car->selected_path.push_back(u);
                car->currentRoad.first = u;
                car->currentRoad.second = v;
            }
            else {
                if (!car->inQueue) {
                    road.vehicleJoinsQueue();
                    car->inQueue = true;
                    car->currentRoad.first = u;
                    car->currentRoad.second = v;
                }
                car->state = 0;
            }
            car++;
        }
    }

    void entraingfromQueetoEdge() {

        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        int mov[size];
        for (int i = 0; i < size; i++) {
            mov[i] = 0;
        }
        while (car != array_of_vehicles.end()) {
            if (car->path.size() < 2) {
                car->state = 2;
                car++;
                continue;
            }
            if (car->state == 0) {
                t u = car->path.front();
                auto it_p = car->path.begin();
                advance(it_p, 1);
                t v = *it_p;
                int index = map->getIndex(u);
                RoadDetails& road = map->getEdgeDetails(u, v);
                float discharge =
                    road.DischargeAllowed(road.capacity, road.currentVehicles);

                if (road.signalState && mov[index] < discharge) {
                    mov[index]++;
                    if (car->inQueue) {
                        road.vehicleExitsQueue();
                        car->inQueue = false;
                    }
                    road.vehicleEntersRoad();
                    car->timeRemaining = road.NonIdealtime();
                    car->initialRoadTravelTime = car->timeRemaining;
                    if (road.hasTollPlaza) {
                        float delay = road.processToll(car->hasMTag);
                        car->payToll(delay, road.tollRatePKR);
                        totalTollCrossings++;
                        totalTollRevenuePKR += road.tollRatePKR;
                    }
                    car->current = u;
                    car->state = 1;
                    car->currentRoad.first = u;
                    car->selected_path.push_back(u);
                    car->currentRoad.second = v;
                }
            }
            car++;
        }
    }

    list<vehicle<t>>& getVehicles() {
        return array_of_vehicles;
    }

    void recordArrival(float travelTime, const vector<t>& selectedPath, float fuel = 0.0f, float co2 = 0.0f, float wastedFuel = 0.0f) {
        this->totalArrivedCount++;
        this->totalTime += travelTime;
        this->totalFuelConsumedLiters += fuel;
        this->totalCO2EmittedKg += co2;
        this->totalFuelWastedInJamLiters += wastedFuel;

        if (travelTime < minTravelTime)
            minTravelTime = travelTime;
        if (travelTime > maxTravelTime)
            maxTravelTime = travelTime;

        recentTravelTimes.push_back(travelTime);
        if ((int)recentTravelTimes.size() > windowSize) {
            recentTravelTimes.pop_front();
        }

        for (size_t i = 0; i + 1 < selectedPath.size(); i++) {
            try {
                RoadDetails& rd =
                    map->getEdgeDetails(selectedPath[i], selectedPath[i + 1]);
                totalFreeFlowTime += rd.bestTime();
            }
            catch (...) {
            }
        }
    }

    void reached() {
        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        while (car != array_of_vehicles.end()) {
            bool deleted = false;
            if (car->state == 2) {
                car->selected_path.push_back(car->current);
                recordArrival(
                    car->timespent,
                    car->selected_path,
                    car->fuelConsumedLiters,
                    car->co2EmittedKg,
                    car->fuelWastedInJamLiters);
                car = array_of_vehicles.erase(car);
                deleted = true;

                continue;
            }
            if (car->state == 1) {

                if (car->path.size() == 2) {

                    if (car->timeRemaining <= 0) {
                        car->state = 2;
                        t u = car->path.front();
                        auto it_p = car->path.begin();
                        it_p++;
                        t v = *it_p;
                        car->selected_path.push_back(v);
                        RoadDetails& r = map->getEdgeDetails(u, v);
                        r.vehicleExitsRoad();
                        car->currentRoad.first = t();
                        car->currentRoad.second = t();
                        car->current = v;
                        car->timeRemaining = 0;
                        string x = car->getTakenPath();
                        string y = car->getstart_and_end();
                        Car_timing_file(car->timespent, car->id, x, y);
                        recordArrival(
                            car->timespent,
                            car->selected_path,
                            car->fuelConsumedLiters,
                            car->co2EmittedKg,
                            car->fuelWastedInJamLiters);
                        car = array_of_vehicles.erase(car);
                        deleted = true;
                    }
                }
            }
            if (!deleted) {
                car++;
            }
        }
    }

    void ShortestPath() {
        typename list<vehicle<t>>::iterator temp = array_of_vehicles.begin();
        while (temp != array_of_vehicles.end()) {
            if (temp->state == -1) {
                temp++;
            }
            else if (temp->state == 2) {
                temp++;
            }
            else {
                list<t> newPath = computeRoute(temp->current, temp->dest);

                if (temp->state == 0 && temp->inQueue && !newPath.empty() &&
                    !temp->path.empty()) {

                    t oldU = temp->path.front();
                    auto oldIt = temp->path.begin();
                    advance(oldIt, 1);
                    t newU = newPath.front();
                    auto newIt = newPath.begin();
                    advance(newIt, 1);

                    if (temp->path.size() >= 2 && newPath.size() >= 2) {
                        t oldV = *oldIt;
                        t newV = *newIt;
                        if (oldU != newU || oldV != newV) {

                            try {
                                RoadDetails& oldRoad = map->getEdgeDetails(
                                    temp->currentRoad.first, temp->currentRoad.second);
                                oldRoad.vehicleExitsQueue();
                            }
                            catch (...) {
                            }
                            temp->inQueue = false;
                        }
                    }
                }

                if (!newPath.empty() && newPath != temp->path) {
                    totalReroutes++;
                }
                temp->path = newPath;
                temp++;
            }
        }
    }

    void addVehicle(int id, t source, t destination, VehicleType vType = VEHICLE_CAR) {
        vehicle<t> newCar(id, source, destination, vType);
        newCar.path = computeRoute(source, destination);
        array_of_vehicles.push_back(newCar);
    }

    void blockRoad(t u, t v) {
        try {
            RoadDetails& rd = map->getEdgeDetails(u, v);
            rd.blockRoad();
            ShortestPath();
        }
        catch (...) {}
    }

    void unblockRoad(t u, t v) {
        try {
            RoadDetails& rd = map->getEdgeDetails(u, v);
            rd.unblockRoad();
            ShortestPath();
        }
        catch (...) {}
    }

    void setRoadCapacityFactor(t u, t v, float factor) {
        try {
            RoadDetails& rd = map->getEdgeDetails(u, v);
            rd.setCapacityFactor(factor);
            ShortestPath();
        }
        catch (...) {}
    }

    void injectPeakDemand(int currentTime, bool isMorningRush) {
        if (map->Vcount < 2) return;
        static int peakCarID = 60000;
        int carsToInject = 15;

        // Detect if this is the legacy national Pakistan highway map
        bool isPakistanMap = (map->getIndex("Karachi") != -1 && map->getIndex("Lahore") != -1 && map->getIndex("Multan") != -1);

        for (int i = 0; i < carsToInject; i++) {
            string start, end;
            if (isPakistanMap) {
                string metroHubs[] = { "Karachi", "Lahore", "Islamabad", "Multan" };
                string regionalCities[] = { "Sukkur", "Quetta", "DG Khan", "Faisalabad", "Peshawar", "Gujranwala", "Sialkot" };
                if (isMorningRush) {
                    start = regionalCities[rand() % 7];
                    end = metroHubs[rand() % 4];
                }
                else {
                    start = metroHubs[rand() % 4];
                    end = regionalCities[rand() % 7];
                }
            } else {
                // Dynamically pick distinct vertices from the currently active city/OSM/metro map
                int u = rand() % map->Vcount;
                int v = rand() % map->Vcount;
                int attempts = 0;
                while (attempts < 15 && (u == v || abs(u - v) < 2)) {
                    v = rand() % map->Vcount;
                    attempts++;
                }
                if (u == v) v = (u + 1) % map->Vcount;
                start = map->getVertexAt(u);
                end = map->getVertexAt(v);
            }

            if (start != end && !start.empty() && !end.empty()) {
                VehicleType vType = VEHICLE_CAR;
                int r = rand() % 100;
                if (r < 60) vType = VEHICLE_CAR;
                else if (r < 75) vType = VEHICLE_BUS;
                else if (r < 88) vType = VEHICLE_TRUCK;
                else if (r < 95) vType = VEHICLE_EV;
                else vType = VEHICLE_EMERGENCY;

                addVehicle(peakCarID++, start, end, vType);
            }
        }
    }

    void arrivalAtIntersection() {
        ShortestPath();
        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        int mov[size];
        for (int i = 0; i < size; i++) {
            mov[i] = 0;
        }

        while (car != array_of_vehicles.end()) {
            if (car->state == 1 && car->timeRemaining <= 0 && car->path.size() > 2) {
                t u = car->path.front();
                int index = map->getIndex(u);
                auto it = car->path.begin();
                advance(it, 1);
                t v = *it;
                RoadDetails& oldRoad = map->getEdgeDetails(u, v);
                oldRoad.vehicleExitsRoad();
                car->path.pop_front();
                car->current = v;

                // EV charging check: if vehicle is low battery and city has charging station
                if (car->isEV() && car->isLowBattery() && hasChargingStation(v)) {
                    car->rechargeBattery(75.0f);
                }

                t next_u = car->path.front();
                auto it2 = car->path.begin();
                it2++;
                t next_v = *it2;
                RoadDetails& road = map->getEdgeDetails(next_u, next_v);
                float discharge =
                    road.DischargeAllowed(road.capacity, road.currentVehicles);
                if (!road.signalState) {
                    road.vehicleJoinsQueue();
                    car->inQueue = true;
                    car->current = v;
                    car->state = 0;
                    car->currentRoad.first = next_u;
                    car->currentRoad.second = next_v;
                }

                else {
                    if (road.currentVehicles < road.capacity && mov[index] < discharge) {
                        mov[index]++;
                        road.vehicleEntersRoad();
                        car->timeRemaining = road.NonIdealtime();
                        car->initialRoadTravelTime = car->timeRemaining;
                        if (road.hasTollPlaza) {
                            float delay = road.processToll(car->hasMTag);
                            car->payToll(delay, road.tollRatePKR);
                            totalTollCrossings++;
                            totalTollRevenuePKR += road.tollRatePKR;
                        }
                        car->state = 1;
                        car->selected_path.push_back(next_u);
                        car->currentRoad.first = next_u;
                        car->currentRoad.second = next_v;
                    }
                    else {
                        road.vehicleJoinsQueue();
                        car->inQueue = true;
                        car->current = v;
                        car->state = 0;
                        car->currentRoad.first = next_u;
                        car->currentRoad.second = next_v;
                    }
                }
            }
            car++;
        }
    }

    void updateSignals() {
        for (int i = 0; i < size && i < map->Vcount; i++) {
            list<RoadDetails*> edges;
            edges = map->getEdges(map->getVertexAt(i), edges);
            if (edges.empty()) {
                continue;
            }

            // Check for Emergency Vehicle preemption on inbound approaches to vertex i
            RoadDetails* emergencyRoad = nullptr;
            for (auto& car : array_of_vehicles) {
                if (car.isEmergency() && (car.state == 0 || car.state == 1)) {
                    if (car.currentRoad.second == map->getVertexAt(i)) {
                        try {
                            emergencyRoad = &(map->getEdgeDetails(car.currentRoad.first, car.currentRoad.second));
                            break;
                        }
                        catch (...) {}
                    }
                }
            }

            RoadDetails* currentGreen = nullptr;
            RoadDetails* worstCandidate = nullptr;
            float max = -1.0f;
            int maxWaitingQueue = 0;

            for (auto* rd : edges) {
                rd->light.Timer(rd->signalState, 0.10f);

                if (rd->signalState == true && rd->light.isGreen()) {
                    currentGreen = rd;
                }

                float totalCost = rd->choosing() + rd->light.starvationCost();
                if (totalCost > max) {
                    max = totalCost;
                    worstCandidate = rd;
                }
                if (rd->queueCount > maxWaitingQueue) {
                    maxWaitingQueue = rd->queueCount;
                }
            }

            // Priority 1: Emergency preemption forces instant green corridor
            if (emergencyRoad != nullptr) {
                for (auto* rd : edges) {
                    if (rd != emergencyRoad) {
                        rd->change_to_red();
                    }
                }
                emergencyRoad->light.triggerEmergencyPreemption(emergencyRoad->signalState);
                continue;
            }

            // Priority 2: Max-Pressure & Q-Learning Adaptive Signal Switching
            if (worstCandidate != nullptr) {
                if (currentGreen == nullptr) {
                    // All-red clearance completed; turn worstCandidate green and all others red
                    worstCandidate->change_to_green();
                    for (auto* rd : edges) {
                        if (rd != worstCandidate) rd->change_to_red();
                    }
                }
                else if (currentGreen != worstCandidate) {
                    // Compute average downstream queue departing from intersection i
                    float downstreamQSum = 0.0f;
                    int outEdgeCount = 0;
                    auto* nodePtr = map->getNodes();
                    for (auto const& outEdge : nodePtr[i].Neighbors) {
                        downstreamQSum += (float)outEdge.weight.queueCount;
                        outEdgeCount++;
                    }
                    float downstreamAvg = (outEdgeCount > 0) ? (downstreamQSum / (float)outEdgeCount) : 0.0f;
                    float downstreamPressure = (float)worstCandidate->queueCount - downstreamAvg;

                    if (currentGreen->currentVehicles == 0 ||
                        currentGreen->light.shouldSwitchQAdaptive(currentGreen->queueCount, maxWaitingQueue, downstreamPressure)) {
                        currentGreen->change_to_yellow();
                        // All non-active approaches remain RED
                        for (auto* rd : edges) {
                            if (rd != currentGreen) rd->change_to_red();
                        }
                    }
                }
            }
        }
    }

    bool printPerformanceMetrics(float tickTime, float averageRush) {

        ofstream outfile("data/performance_metrics.txt", ios::app);
        if (outfile.is_open()) {
            outfile << "\n--- PERFORMANCE METRICS REPORT ---" << endl;
            outfile << "Tick: " << (int)tickTime
                << endl;

            if (this->totalArrivedCount > 0) {
                float actualAvgTravelTime =
                    (float)this->totalTime / this->totalArrivedCount;
                float throughput = (float)this->totalArrivedCount / tickTime;

                outfile << "Total Vehicles Arrived: " << (int)totalArrivedCount << endl;
                outfile << "Cumulative Avg Travel Time: " << actualAvgTravelTime
                    << " ticks" << endl;


                if (!recentTravelTimes.empty()) {
                    float windowSum = 0;
                    for (size_t i = 0; i < recentTravelTimes.size(); i++)
                        windowSum += recentTravelTimes[i];
                    float windowedAvg = windowSum / recentTravelTimes.size();
                    outfile << "Windowed Avg Travel Time (last "
                        << recentTravelTimes.size() << "): " << windowedAvg
                        << " ticks" << endl;
                }


                outfile << "Min Travel Time: " << minTravelTime << " ticks" << endl;
                outfile << "Max Travel Time: " << maxTravelTime << " ticks" << endl;
                if (recentTravelTimes.size() >= 20) {
                    vector<float> sorted(recentTravelTimes.begin(), recentTravelTimes.end());
                    sort(sorted.begin(), sorted.end());
                    int p95idx = (int)(sorted.size() * 0.95);
                    if (p95idx >= (int)sorted.size())
                        p95idx = (int)sorted.size() - 1;
                    outfile << "P95 Travel Time: " << sorted[p95idx] << " ticks" << endl;
                }

                outfile << "System Throughput: " << throughput << " vehicles/tick"
                    << endl;

                float totalDelay = this->totalTime - this->totalFreeFlowTime;
                outfile << "Total Delay (actual - free_flow): " << totalDelay
                    << " ticks" << endl;
            }
            else {
                outfile << "No vehicles have finished yet." << endl;
            }

            outfile << "Total System Cost: " << map->total_System_Cost() << endl;
            outfile << "Average Rush Level: " << averageRush << endl;
            outfile << "Total Reroutes: " << totalReroutes << endl;
            outfile << "Active Vehicles: " << array_of_vehicles.size() << endl;
            outfile << "Total Fuel Consumed: " << totalFuelConsumedLiters << " Liters" << endl;
            outfile << "Total CO2 Emitted: " << totalCO2EmittedKg << " kg" << endl;
            outfile << "Fuel Wasted in Congestion: " << totalFuelWastedInJamLiters << " Liters" << endl;
            outfile << "Total Toll Crossings: " << totalTollCrossings << " (Revenue: PKR " << (int)totalTollRevenuePKR << ")" << endl;

            string roadSnap = map->printRoadSnapshot();
            if (!roadSnap.empty()) {
                outfile << "Active Roads:" << endl;
                outfile << roadSnap;
            }

            outfile << "----------------------------------" << endl;
            return (averageRush == 0);
        }
        else {
            cerr << "Unable to open data/performance_metrics.txt for writing." << endl;
            return false;
        }
    }

    void printCSVHeader() {
        ofstream csv("data/metrics.csv", ios::trunc);
        if (csv.is_open()) {
            csv << "tick,arrived,cumulative_avg,windowed_avg,min_time,max_time,p95_"
                "time,throughput,rush_level,system_cost,total_delay,active_"
                "vehicles,reroutes"
                << endl;
            csv.close();
        }
    }
    // do this again
    void printCSVRow(float tickTime, float averageRush) {
        ofstream csv("data/metrics.csv", ios::app);
        if (csv.is_open()) {
            float cumAvg =
                (totalArrivedCount > 0) ? totalTime / totalArrivedCount : 0;
            float windowedAvg = 0;
            float p95 = 0;
            if (!recentTravelTimes.empty()) {
                float wsum = 0;
                for (size_t i = 0; i < recentTravelTimes.size(); i++)
                    wsum += recentTravelTimes[i];
                windowedAvg = wsum / recentTravelTimes.size();
            }
            if (recentTravelTimes.size() >= 20) {
                vector<float> sorted(recentTravelTimes.begin(), recentTravelTimes.end());
                sort(sorted.begin(), sorted.end());
                int p95idx = (int)(sorted.size() * 0.95);
                if (p95idx >= (int)sorted.size())
                    p95idx = (int)sorted.size() - 1;
                p95 = sorted[p95idx];
            }
            float throughput = (tickTime > 0) ? totalArrivedCount / tickTime : 0;
            float totalDelay = this->totalTime - this->totalFreeFlowTime;
            float sysCost = map->total_System_Cost();

            csv << fixed << setprecision(2)
                << tickTime << ","
                << (int)totalArrivedCount << ","
                << cumAvg << ","
                << windowedAvg << ","
                << minTravelTime << ","
                << maxTravelTime << ","
                << p95 << ","
                << throughput << ","
                << averageRush << ","
                << sysCost << ","
                << totalDelay << ","
                << array_of_vehicles.size() << ","
                << totalReroutes
                << endl;
            csv.close();
        }
    }

    void Car_timing_file(float time, int id, string path,
        string initial_and_final) {
        ofstream outfile("data/car_timings.txt", ios::app);
        if (outfile.is_open()) {
            outfile << "Car " << left << setw(8) << id << left << setw(35)
                << ("(" + initial_and_final + ")") << left << setw(30)
                << ("arrived in " + to_string(time) + " ticks.")
                << "Path: " << path << endl;
        }
        else {
            cerr << "Unable to open data/car_timings.txt for writing." << endl;
        }
    }
    void clearMetricsFile() {
        ofstream outfile("data/performance_metrics.txt", ios::trunc);
        if (outfile.is_open()) {
            outfile << "--- NEW SIMULATION SESSION ---" << endl;
            outfile.close();
        }
        ofstream outfile1("data/car_timings.txt", ios::trunc);
        if (outfile1.is_open()) {
            outfile1 << "--- NEW SIMULATION SESSION ---" << endl;
            outfile1.close();
        }
        ofstream outfile2("data/map.txt", ios::trunc);
        if (outfile2.is_open()) {
            outfile2 << "--- NEW SIMULATION SESSION ---" << endl;
            outfile2.close();
        }
        printCSVHeader();
    }

    void printNetwork() {
        ofstream outfile("data/map.txt", ios::app);
        if (outfile.is_open()) {
            outfile << map->printGrapgh();
        }
    }

    void PrintAllConnetedCityFromAcity() { map->PrintLevels(); }

    void checkBrokenPath() {
        typename list<vehicle<t>>::iterator car = array_of_vehicles.begin();
        while (car != array_of_vehicles.end()) {
            if (car->path.empty()) {

                cout << "Removing Vehicle " << car->id << ": No viable path to "
                    << car->dest << endl;
                if (car->state == 1) {
                    RoadDetails& d = map->getEdgeDetails(car->currentRoad.first,
                        car->currentRoad.second);
                    d.vehicleExitsRoad();
                }

                if (car->inQueue) {
                    try {
                        RoadDetails& d = map->getEdgeDetails(car->currentRoad.first,
                            car->currentRoad.second);
                        d.vehicleExitsQueue();
                    }
                    catch (...) {
                    }
                    car->inQueue = false;
                }
                car = array_of_vehicles.erase(car);
            }
            else {
                car++;
            }
        }
    }

    void physics() {
        float x = 1;
        int total_time = 0;
        cout << "time step = 6 minutes ticks per loop" << endl;

        while (array_of_vehicles.size() > 0) {
            if (total_time % 50 == 0 && total_time < 4000) {
                injectSinusoidalTraffic(total_time);
            }
            total_time++;
            advanceClock(2.0f); // 2 minutes simulated per tick
            updateSignals();

            for (auto& car : array_of_vehicles) {
                if (car.state == 1) {
                    float step = 2.0f * car.speedMultiplier;
                    car.timeRemaining -= step;
                    car.timespent += 2.0f;
                    car.updateFuelAndEmissions(2.0f, false, car.currentSpeed);
                }
                else if (car.state == 0) {
                    car.timespent += 2.0f;
                    car.updateFuelAndEmissions(2.0f, true, 0.0f);
                }

                if (car.timeRemaining < 0) {
                    car.timeRemaining = 0;
                }
            }

            if (total_time % 100 == 0) {
                printPerformanceMetrics(total_time, x);
                printCSVRow(total_time, x);
            }

            reached();
            arrivalAtIntersection();
            entraingfromQueetoEdge();
            entrance();
            checkBrokenPath();

            x = map->AverageRush();

            if (total_time > 7000) {
                cout << "Simulation timed out after 7,000 ticks." << endl;
                break;
            }
        }

        cout << "\n--- FINAL SIMULATION REPORT ---" << endl;
        printPerformanceMetrics(total_time, x);
        printCSVRow(total_time, x);
        cout << "All reachable cars have reached their destinations." << endl;
        cout << this->array_of_vehicles.size()
            << " left in the system (unreachable or still en route)." << endl;
    }

    void injectSinusoidalTraffic(int currentTime) {
        string cities[] = { "Karachi", "Sukkur", "Quetta",   "DG Khan",
                           "Multan",  "Lahore", "Islamabad" };
        float amplitude = 10.0;
        float baseline = 5.0;
        float frequency = 0.01;
        int carsToAdd =
            (int)(amplitude * (sin(frequency * currentTime) + 1) + baseline);
        static int globalCarID = 1000;

        for (int i = 0; i < carsToAdd; i++) {
            string start = cities[rand() % 4];
            string end = cities[4 + (rand() % 3)];
            if (start != end) {
                addVehicle(globalCarID++, start, end);
            }
        }
    }
};

template <class t, int size = 100>
using Manger = Manager<t, size>;
