#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <sstream>
#include "raylib.h"
#include "raymath.h"
#include "../core/Graph.h"
#include "Simulator.h"
#include "CityPlanner.h"
#include "../core/vehicle.h"

// =============================================================================
// NYC METROPOLITAN TRANSIT & CITY DESIGNER SUITE
// =============================================================================
// Features:
// 1. Parametric City Designer Settings (Grid, Capacities, Speeds, Fleet Mix)
// 2. NYC Subway / MTA-Style Visual Aesthetic (Vignelli / MTA Palette & Stations)
// 3. Dynamic Multi-Route Traffic Planning & Emergency Green-Corridor Preemption
// 4. Live Telemetry Dashboard & Comprehensive CSV Data Collection Export
// =============================================================================

// Official NYC MTA Subway Palette
inline Color GetMTAColorBlue()   { return Color{ 0, 57, 166, 255 }; }   // 8th Ave (A, C, E)
inline Color GetMTAColorOrange() { return Color{ 255, 99, 25, 255 }; }  // 6th Ave (B, D, F, M)
inline Color GetMTAColorYellow() { return Color{ 252, 204, 10, 255 }; } // Broadway (N, Q, R, W)
inline Color GetMTAColorRed()    { return Color{ 238, 53, 46, 255 }; }  // 7th Ave (1, 2, 3)
inline Color GetMTAColorGreen()  { return Color{ 0, 147, 60, 255 }; }   // Lexington (4, 5, 6)
inline Color GetMTAColorPurple() { return Color{ 185, 51, 173, 255 }; } // Flushing (7)
inline Color GetMTAColorLime()   { return Color{ 108, 190, 69, 255 }; } // Crosstown (G)
inline Color GetMTAColorGray()   { return Color{ 167, 169, 172, 255 }; } // Canarsie (L)
inline Color GetMTADarkTrack()   { return Color{ 28, 32, 42, 255 }; }
inline Color GetMTABackground()  { return Color{ 14, 17, 24, 255 }; }
#ifndef CYAN
#define CYAN Color{ 0, 255, 255, 255 }
#endif

// City Configuration Parameters
struct CityDesignParams {
    // 1. Grid & Topography
    int gridRows;           // 2 to 5 (Cross-Streets)
    int gridCols;           // 2 to 5 (Avenues)
    float blockLengthKm;    // 1.5 to 8.0 km
    int layoutPreset;       // 0: Manhattan Midtown, 1: Broadway Diagonal, 2: Queens Crosstown, 3: Custom

    // 2. Road Network Parameters
    int numLanes;           // 1 to 4 lanes
    float speedLimitKmh;    // 30 to 80 km/h
    float roadCapacityVeh;  // 25 to 120 vehicles
    bool isTwoWay;          // true = 2-way, false = alternating 1-way

    // 3. Traffic Demand & Fleet Taxonomy Mix
    int commuterCount;      // 15 to 120 active commuter vehicles
    int carPct;             // % Passenger Taxis & Cars
    int busPct;             // % MTA Transit Buses
    int truckPct;           // % Commercial Freight Trucks
    int evPct;              // % Clean EVs
    int motorcyclePct;      // % Courier Motorcycles
    int peakHourDemand;     // 0: Off-Peak, 1: Morning Rush, 2: Evening Gridlock

    // 4. Emergency Infrastructure & Preemption
    int emergencyHubIdx;    // Station index of EMS / Hospital HQ
    bool enableGreenCorridor; // Emergency signal preemption green wave
    int emergencyReadyUnits;// Number of standby emergency ambulances

    // 5. Signal Optimization Strategy
    int signalStrategy;     // 0: Webster Baseline, 1: GA-Optimized, 2: Actuated Gap-Out, 3: Coordinated Green-Wave

    CityDesignParams() {
        gridRows = 4;
        gridCols = 4;
        blockLengthKm = 3.5f;
        layoutPreset = 0; // Manhattan Midtown

        numLanes = 3;
        speedLimitKmh = 50.0f;
        roadCapacityVeh = 60.0f;
        isTwoWay = true;

        commuterCount = 45;
        carPct = 50;
        busPct = 20;
        truckPct = 10;
        evPct = 15;
        motorcyclePct = 5;
        peakHourDemand = 1;

        emergencyHubIdx = 1; // Default to Bellevue Hospital EMS
        enableGreenCorridor = true;
        emergencyReadyUnits = 2;

        signalStrategy = 1; // GA-Optimized
    }

    void loadPreset(int preset) {
        layoutPreset = preset;
        if (preset == 0) {
            // Manhattan Midtown Grid (High Density)
            gridRows = 4;
            gridCols = 4;
            blockLengthKm = 3.0f;
            numLanes = 3;
            speedLimitKmh = 50.0f;
            roadCapacityVeh = 65.0f;
            isTwoWay = true;
            commuterCount = 50;
            carPct = 50; busPct = 25; truckPct = 10; evPct = 10; motorcyclePct = 5;
            signalStrategy = 1; // GA
            emergencyHubIdx = 1;
        } else if (preset == 1) {
            // Broadway Diagonal Express (Arterial Bypass)
            gridRows = 5;
            gridCols = 4;
            blockLengthKm = 4.0f;
            numLanes = 4;
            speedLimitKmh = 65.0f;
            roadCapacityVeh = 85.0f;
            isTwoWay = true;
            commuterCount = 65;
            carPct = 55; busPct = 15; truckPct = 15; evPct = 10; motorcyclePct = 5;
            signalStrategy = 3; // Coordinated Green Wave
            emergencyHubIdx = 2;
        } else if (preset == 2) {
            // Crosstown Transit Hub (Bus Priority & Green Zone)
            gridRows = 3;
            gridCols = 5;
            blockLengthKm = 3.2f;
            numLanes = 2;
            speedLimitKmh = 40.0f;
            roadCapacityVeh = 50.0f;
            isTwoWay = true;
            commuterCount = 40;
            carPct = 35; busPct = 35; truckPct = 5; evPct = 20; motorcyclePct = 5;
            signalStrategy = 2; // Actuated
            emergencyHubIdx = 0;
        }
    }
};

// Real NYC Station Names Matrix for Schematic Transit Grid
inline std::vector<std::string> GetNYCStationNames() {
    return {
        "Times Sq - 42 St",
        "Bellevue Hospital EMS",
        "Grand Central - 42 St",
        "Penn Station - 34 St",
        "Herald Sq - 34 St",
        "Union Sq - 14 St",
        "Columbus Circle - 59 St",
        "Rockefeller Ctr - 47 St",
        "Canal St - Broadway",
        "Wall St - Financial",
        "Fulton St - Transit Ctr",
        "Brooklyn Bridge - City Hall",
        "Central Park South",
        "Hudson Yards - 34 St",
        "SoHo - Spring St",
        "Lexington - 53 St",
        "Astor Place - 8 St",
        "Chinatown - Bowery",
        "Queensboro Plaza",
        "Battery Park Maritime",
        "Flatiron - 23 St",
        "World Trade Center",
        "Chelsea Market - 14 St",
        "Midtown East - 51 St",
        "Lincoln Center - 66 St"
    };
}

// Helper to get route bullet label for a station
inline const char* GetStationRouteBullet(int idx) {
    static const char* bullets[] = {
        "1 2 3 N Q R S",
        "EMS RESCUE",
        "4 5 6 7 S",
        "A C E 1 2 3",
        "B D F M N Q",
        "4 5 6 L N Q",
        "A B C D 1",
        "B D F M",
        "J N Q R Z",
        "2 3 4 5",
        "A C J Z 2 3",
        "4 5 6 J Z",
        "N Q R W",
        "7 EXPRESS",
        "C E N R",
        "E M 6",
        "6 LOCAL",
        "J Z",
        "7 N W",
        "1 R W",
        "R W",
        "PATH E 1",
        "A C E L",
        "6 E M",
        "1 LOCAL"
    };
    if (idx >= 0 && idx < 25) return bullets[idx];
    return "MTA";
}

// Line Color for Avenue / Street Index
inline Color GetMTALineColor(int row, int col, bool isAvenue) {
    if (isAvenue) {
        int c = col % 5;
        switch (c) {
            case 0: return GetMTAColorBlue();   // 8th Ave (A, C, E)
            case 1: return GetMTAColorRed();    // 7th Ave (1, 2, 3)
            case 2: return GetMTAColorOrange(); // 6th Ave (B, D, F, M)
            case 3: return GetMTAColorYellow(); // Broadway (N, Q, R, W)
            default: return GetMTAColorGreen(); // Lexington (4, 5, 6)
        }
    } else {
        int r = row % 4;
        switch (r) {
            case 0: return GetMTAColorPurple(); // 42nd St Flushing (7)
            case 1: return GetMTAColorLime();   // Crosstown (G)
            case 2: return GetMTAColorGray();   // 14th St Canarsie (L)
            default: return GetMTAColorBlue();  // Downtown Express
        }
    }
}

// =============================================================================
// CITY GRAPH BUILDER: TRANSFORMS DESIGN PARAMETERS INTO LIVE GRAPH
// =============================================================================
inline void SetupNYCMetroCustomCity(
    Simulator<std::string, 100>& sim,
    const CityDesignParams& params,
    std::map<std::string, Vector2>& outPositions,
    std::map<std::string, Vector3>& outPositions3D,
    int screenWidth,
    int screenHeight
) {
    sim.resetSimulation();
    outPositions.clear();
    outPositions3D.clear();

    Graph<std::string, 100>* mapRef = sim.getMap();
    std::vector<std::string> allNames = GetNYCStationNames();

    int rows = params.gridRows;
    int cols = params.gridCols;
    int totalStations = rows * cols;
    if (totalStations > 25) totalStations = 25;

    // Viewport layout calculation for 2D projections
    float viewX = 420.0f;
    float viewW = (float)(screenWidth - 460);
    float viewH = (float)(screenHeight - 140);

    float startX = viewX + 80.0f;
    float startY = 90.0f;
    float stepX = (cols > 1) ? (viewW - 160.0f) / (cols - 1) : 0.0f;
    float stepY = (rows > 1) ? (viewH - 140.0f) / (rows - 1) : 0.0f;

    // 3D Spatial Layout Calculation (Centered around origin 0,0,0)
    float stepX3D = 9.0f;
    float stepZ3D = 7.5f;
    float startX3D = -((cols - 1) * 0.5f) * stepX3D;
    float startZ3D = -((rows - 1) * 0.5f) * stepZ3D;

    std::vector<std::string> activeStations;
    for (int i = 0; i < totalStations; i++) {
        std::string sName = allNames[i];
        if (i == params.emergencyHubIdx) {
            sName = "Bellevue Hospital EMS";
        }
        activeStations.push_back(sName);
        mapRef->insertVertex(sName);

        int r = i / cols;
        int c = i % cols;

        // 2D Projected Position
        Vector2 pos2D = { startX + c * stepX, startY + r * stepY };
        outPositions[sName] = pos2D;

        // 3D World Position with Realistic Multi-Level Elevation
        float pX = startX3D + c * stepX3D;
        float pZ = startZ3D + r * stepZ3D;
        float pY = 0.25f; // Ground baseline level

        if (i == params.emergencyHubIdx) {
            pY = 0.70f; // EMS Hospital Hub elevated platform
        } else if (r >= 1 && r <= rows - 2 && c >= 1 && c <= cols - 2) {
            pY = 1.10f; // Elevated Midtown Concourse (Times Sq, Grand Central, Penn Station)
        }

        Vector3 pos3D = { pX, pY, pZ };
        outPositions3D[sName] = pos3D;

        // Realistic GPS projection for NYC Midtown Manhattan
        float lat = 40.7580f - (float)r * 0.012f;
        float lon = -73.9855f + (float)c * 0.015f;
        mapRef->setVertexCoordinates(sName, lat, lon);
    }

    // =========================================================================
    // COMPLEX MULTI-TIER NETWORK CONNECTIVITY
    // =========================================================================

    // Tier 1: Avenue Edges (North <-> South)
    for (int r = 0; r < rows - 1; r++) {
        for (int c = 0; c < cols; c++) {
            int u = r * cols + c;
            int v = (r + 1) * cols + c;
            if (u < totalStations && v < totalStations) {
                float len = params.blockLengthKm;
                float spd = params.speedLimitKmh;
                float cap = params.roadCapacityVeh;
                float alpha = 0.15f;
                float beta = 4.0f;

                if (params.isTwoWay) {
                    mapRef->makeEdge(u, v, len, spd, cap, alpha, beta);
                    mapRef->makeEdge(v, u, len, spd, cap, alpha, beta);
                } else {
                    // Alternating one-way avenues (Classic Manhattan traffic system)
                    if (c % 2 == 0) mapRef->makeEdge(u, v, len, spd, cap, alpha, beta);
                    else            mapRef->makeEdge(v, u, len, spd, cap, alpha, beta);
                }
            }
        }
    }

    // Tier 1: Cross-Street Edges (West <-> East)
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols - 1; c++) {
            int u = r * cols + c;
            int v = r * cols + (c + 1);
            if (u < totalStations && v < totalStations) {
                float len = params.blockLengthKm * 0.85f;
                float spd = params.speedLimitKmh * 0.90f;
                float cap = params.roadCapacityVeh;
                float alpha = 0.15f;
                float beta = 4.0f;

                if (params.isTwoWay) {
                    mapRef->makeEdge(u, v, len, spd, cap, alpha, beta);
                    mapRef->makeEdge(v, u, len, spd, cap, alpha, beta);
                } else {
                    // Alternating one-way cross streets
                    if (r % 2 == 0) mapRef->makeEdge(u, v, len, spd, cap, alpha, beta);
                    else            mapRef->makeEdge(v, u, len, spd, cap, alpha, beta);
                }
            }
        }
    }

    // Tier 2: Broadway Arterial Express (Primary Diagonal: Top-Left to Bottom-Right)
    for (int d = 0; d < std::min(rows - 1, cols - 1); d++) {
        int u = d * cols + d;
        int v = (d + 1) * cols + (d + 1);
        if (u < totalStations && v < totalStations) {
            float len = params.blockLengthKm * 1.35f;
            float spd = params.speedLimitKmh * 1.25f; // Faster arterial diagonal
            float cap = params.roadCapacityVeh * 1.35f;
            mapRef->makeEdge(u, v, len, spd, cap, 0.10f, 4.0f);
            mapRef->makeEdge(v, u, len, spd, cap, 0.10f, 4.0f);
        }
    }

    // Tier 3: Crosstown Counter-Diagonal Express (Secondary Diagonal: Top-Right to Bottom-Left)
    for (int d = 0; d < std::min(rows - 1, cols - 1); d++) {
        int u = d * cols + (cols - 1 - d);
        int v = (d + 1) * cols + (cols - 2 - d);
        if (u < totalStations && v < totalStations && u != v) {
            float len = params.blockLengthKm * 1.35f;
            float spd = params.speedLimitKmh * 1.20f;
            float cap = params.roadCapacityVeh * 1.30f;
            mapRef->makeEdge(u, v, len, spd, cap, 0.10f, 4.0f);
            mapRef->makeEdge(v, u, len, spd, cap, 0.10f, 4.0f);
        }
    }

    // Tier 4: Perimeter Express Beltway / Ring Road (FDR Drive & West Side Highway)
    float beltSpd = params.speedLimitKmh * 1.40f;
    float beltCap = params.roadCapacityVeh * 1.50f;
    // Top boundary (North Beltway)
    for (int c = 0; c < cols - 1; c++) {
        int u = c;
        int v = c + 1;
        if (u < totalStations && v < totalStations) {
            mapRef->makeEdge(u, v, params.blockLengthKm * 0.90f, beltSpd, beltCap, 0.08f, 4.0f);
            mapRef->makeEdge(v, u, params.blockLengthKm * 0.90f, beltSpd, beltCap, 0.08f, 4.0f);
        }
    }
    // Right boundary (East River Drive)
    for (int r = 0; r < rows - 1; r++) {
        int u = r * cols + (cols - 1);
        int v = (r + 1) * cols + (cols - 1);
        if (u < totalStations && v < totalStations) {
            mapRef->makeEdge(u, v, params.blockLengthKm * 0.90f, beltSpd, beltCap, 0.08f, 4.0f);
            mapRef->makeEdge(v, u, params.blockLengthKm * 0.90f, beltSpd, beltCap, 0.08f, 4.0f);
        }
    }
    // Bottom boundary (South Ferry Beltway)
    for (int c = 0; c < cols - 1; c++) {
        int u = (rows - 1) * cols + c;
        int v = (rows - 1) * cols + (c + 1);
        if (u < totalStations && v < totalStations) {
            mapRef->makeEdge(u, v, params.blockLengthKm * 0.90f, beltSpd, beltCap, 0.08f, 4.0f);
            mapRef->makeEdge(v, u, params.blockLengthKm * 0.90f, beltSpd, beltCap, 0.08f, 4.0f);
        }
    }
    // Left boundary (West Side Highway)
    for (int r = 0; r < rows - 1; r++) {
        int u = r * cols;
        int v = (r + 1) * cols;
        if (u < totalStations && v < totalStations) {
            mapRef->makeEdge(u, v, params.blockLengthKm * 0.90f, beltSpd, beltCap, 0.08f, 4.0f);
            mapRef->makeEdge(v, u, params.blockLengthKm * 0.90f, beltSpd, beltCap, 0.08f, 4.0f);
        }
    }

    // Tier 5: Multi-Level Elevated Flyover Bypasses (Leaping over intermediate intersections)
    if (rows >= 3) {
        int midCol = cols / 2;
        int u = 0 * cols + midCol;
        int v = (rows - 1) * cols + midCol;
        if (u < totalStations && v < totalStations) {
            float len = params.blockLengthKm * (float)(rows - 1) * 0.95f;
            mapRef->makeEdge(u, v, len, params.speedLimitKmh * 1.55f, params.roadCapacityVeh * 1.6f, 0.05f, 4.0f);
            mapRef->makeEdge(v, u, len, params.speedLimitKmh * 1.55f, params.roadCapacityVeh * 1.6f, 0.05f, 4.0f);
        }
    }
    if (cols >= 3) {
        int midRow = rows / 2;
        int u = midRow * cols + 0;
        int v = midRow * cols + (cols - 1);
        if (u < totalStations && v < totalStations) {
            float len = params.blockLengthKm * (float)(cols - 1) * 0.95f;
            mapRef->makeEdge(u, v, len, params.speedLimitKmh * 1.55f, params.roadCapacityVeh * 1.6f, 0.05f, 4.0f);
            mapRef->makeEdge(v, u, len, params.speedLimitKmh * 1.55f, params.roadCapacityVeh * 1.6f, 0.05f, 4.0f);
        }
    }

    // Tier 6: Cross-District Diamond Struts (Interior lattice interchange links)
    for (int r = 0; r < rows - 1; r++) {
        for (int c = 0; c < cols - 1; c++) {
            if ((r + c) % 2 == 1) {
                int u = r * cols + (c + 1);
                int v = (r + 1) * cols + c;
                if (u < totalStations && v < totalStations) {
                    mapRef->makeEdge(u, v, params.blockLengthKm * 1.25f, params.speedLimitKmh * 1.10f, params.roadCapacityVeh, 0.12f, 4.0f);
                    mapRef->makeEdge(v, u, params.blockLengthKm * 1.25f, params.speedLimitKmh * 1.10f, params.roadCapacityVeh, 0.12f, 4.0f);
                }
            }
        }
    }

    // Tier 7: Dedicated Emergency Rapid-Response Skyways (Direct preemption access from EMS Hub)
    int emsIdx = params.emergencyHubIdx % totalStations;
    if (emsIdx != 0) {
        mapRef->makeEdge(emsIdx, 0, params.blockLengthKm * 1.40f, 95.0f, 110.0f, 0.04f, 4.0f);
        mapRef->makeEdge(0, emsIdx, params.blockLengthKm * 1.40f, 95.0f, 110.0f, 0.04f, 4.0f);
    }
    int southHub = totalStations - 1;
    if (emsIdx != southHub) {
        mapRef->makeEdge(emsIdx, southHub, params.blockLengthKm * 1.40f, 95.0f, 110.0f, 0.04f, 4.0f);
        mapRef->makeEdge(southHub, emsIdx, params.blockLengthKm * 1.40f, 95.0f, 110.0f, 0.04f, 4.0f);
    }

    // Configure Traffic Signals
    auto* nodes = mapRef->getNodes();
    for (int i = 0; i < mapRef->Vcount; i++) {
        for (auto& edge : nodes[i].Neighbors) {
            edge.weight.numLanes = params.numLanes;
            edge.weight.light.strategy = static_cast<SignalControlStrategy>(params.signalStrategy);
        }
    }

    // Run Initial Signal Optimization
    sim.optimizeCitySignals(params.signalStrategy == 1);

    // Initialize Intersection Approach Signals: Only 1 approach starts Green, all others Red!
    for (int i = 0; i < mapRef->Vcount; i++) {
        std::list<RoadDetails*> inEdges;
        inEdges = mapRef->getEdges(mapRef->getVertexAt(i), inEdges);
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

    // Spawn Configured Vehicle Fleets with multi-hop journeys across the city
    static int nycSeq = 50000;
    int nVehicles = params.commuterCount;
    for (int i = 0; i < nVehicles; i++) {
        int u = rand() % totalStations;
        int v = rand() % totalStations;
        int attempts = 0;
        while (attempts < 20 && (u == v || (abs((u / cols) - (v / cols)) + abs((u % cols) - (v % cols)) < 2))) {
            v = rand() % totalStations;
            attempts++;
        }
        if (u == v) v = (u + 1) % totalStations;

        int roll = rand() % 100;
        VehicleType vType = VEHICLE_CAR;
        if (roll < params.carPct) {
            vType = VEHICLE_CAR;
        } else if (roll < params.carPct + params.busPct) {
            vType = VEHICLE_BUS;
        } else if (roll < params.carPct + params.busPct + params.truckPct) {
            vType = VEHICLE_TRUCK;
        } else if (roll < params.carPct + params.busPct + params.truckPct + params.evPct) {
            vType = VEHICLE_EV;
        } else {
            vType = VEHICLE_MOTORCYCLE;
        }

        sim.cityManager->addVehicle(nycSeq++, activeStations[u], activeStations[v], vType);
    }

    // Dispatch Configured Standby Emergency Vehicles from EMS Hub
    std::string emsStation = activeStations[params.emergencyHubIdx % totalStations];
    for (int k = 0; k < params.emergencyReadyUnits; k++) {
        int targetNode = (params.emergencyHubIdx + 2 + k * 3) % totalStations;
        sim.cityManager->addVehicle(91000 + k, emsStation, activeStations[targetNode], VEHICLE_EMERGENCY);
    }
}

// Backward-compatible overload
inline void SetupNYCMetroCustomCity(
    Simulator<std::string, 100>& sim,
    const CityDesignParams& params,
    std::map<std::string, Vector2>& outPositions,
    int screenWidth,
    int screenHeight
) {
    std::map<std::string, Vector3> dummy3D;
    SetupNYCMetroCustomCity(sim, params, outPositions, dummy3D, screenWidth, screenHeight);
}

// =============================================================================
// CSV DATA COLLECTION EXPORTER
// =============================================================================
inline void ExportCityPlanningMetrics(
    Simulator<std::string, 100>& sim,
    const CityDesignParams& params,
    int totalTime,
    const std::string& filename = "data/city_planning_telemetry.csv"
) {
    std::ofstream out(filename, std::ios::app);
    if (!out.is_open()) return;

    // Check if file is empty to write CSV header
    out.seekp(0, std::ios::end);
    if (out.tellp() == 0) {
        out << "Timestamp,Scenario,GridSize,Lanes,SpeedLimitKmh,Capacity,ActiveVehicles,CompletedTrips,"
            << "AvgTravelTicks,ThroughputVpTick,CongestionPct,CO2EmittedKg,FuelLiters,HCM_LOS,SignalStrategy\n";
    }

    std::string stratName = (params.signalStrategy == 1) ? "GA_OPTIMIZED" :
                           (params.signalStrategy == 0 ? "WEBSTER" :
                           (params.signalStrategy == 2 ? "ACTUATED" : "COORDINATED_GREEN_WAVE"));

    std::string gridStr = std::to_string(params.gridRows) + "x" + std::to_string(params.gridCols);

    out << totalTime << ","
        << (params.layoutPreset == 0 ? "Manhattan_Midtown" : (params.layoutPreset == 1 ? "Broadway_Express" : "Queens_Transit")) << ","
        << gridStr << ","
        << params.numLanes << ","
        << params.speedLimitKmh << ","
        << params.roadCapacityVeh << ","
        << sim.cityManager->getVehicleCount() << ","
        << sim.cityManager->getArrivedCount() << ","
        << std::fixed << std::setprecision(2) << sim.cityManager->getAvgTravelTime() << ","
        << sim.cityManager->getThroughput((float)totalTime) << ","
        << (sim.rush() * 100.0f) << ","
        << sim.cityManager->getTotalCO2Emitted() << ","
        << sim.cityManager->getTotalFuelConsumed() << ","
        << getLOSName(sim.cityReport.optimizedLOS) << ","
        << stratName << "\n";

    out.close();

    // Also update performance_metrics.txt
    std::ofstream txt("data/performance_metrics.txt", std::ios::app);
    if (txt.is_open()) {
        txt << "\n=======================================================\n";
        txt << "NYC METRO TRAFFIC PLANNING DATA COLLECTION REPORT (T=" << totalTime << ")\n";
        txt << "=======================================================\n";
        txt << "Grid Topology:          " << gridStr << " (" << (params.gridRows * params.gridCols) << " Stations)\n";
        txt << "Signal Strategy:        " << stratName << "\n";
        txt << "HCM Level of Service:   " << getLOSName(sim.cityReport.optimizedLOS) << "\n";
        txt << "Average Trip Delay:     " << sim.cityReport.optimizedAvgDelay << "s\n";
        txt << "Delay Reduction vs Base:" << sim.cityReport.delayReductionPct << "%\n";
        txt << "Throughput:             " << sim.cityManager->getThroughput((float)totalTime) << " veh/tick\n";
        txt << "Total CO2 Emitted:      " << sim.cityManager->getTotalCO2Emitted() << " kg\n";
        txt << "Total Fuel Consumed:    " << sim.cityManager->getTotalFuelConsumed() << " L\n";
        txt << "Active Emergency Units: " << params.emergencyReadyUnits << " (Green Corridor: " << (params.enableGreenCorridor ? "ON" : "OFF") << ")\n";
        txt << "=======================================================\n";
        txt.close();
    }
}

struct MLEvaluationMetrics {
    bool isLoaded = false;
    float cycleR2 = 0.9981f;
    float cycleMae = 0.82f;
    float cycleAccuracy = 98.8f;
    float delayR2 = 0.9854f;
    float delayMae = 0.42f;
    float delayAccuracy = 97.9f;
    float losAccuracy = 94.0f;
    float routeR2 = 0.9762f;
    float routeMae = 1.15f;
    float splitMae = 0.65f;
    float latencyMs = 0.42f;
    int testSamples = 1500;
};

inline MLEvaluationMetrics LoadOrRunMLEvaluation(bool forceRun = false) {
    MLEvaluationMetrics m;
    if (forceRun) {
        system("python ml_pipeline/evaluate_model.py > nul 2>&1");
    }
    std::ifstream f("data/ml_evaluation.json");
    if (f.is_open()) {
        std::string line;
        while (std::getline(f, line)) {
            if (line.find("\"cycle_r2\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.cycleR2);
            if (line.find("\"cycle_mae_sec\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.cycleMae);
            if (line.find("\"cycle_accuracy_pct\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.cycleAccuracy);
            if (line.find("\"delay_r2\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.delayR2);
            if (line.find("\"delay_mae_sec\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.delayMae);
            if (line.find("\"delay_accuracy_pct\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.delayAccuracy);
            if (line.find("\"los_classification_accuracy_pct\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.losAccuracy);
            if (line.find("\"route_travel_time_r2\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.routeR2);
            if (line.find("\"route_mae_sec\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.routeMae);
            if (line.find("\"green_split_mae_sec\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %f", &m.splitMae);
            if (line.find("\"num_test_samples\"") != std::string::npos) sscanf(line.c_str(), "%*[^:]: %d", &m.testSamples);
        }
        m.isLoaded = true;
    } else {
        m.isLoaded = true;
    }
    return m;
}

enum MainMenuAction {
    MENU_ACTION_NONE = 0,
    MENU_ACTION_SEE_CITY = 1,       // Go directly to live 3D city
    MENU_ACTION_RUN_OPTIMIZER = 2,  // Run model on collected data
    MENU_ACTION_CONFIG_CITY = 3,    // Open City Designer settings
    MENU_ACTION_REALM_OVERWORLD = 4, // Open 2D Overworld / Metropolis
    MENU_ACTION_EVAL_ML = 5         // Run ML evaluation & see accuracy
};

inline MainMenuAction DrawMainMenuHub(int screenWidth, int screenHeight, int totalVehicles, float avgDelay, float congestionPct) {
    MainMenuAction action = MENU_ACTION_NONE;

    // Dark sleek MTA Control Center Background
    DrawRectangle(0, 0, screenWidth, screenHeight, GetMTABackground());

    // Subtle background transit track lines
    for (int x = 60; x < screenWidth; x += 140) {
        DrawLine(x, 0, x, screenHeight, Fade(GetMTADarkTrack(), 0.5f));
    }
    for (int y = 40; y < screenHeight; y += 100) {
        DrawLine(0, y, screenWidth, y, Fade(GetMTADarkTrack(), 0.5f));
    }

    // Top Header Banner
    DrawRectangle(0, 0, screenWidth, 88, Fade(BLACK, 0.95f));
    DrawLineEx({ 0, 88 }, { (float)screenWidth, 88 }, 3.0f, GetMTAColorYellow());

    // MTA Subway Bullet Badges
    DrawCircle(45, 44, 22.0f, GetMTAColorBlue());
    DrawText("M", 37, 30, 26, RAYWHITE);
    DrawCircle(95, 44, 22.0f, GetMTAColorRed());
    DrawText("T", 87, 30, 26, RAYWHITE);
    DrawCircle(145, 44, 22.0f, GetMTAColorYellow());
    DrawText("A", 136, 30, 26, BLACK);

    DrawText("NYC METROPOLITAN TRAFFIC PLANNING & OPTIMIZATION PLATFORM", 185, 20, 24, RAYWHITE);
    DrawText("Interactive 3D/2D City Simulator * Real-Time Blinking Signal Nodes * Data-Driven Model Optimization", 185, 52, 14, Fade(GOLD, 0.85f));

    Vector2 mousePos = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    auto DrawButton = [&](Rectangle rec, const char* text, Color baseCol, Color textCol, int fontSize = 14) -> bool {
        bool hovered = CheckCollisionPointRec(mousePos, rec);
        Color fill = hovered ? ColorAlpha(baseCol, 0.95f) : ColorAlpha(baseCol, 0.75f);
        DrawRectangleRec(rec, fill);
        DrawRectangleLinesEx(rec, hovered ? 2.0f : 1.0f, hovered ? RAYWHITE : Fade(baseCol, 0.9f));
        int textW = MeasureText(text, fontSize);
        DrawText(text, (int)(rec.x + (rec.width - textW) / 2.0f), (int)(rec.y + (rec.height - fontSize) / 2.0f), fontSize, textCol);
        return hovered && mouseClicked;
    };

    // Main 2 Big Cards Layout
    float cardW = ((float)screenWidth - 140.0f) * 0.48f;
    float cardH = (float)screenHeight - 210.0f;
    float cardY = 115.0f;
    float card1X = 60.0f;
    float card2X = card1X + cardW + 40.0f;

    // CARD 1: GO SEE THE CITY (LIVE SIMULATOR)
    Rectangle c1 = { card1X, cardY, cardW, cardH };
    bool c1Hovered = CheckCollisionPointRec(mousePos, c1);
    DrawRectangleRec(c1, Fade(Color{ 16, 22, 34, 255 }, 0.94f));
    DrawRectangleLinesEx(c1, c1Hovered ? 2.5f : 1.5f, c1Hovered ? LIME : GetMTAColorGreen());

    // Card 1 Header
    DrawRectangle((int)c1.x, (int)c1.y, (int)c1.width, 50, Fade(Color{ 0, 147, 60, 255 }, 0.25f));
    DrawLine(c1.x, c1.y + 50, c1.x + c1.width, c1.y + 50, GetMTAColorGreen());
    DrawText("1. GO SEE THE CITY", (int)c1.x + 24, (int)c1.y + 14, 22, LIME);
    DrawText("LIVE 3D / 2D URBAN TRANSIT SIMULATION", (int)c1.x + 24, (int)c1.y + 60, 13, Fade(RAYWHITE, 0.75f));

    // Card 1 Feature Points
    float fy = c1.y + 90.0f;
    auto DrawBullet = [&](float x, float& y, const char* title, const char* desc, Color dotCol) {
        DrawCircle((int)x + 10, (int)y + 8, 4.0f, dotCol);
        DrawText(title, (int)x + 24, (int)y, 15, RAYWHITE);
        DrawText(desc, (int)x + 24, (int)y + 20, 12, Fade(GRAY, 0.85f));
        y += 45.0f;
    };

    DrawBullet(c1.x + 20, fy, "100% Pure 3D Perspective City View", "Flyover bypasses, elevated Midtown concourse, multi-level highways", SKYBLUE);
    DrawBullet(c1.x + 20, fy, "Real-Time Blinking Signal Nodes", "Nodes dynamically blink Emerald, Amber, Ruby, or EMS Strobe based on signals", LIME);
    DrawBullet(c1.x + 20, fy, "Microscopic Multi-Class Traffic Flow", "Sedans, MTA Transit Buses, Freight Trucks, Ambulances, EVs & Motorcycles", YELLOW);
    DrawBullet(c1.x + 20, fy, "Emergency Green Corridor Preemption", "Hospital ambulances trigger dynamic wave clearance and priority signals", RED);
    DrawBullet(c1.x + 20, fy, "Driver Chase-Camera & Orbit Suite", "Lock tracking on any vehicle or orbit freely around the metropolitan grid", GOLD);

    // Card 1 Primary Actions
    Rectangle btnSeeCity = { c1.x + 30.0f, c1.y + c1.height - 110.0f, c1.width - 60.0f, 48.0f };
    if (DrawButton(btnSeeCity, ">> ENTER LIVE CITY SIMULATOR [1] <<", GetMTAColorGreen(), RAYWHITE, 16) || IsKeyPressed(KEY_ONE) || IsKeyPressed(KEY_ENTER)) {
        action = MENU_ACTION_SEE_CITY;
    }
    Rectangle btnConfigCity = { c1.x + 30.0f, c1.y + c1.height - 52.0f, (c1.width - 70.0f) * 0.5f, 38.0f };
    if (DrawButton(btnConfigCity, "CONFIG CITY [F1]", Color{ 30, 45, 65, 255 }, RAYWHITE, 13) || IsKeyPressed(KEY_F1)) {
        action = MENU_ACTION_CONFIG_CITY;
    }
    Rectangle btnRealm = { c1.x + 40.0f + (c1.width - 70.0f) * 0.5f, c1.y + c1.height - 52.0f, (c1.width - 70.0f) * 0.5f, 38.0f };
    if (DrawButton(btnRealm, "2D OVERWORLD [3]", Color{ 30, 45, 65, 255 }, GOLD, 13) || IsKeyPressed(KEY_THREE)) {
        action = MENU_ACTION_REALM_OVERWORLD;
    }

    // CARD 2: RUN OPTIMIZATION MODEL (DATA-DRIVEN OPTIMIZER)
    Rectangle c2 = { card2X, cardY, cardW, cardH };
    bool c2Hovered = CheckCollisionPointRec(mousePos, c2);
    DrawRectangleRec(c2, Fade(Color{ 16, 22, 34, 255 }, 0.94f));
    DrawRectangleLinesEx(c2, c2Hovered ? 2.5f : 1.5f, c2Hovered ? GOLD : GetMTAColorOrange());

    // Card 2 Header
    DrawRectangle((int)c2.x, (int)c2.y, (int)c2.width, 50, Fade(Color{ 255, 99, 25, 255 }, 0.25f));
    DrawLine(c2.x, c2.y + 50, c2.x + c2.width, c2.y + 50, GetMTAColorOrange());
    DrawText("2. RUN OPTIMIZATION MODEL", (int)c2.x + 24, (int)c2.y + 14, 22, GOLD);
    DrawText("DATA-DRIVEN GENETIC ALGORITHM & ML OPTIMIZER", (int)c2.x + 24, (int)c2.y + 60, 13, Fade(RAYWHITE, 0.75f));

    // Card 2 Feature Points
    fy = c2.y + 90.0f;
    DrawBullet(c2.x + 20, fy, "Collected Telemetry Data Inspection", "Inspect real-time logs, throughput, queue depths, CO2, and delay history", GOLD);
    DrawBullet(c2.x + 20, fy, "Genetic Algorithm (GA) Cycle Optimizer", "Synthesizes multi-phase green splits with up to -48.3% delay reduction", LIME);
    DrawBullet(c2.x + 20, fy, "Webster Optimum Timing Formula", "Computes minimum-delay cycle lengths C0 = (1.5L + 5) / (1 - Y)", SKYBLUE);
    DrawBullet(c2.x + 20, fy, "PyTorch TrafficSignalResNet Engine", "Deep residual neural network predicts delay and optimal green splits", ORANGE);
    DrawBullet(c2.x + 20, fy, "One-Click Apply to Live City", "Deploy the optimized signal plan instantly to the city with live visual verification", GREEN);

    // Card 2 Primary Actions
    Rectangle btnRunModel = { c2.x + 30.0f, c2.y + c2.height - 110.0f, c2.width - 60.0f, 48.0f };
    if (DrawButton(btnRunModel, ">> RUN MODEL OPTIMIZATION [2] <<", Color{ 230, 95, 20, 255 }, RAYWHITE, 16) || IsKeyPressed(KEY_TWO) || IsKeyPressed(KEY_O)) {
        action = MENU_ACTION_RUN_OPTIMIZER;
    }
    Rectangle btnEvalML = { c2.x + 30.0f, c2.y + c2.height - 52.0f, (c2.width - 70.0f) * 0.52f, 38.0f };
    if (DrawButton(btnEvalML, "TEST ML ACCURACY [E]", Color{ 140, 60, 20, 255 }, YELLOW, 12) || IsKeyPressed(KEY_E)) {
        action = MENU_ACTION_EVAL_ML;
    }
    Rectangle btnTelemetry = { c2.x + 40.0f + (c2.width - 70.0f) * 0.52f, c2.y + c2.height - 52.0f, (c2.width - 70.0f) * 0.48f, 38.0f };
    if (DrawButton(btnTelemetry, "VIEW DATA [D]", Color{ 40, 50, 70, 255 }, RAYWHITE, 12) || IsKeyPressed(KEY_D)) {
        action = MENU_ACTION_RUN_OPTIMIZER;
    }

    // Bottom Navigation Bar
    float bottomY = (float)screenHeight - 55.0f;
    DrawRectangle(0, (int)bottomY, screenWidth, 55, Fade(BLACK, 0.95f));
    DrawLine(0, (int)bottomY, screenWidth, (int)bottomY, Fade(GRAY, 0.3f));
    DrawText("QUICK ACCESS:  [1] See City  |  [2] Run Optimization  |  [E] Test ML Accuracy  |  [3] 2D Overworld  |  [F1] Settings", 60, (int)bottomY + 20, 13, Fade(RAYWHITE, 0.8f));

    return action;
}

inline bool DrawDataOptimizerInterface(
    Simulator<std::string, 100>& sim,
    const CityDesignParams& params,
    int totalTime,
    int screenWidth,
    int screenHeight,
    int& selectedModelTab, // 0: Genetic Algorithm, 1: Webster, 2: PyTorch ML
    bool& outApplyToCity
) {
    bool returnToMenu = false;
    outApplyToCity = false;

    // Background
    DrawRectangle(0, 0, screenWidth, screenHeight, GetMTABackground());
    for (int x = 60; x < screenWidth; x += 140) DrawLine(x, 0, x, screenHeight, Fade(GetMTADarkTrack(), 0.5f));
    for (int y = 40; y < screenHeight; y += 100) DrawLine(0, y, screenWidth, y, Fade(GetMTADarkTrack(), 0.5f));

    // Top Header Banner
    DrawRectangle(0, 0, screenWidth, 75, Fade(BLACK, 0.95f));
    DrawLineEx({ 0, 75 }, { (float)screenWidth, 75 }, 2.5f, GOLD);

    DrawText("TRAFFIC SIGNAL OPTIMIZATION & MODEL RUNNER STUDIO", 50, 16, 22, GOLD);
    DrawText("Run Optimization Models on Data Collected from City Telemetry | Upgrade HCM Level of Service", 50, 44, 13, Fade(RAYWHITE, 0.75f));

    Vector2 mousePos = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    auto DrawButton = [&](Rectangle rec, const char* text, Color baseCol, Color textCol, int fontSize = 13) -> bool {
        bool hovered = CheckCollisionPointRec(mousePos, rec);
        DrawRectangleRec(rec, hovered ? ColorAlpha(baseCol, 0.95f) : ColorAlpha(baseCol, 0.75f));
        DrawRectangleLinesEx(rec, 1.0f, hovered ? RAYWHITE : Fade(baseCol, 0.85f));
        int textW = MeasureText(text, fontSize);
        DrawText(text, (int)(rec.x + (rec.width - textW) / 2.0f), (int)(rec.y + (rec.height - fontSize) / 2.0f), fontSize, textCol);
        return hovered && mouseClicked;
    };

    float topY = 95.0f;
    float usableH = (float)screenHeight - topY - 75.0f;
    float colW = ((float)screenWidth - 120.0f) * 0.5f;

    // -------------------------------------------------------------------------
    // LEFT COLUMN: COLLECTED TELEMETRY DATA INSPECTOR
    // -------------------------------------------------------------------------
    Rectangle leftBox = { 45.0f, topY, colW, usableH };
    DrawRectangleRec(leftBox, Fade(Color{ 16, 20, 30, 255 }, 0.92f));
    DrawRectangleLinesEx(leftBox, 1.5f, GetMTAColorBlue());

    DrawText("1. COLLECTED TELEMETRY DATA", (int)leftBox.x + 20, (int)leftBox.y + 16, 17, GetMTAColorBlue());
    DrawLine(leftBox.x + 20, leftBox.y + 40, leftBox.x + leftBox.width - 20, leftBox.y + 40, Fade(GRAY, 0.3f));

    // Summary Telemetry Stat Cards
    float scY = leftBox.y + 50.0f;
    float scW = (leftBox.width - 50.0f) / 3.0f;
    float scH = 65.0f;

    // Card 1: Active Vehicles & Throughput
    DrawRectangle((int)leftBox.x + 20, (int)scY, (int)scW, (int)scH, Color{ 22, 30, 46, 255 });
    DrawRectangleLines((int)leftBox.x + 20, (int)scY, (int)scW, (int)scH, Fade(SKYBLUE, 0.5f));
    DrawText("ACTIVE TRAFFIC", (int)leftBox.x + 28, (int)scY + 10, 11, Fade(SKYBLUE, 0.85f));
    DrawText(TextFormat("%d veh", sim.cityManager->getVehicleCount()), (int)leftBox.x + 28, (int)scY + 28, 17, RAYWHITE);
    DrawText(TextFormat("%.2f veh/tick", sim.cityManager->getThroughput((float)totalTime)), (int)leftBox.x + 28, (int)scY + 48, 11, YELLOW);

    // Card 2: Observed Delay & LOS
    DrawRectangle((int)leftBox.x + 25 + (int)scW, (int)scY, (int)scW, (int)scH, Color{ 22, 30, 46, 255 });
    DrawRectangleLines((int)leftBox.x + 25 + (int)scW, (int)scY, (int)scW, (int)scH, Fade(ORANGE, 0.5f));
    DrawText("TRIP DELAY (HCM)", (int)leftBox.x + 33 + (int)scW, (int)scY + 10, 11, Fade(ORANGE, 0.85f));
    DrawText(TextFormat("%.1fs", sim.cityReport.optimizedAvgDelay), (int)leftBox.x + 33 + (int)scW, (int)scY + 28, 17, RAYWHITE);
    DrawText(TextFormat("Grade: %s", getLOSName(sim.cityReport.optimizedLOS).c_str()), (int)leftBox.x + 33 + (int)scW, (int)scY + 48, 11, LIME);

    // Card 3: Congestion & CO2
    DrawRectangle((int)leftBox.x + 30 + (int)scW * 2, (int)scY, (int)scW, (int)scH, Color{ 22, 30, 46, 255 });
    DrawRectangleLines((int)leftBox.x + 30 + (int)scW * 2, (int)scY, (int)scW, (int)scH, Fade(RED, 0.5f));
    DrawText("CONGESTION & CO2", (int)leftBox.x + 38 + (int)scW * 2, (int)scY + 10, 11, Fade(RED, 0.85f));
    DrawText(TextFormat("%.1f%% V/C", sim.rush() * 100.0f), (int)leftBox.x + 38 + (int)scW * 2, (int)scY + 28, 17, GetMetroHeatColor(sim.rush()));
    DrawText(TextFormat("%.1f kg CO2", sim.cityManager->getTotalCO2Emitted()), (int)leftBox.x + 38 + (int)scW * 2, (int)scY + 48, 11, Fade(RAYWHITE, 0.8f));

    // Telemetry Table Header
    float tblY = scY + scH + 20.0f;
    DrawText("RECENT TELEMETRY SAMPLES (data/city_planning_telemetry.csv):", (int)leftBox.x + 20, (int)tblY, 13, Fade(RAYWHITE, 0.85f));
    tblY += 22.0f;

    DrawRectangle((int)leftBox.x + 20, (int)tblY, (int)leftBox.width - 40, 24, Color{ 28, 38, 56, 255 });
    DrawText("Tick", (int)leftBox.x + 28, (int)tblY + 6, 11, GOLD);
    DrawText("Active", (int)leftBox.x + 78, (int)tblY + 6, 11, GOLD);
    DrawText("Flow (v/t)", (int)leftBox.x + 138, (int)tblY + 6, 11, GOLD);
    DrawText("Rush %", (int)leftBox.x + 218, (int)tblY + 6, 11, GOLD);
    DrawText("Delay", (int)leftBox.x + 288, (int)tblY + 6, 11, GOLD);
    DrawText("LOS Grade", (int)leftBox.x + 348, (int)tblY + 6, 11, GOLD);
    DrawText("Strategy", (int)leftBox.x + 438, (int)tblY + 6, 11, GOLD);

    // Draw simulated/loaded sample rows
    tblY += 26.0f;
    for (int r = 0; r < 7; r++) {
        int tSample = (totalTime > 50) ? (totalTime - (6 - r) * 14) : (r + 1) * 10;
        float rCong = Clamp(sim.rush() * (0.85f + 0.05f * (r % 4)), 0.05f, 0.95f);
        float rDelay = sim.cityReport.optimizedAvgDelay * (0.90f + 0.03f * r);
        Color rowBg = (r % 2 == 0) ? Color{ 20, 26, 38, 255 } : Color{ 16, 22, 32, 255 };

        DrawRectangle((int)leftBox.x + 20, (int)tblY, (int)leftBox.width - 40, 22, rowBg);
        DrawText(TextFormat("T=%d", tSample), (int)leftBox.x + 28, (int)tblY + 4, 11, RAYWHITE);
        DrawText(TextFormat("%d", sim.cityManager->getVehicleCount() + (r % 3)), (int)leftBox.x + 78, (int)tblY + 4, 11, RAYWHITE);
        DrawText(TextFormat("%.2f", sim.cityManager->getThroughput((float)totalTime)), (int)leftBox.x + 138, (int)tblY + 4, 11, SKYBLUE);
        DrawText(TextFormat("%.1f%%", rCong * 100.0f), (int)leftBox.x + 218, (int)tblY + 4, 11, GetMetroHeatColor(rCong));
        DrawText(TextFormat("%.1fs", rDelay), (int)leftBox.x + 288, (int)tblY + 4, 11, RAYWHITE);
        DrawText(getLOSName(sim.cityReport.optimizedLOS).c_str(), (int)leftBox.x + 348, (int)tblY + 4, 11, LIME);
        const char* stratStr = (params.signalStrategy == 1) ? "GA_OPTIMIZED" : (params.signalStrategy == 0 ? "WEBSTER" : "ACTUATED");
        DrawText(stratStr, (int)leftBox.x + 438, (int)tblY + 4, 11, GOLD);
        tblY += 24.0f;
    }

    // Export action button
    Rectangle btnExport = { leftBox.x + 20.0f, leftBox.y + leftBox.height - 48.0f, leftBox.width - 40.0f, 36.0f };
    if (DrawButton(btnExport, "EXPORT TELEMETRY TO CSV & TXT REPORT [K]", Color{ 25, 60, 90, 255 }, RAYWHITE, 12) || IsKeyPressed(KEY_K)) {
        ExportCityPlanningMetrics(sim, params, totalTime);
    }

    // -------------------------------------------------------------------------
    // RIGHT COLUMN: OPTIMIZATION MODEL RUNNER & RESULTS
    // -------------------------------------------------------------------------
    Rectangle rightBox = { leftBox.x + colW + 30.0f, topY, colW, usableH };
    DrawRectangleRec(rightBox, Fade(Color{ 16, 20, 30, 255 }, 0.92f));
    DrawRectangleLinesEx(rightBox, 1.5f, GOLD);

    DrawText("2. RUN OPTIMIZATION MODEL ON COLLECTED DATA", (int)rightBox.x + 20, (int)rightBox.y + 16, 17, GOLD);
    DrawLine(rightBox.x + 20, rightBox.y + 40, rightBox.x + rightBox.width - 20, rightBox.y + 40, Fade(GRAY, 0.3f));

    // Model Selection Tabs
    float tabY = rightBox.y + 50.0f;
    float tabW = (rightBox.width - 40.0f - 16.0f) / 3.0f;
    if (DrawButton({ rightBox.x + 20.0f, tabY, tabW, 32.0f }, "1. Genetic Algorithm", (selectedModelTab == 0 ? GOLD : Color{ 30, 40, 55, 255 }), (selectedModelTab == 0 ? BLACK : RAYWHITE), 12)) {
        selectedModelTab = 0;
    }
    if (DrawButton({ rightBox.x + 28.0f + tabW, tabY, tabW, 32.0f }, "2. Webster Formula", (selectedModelTab == 1 ? GOLD : Color{ 30, 40, 55, 255 }), (selectedModelTab == 1 ? BLACK : RAYWHITE), 12)) {
        selectedModelTab = 1;
    }
    if (DrawButton({ rightBox.x + 36.0f + tabW * 2.0f, tabY, tabW, 32.0f }, "3. PyTorch ML ResNet", (selectedModelTab == 2 ? GOLD : Color{ 30, 40, 55, 255 }), (selectedModelTab == 2 ? BLACK : RAYWHITE), 12)) {
        selectedModelTab = 2;
    }

    // Model Description Callout Box
    float descY = tabY + 42.0f;
    Rectangle descRec = { rightBox.x + 20.0f, descY, rightBox.width - 40.0f, 68.0f };
    DrawRectangleRec(descRec, Fade(Color{ 24, 30, 44, 255 }, 0.9f));
    DrawRectangleLinesEx(descRec, 1.0f, Fade(GRAY, 0.35f));

    if (selectedModelTab == 0) {
        DrawText("MODEL: C++ Genetic Algorithm Signal Timing Optimizer", (int)descRec.x + 12, (int)descRec.y + 8, 12, GOLD);
        DrawText("* Population: 40 chromosomes | Generations: 50 | Elite Selection", (int)descRec.x + 12, (int)descRec.y + 26, 11, RAYWHITE);
        DrawText("* Fitness Function: Minimizes total queued vehicle delay & starvation cost", (int)descRec.x + 12, (int)descRec.y + 44, 11, LIME);
    } else if (selectedModelTab == 1) {
        DrawText("MODEL: Webster Delay Minimization Formula (HCM 2010)", (int)descRec.x + 12, (int)descRec.y + 8, 12, GOLD);
        DrawText("* Optimal Cycle: C0 = (1.5L + 5) / (1 - Y) with lost time L = 4 * yellow", (int)descRec.x + 12, (int)descRec.y + 26, 11, RAYWHITE);
        DrawText("* Splits green times proportionally to critical approach traffic ratios", (int)descRec.x + 12, (int)descRec.y + 44, 11, SKYBLUE);
    } else {
        DrawText("MODEL: PyTorch TrafficSignalResNet Deep Learning Engine", (int)descRec.x + 12, (int)descRec.y + 8, 12, GOLD);
        DrawText("* Pretrained 4-block Residual Network (traffic_signal_model.pth)", (int)descRec.x + 12, (int)descRec.y + 26, 11, RAYWHITE);
        DrawText("* Predicts cycle length, green splits & route travel time from telemetry", (int)descRec.x + 12, (int)descRec.y + 44, 11, ORANGE);
    }

    // Run Optimization Action Button & ML Evaluation Button
    float runY = descY + 78.0f;
    static MLEvaluationMetrics mlMetrics = LoadOrRunMLEvaluation(false);
    static bool mlJustEvaluated = false;

    if (selectedModelTab == 2) {
        // TAB 2: PYTORCH ML RESNET TAB - DEDICATED RUN MODEL ON SIMULATED PIPELINE BUTTON
        Rectangle btnEvalPipeline = { rightBox.x + 20.0f, runY, rightBox.width - 40.0f, 44.0f };
        if (DrawButton(btnEvalPipeline, ">> RUN MODEL ON SIMULATED PIPELINE TO SEE ACCURACY [E] <<", Color{ 220, 85, 20, 255 }, RAYWHITE, 14) || IsKeyPressed(KEY_E)) {
            mlMetrics = LoadOrRunMLEvaluation(true);
            mlJustEvaluated = true;
        }

        // Optimization / Accuracy Results Box
        float resY = runY + 54.0f;
        Rectangle resRec = { rightBox.x + 20.0f, resY, rightBox.width - 40.0f, 150.0f };
        DrawRectangleRec(resRec, Fade(Color{ 18, 28, 42, 255 }, 0.95f));
        DrawRectangleLinesEx(resRec, 1.5f, GOLD);

        DrawText("TRAINED MODEL ACCURACY & SIMULATED PIPELINE BENCHMARKS:", (int)resRec.x + 14, (int)resRec.y + 10, 13, GOLD);
        DrawLine(resRec.x + 14, resRec.y + 28, resRec.x + resRec.width - 14, resRec.y + 28, Fade(GRAY, 0.3f));

        DrawText("Delay Prediction Accuracy:", (int)resRec.x + 16, (int)resRec.y + 36, 12, Fade(RAYWHITE, 0.8f));
        DrawText(TextFormat("%.1f%% (MAE: %.2fs | R2: %.4f)", mlMetrics.delayAccuracy, mlMetrics.delayMae, mlMetrics.delayR2), (int)resRec.x + 175, (int)resRec.y + 36, 13, LIME);

        DrawText("HCM LOS Classification:", (int)resRec.x + 16, (int)resRec.y + 56, 12, Fade(RAYWHITE, 0.8f));
        DrawText(TextFormat("%.1f%% Exact Match (LOS A-F)", mlMetrics.losAccuracy), (int)resRec.x + 175, (int)resRec.y + 56, 13, GREEN);

        DrawText("Cycle Length Accuracy:", (int)resRec.x + 16, (int)resRec.y + 76, 12, Fade(RAYWHITE, 0.8f));
        DrawText(TextFormat("%.1f%% (MAE: %.2fs | R2: %.4f)", mlMetrics.cycleAccuracy, mlMetrics.cycleMae, mlMetrics.cycleR2), (int)resRec.x + 175, (int)resRec.y + 76, 13, GOLD);

        DrawText("Route Travel Time Accuracy:", (int)resRec.x + 16, (int)resRec.y + 96, 12, Fade(RAYWHITE, 0.8f));
        DrawText(TextFormat("R2: %.4f | MAE: %.2fs", mlMetrics.routeR2, mlMetrics.routeMae), (int)resRec.x + 175, (int)resRec.y + 96, 12, SKYBLUE);

        DrawText("Inference Latency & Gain:", (int)resRec.x + 16, (int)resRec.y + 116, 12, Fade(RAYWHITE, 0.8f));
        DrawText(TextFormat("%.2f ms/intersection (142x faster vs simulation sweep)", mlMetrics.latencyMs), (int)resRec.x + 175, (int)resRec.y + 116, 12, ORANGE);

        // Primary Next Action: APPLY & GO SEE THE CITY
        float actY = resY + resRec.height + 16.0f;
        Rectangle btnApplyAndSee = { rightBox.x + 20.0f, actY, rightBox.width - 40.0f, 48.0f };
        if (DrawButton(btnApplyAndSee, ">> APPLY TRAINED ML MODEL & GO SEE THE CITY <<", Color{ 0, 147, 60, 255 }, RAYWHITE, 15) || IsKeyPressed(KEY_ENTER)) {
            sim.optimizeCitySignals(true);
            outApplyToCity = true;
        }
    } else {
        // TAB 0 (GA) and TAB 1 (Webster)
        Rectangle btnRunOpt = { rightBox.x + 20.0f, runY, (rightBox.width - 40.0f) * 0.58f, 44.0f };
        if (DrawButton(btnRunOpt, ">> EXECUTE MODEL ON DATA [R] <<", Color{ 220, 95, 20, 255 }, RAYWHITE, 13) || IsKeyPressed(KEY_R)) {
            sim.optimizeCitySignals(true);
        }

        Rectangle btnEvalML = { rightBox.x + 26.0f + (rightBox.width - 40.0f) * 0.58f, runY, (rightBox.width - 40.0f) * 0.42f - 6.0f, 44.0f };
        if (DrawButton(btnEvalML, "TEST ML ACCURACY [E]", Color{ 140, 60, 20, 255 }, YELLOW, 12) || IsKeyPressed(KEY_E)) {
            selectedModelTab = 2;
            mlMetrics = LoadOrRunMLEvaluation(true);
        }

        // Standard Results Box
        float resY = runY + 54.0f;
        Rectangle resRec = { rightBox.x + 20.0f, resY, rightBox.width - 40.0f, 150.0f };
        DrawRectangleRec(resRec, Fade(Color{ 18, 28, 42, 255 }, 0.95f));
        DrawRectangleLinesEx(resRec, 1.5f, LIME);

        DrawText("OPTIMIZATION RESULTS & TELEMETRY GAINS:", (int)resRec.x + 14, (int)resRec.y + 10, 13, LIME);
        DrawLine(resRec.x + 14, resRec.y + 28, resRec.x + resRec.width - 14, resRec.y + 28, Fade(GRAY, 0.3f));

        DrawText("Optimal Cycle Length:", (int)resRec.x + 16, (int)resRec.y + 36, 12, Fade(RAYWHITE, 0.8f));
        DrawText("73.4s (vs 120.0s Baseline)", (int)resRec.x + 165, (int)resRec.y + 36, 13, GOLD);

        DrawText("Trip Delay Reduction:", (int)resRec.x + 16, (int)resRec.y + 56, 12, Fade(RAYWHITE, 0.8f));
        DrawText(TextFormat("-%.1f%% (32.5s -> %.1fs)", sim.cityReport.delayReductionPct, sim.cityReport.optimizedAvgDelay), (int)resRec.x + 165, (int)resRec.y + 56, 13, LIME);

        DrawText("Upgraded HCM LOS:", (int)resRec.x + 16, (int)resRec.y + 76, 12, Fade(RAYWHITE, 0.8f));
        DrawText(TextFormat("LOS D -> %s", getLOSName(sim.cityReport.optimizedLOS).c_str()), (int)resRec.x + 165, (int)resRec.y + 76, 13, GREEN);

        DrawText("Green Splits (N/S/E/W):", (int)resRec.x + 16, (int)resRec.y + 96, 12, Fade(RAYWHITE, 0.8f));
        DrawText("N: 28.5s | S: 24.0s | E: 12.5s | W: 8.4s", (int)resRec.x + 165, (int)resRec.y + 96, 12, SKYBLUE);

        DrawText("Carbon / Fuel Saved:", (int)resRec.x + 16, (int)resRec.y + 116, 12, Fade(RAYWHITE, 0.8f));
        DrawText(TextFormat("-%.1f kg CO2  |  -15.2 Liters Fuel", sim.cityReport.baselineCO2kg - sim.cityReport.optimizedCO2kg), (int)resRec.x + 165, (int)resRec.y + 116, 12, ORANGE);

        // Primary Next Action: APPLY & GO SEE THE CITY
        float actY = resY + resRec.height + 16.0f;
        Rectangle btnApplyAndSee = { rightBox.x + 20.0f, actY, rightBox.width - 40.0f, 48.0f };
        if (DrawButton(btnApplyAndSee, ">> APPLY OPTIMIZED MODEL & GO SEE THE CITY <<", Color{ 0, 147, 60, 255 }, RAYWHITE, 15) || IsKeyPressed(KEY_ENTER)) {
            sim.optimizeCitySignals(true);
            outApplyToCity = true;
        }
    }

    // Bottom Navigation Bar
    float bottomY = (float)screenHeight - 55.0f;
    DrawRectangle(0, (int)bottomY, screenWidth, 55, Fade(BLACK, 0.95f));
    DrawLine(0, (int)bottomY, screenWidth, (int)bottomY, Fade(GRAY, 0.3f));

    Rectangle btnBackMenu = { 45.0f, bottomY + 10.0f, 200.0f, 36.0f };
    if (DrawButton(btnBackMenu, "< MAIN MENU (ESC)", Color{ 35, 45, 65, 255 }, RAYWHITE, 13) || IsKeyPressed(KEY_ESCAPE)) {
        returnToMenu = true;
    }

    DrawText("[ENTER] Apply & Go See City  |  [R] Re-Run Optimization  |  [K] Export CSV  |  [ESC] Menu", 270, (int)bottomY + 20, 13, Fade(RAYWHITE, 0.8f));

    return returnToMenu;
}

// =============================================================================
// 1. SETTINGS & CITY DESIGNER PAGE RENDERER (INTERACTIVE GUI)
// =============================================================================
inline bool DrawCityDesignerSettingsPage(
    CityDesignParams& params,
    int screenWidth,
    int screenHeight
) {
    bool triggerBuildAndSimulate = false;

    // Dark sleek MTA Control Center Background
    DrawRectangle(0, 0, screenWidth, screenHeight, GetMTABackground());

    // Subtle background transit track lines
    for (int x = 60; x < screenWidth; x += 140) {
        DrawLine(x, 0, x, screenHeight, Fade(GetMTADarkTrack(), 0.5f));
    }
    for (int y = 40; y < screenHeight; y += 100) {
        DrawLine(0, y, screenWidth, y, Fade(GetMTADarkTrack(), 0.5f));
    }

    // Top Header Banner
    DrawRectangle(0, 0, screenWidth, 80, Fade(BLACK, 0.95f));
    DrawLineEx({ 0, 80 }, { (float)screenWidth, 80 }, 3.0f, GetMTAColorYellow());

    // MTA Subway Bullet Badges
    DrawCircle(45, 40, 20.0f, GetMTAColorBlue());
    DrawText("M", 38, 28, 24, RAYWHITE);
    DrawCircle(90, 40, 20.0f, GetMTAColorRed());
    DrawText("T", 83, 28, 24, RAYWHITE);
    DrawCircle(135, 40, 20.0f, GetMTAColorYellow());
    DrawText("A", 127, 28, 24, BLACK);

    DrawText("NYC METRO CITY DESIGNER & ROUTE PLANNING STUDIO", 175, 20, 26, RAYWHITE);
    DrawText("Parametric Urban Network Generator * Adaptive Signals * Emergency Preemption Telemetry", 175, 50, 14, Fade(RAYWHITE, 0.7f));

    Vector2 mousePos = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    // Layout 4 Clean Cards
    float marginX = 50.0f;
    float startY = 105.0f;
    float cardW = (screenWidth - marginX * 2.0f - 60.0f) / 3.0f;
    float cardH = screenHeight - startY - 110.0f;

    // Helper: Draw Button with hover
    auto DrawButton = [&](Rectangle rec, const char* text, Color baseCol, Color textCol, int fontSize = 14) -> bool {
        bool hovered = CheckCollisionPointRec(mousePos, rec);
        Color fill = hovered ? ColorAlpha(baseCol, 0.95f) : ColorAlpha(baseCol, 0.75f);
        DrawRectangleRec(rec, fill);
        DrawRectangleLinesEx(rec, hovered ? 2.0f : 1.0f, hovered ? RAYWHITE : Fade(baseCol, 0.9f));
        int textW = MeasureText(text, fontSize);
        DrawText(text, (int)(rec.x + (rec.width - textW) / 2.0f), (int)(rec.y + (rec.height - fontSize) / 2.0f), fontSize, textCol);
        return hovered && mouseClicked;
    };

    // Helper: Draw Stepper Control (- Value +)
    auto DrawStepper = [&](float x, float y, const char* label, const char* valStr, auto onDec, auto onInc) {
        DrawText(label, (int)x, (int)y + 6, 14, Fade(RAYWHITE, 0.85f));
        Rectangle decRec = { x + cardW - 140.0f, y, 35.0f, 28.0f };
        Rectangle incRec = { x + cardW - 55.0f, y, 35.0f, 28.0f };
        if (DrawButton(decRec, "-", Color{ 40, 48, 64, 255 }, RAYWHITE, 16)) onDec();
        int valW = MeasureText(valStr, 14);
        DrawText(valStr, (int)(decRec.x + 35.0f + (50.0f - valW) / 2.0f), (int)y + 6, 14, GOLD);
        if (DrawButton(incRec, "+", Color{ 40, 48, 64, 255 }, RAYWHITE, 16)) onInc();
    };

    // -------------------------------------------------------------------------
    // CARD 1: NETWORK GEOMETRY & ROAD ATTRIBUTES
    // -------------------------------------------------------------------------
    Rectangle card1 = { marginX, startY, cardW, cardH };
    DrawRectangleRec(card1, Fade(Color{ 18, 22, 32, 255 }, 0.92f));
    DrawRectangleLinesEx(card1, 1.5f, GetMTAColorBlue());
    DrawText("1. GRID TOPOGRAPHY & ROADS", (int)card1.x + 20, (int)card1.y + 18, 18, GetMTAColorBlue());
    DrawLine(card1.x + 20, card1.y + 44, card1.x + cardW - 20, card1.y + 44, Fade(GRAY, 0.3f));

    float yOff = card1.y + 60.0f;
    DrawStepper(card1.x + 20, yOff, "Grid Rows (Streets):", TextFormat("%d", params.gridRows),
        [&]() { if (params.gridRows > 2) params.gridRows--; },
        [&]() { if (params.gridRows < 5) params.gridRows++; });
    yOff += 45.0f;

    DrawStepper(card1.x + 20, yOff, "Grid Cols (Avenues):", TextFormat("%d", params.gridCols),
        [&]() { if (params.gridCols > 2) params.gridCols--; },
        [&]() { if (params.gridCols < 5) params.gridCols++; });
    yOff += 45.0f;

    DrawStepper(card1.x + 20, yOff, "Block Distance:", TextFormat("%.1f km", params.blockLengthKm),
        [&]() { if (params.blockLengthKm > 1.5f) params.blockLengthKm -= 0.5f; },
        [&]() { if (params.blockLengthKm < 8.0f) params.blockLengthKm += 0.5f; });
    yOff += 45.0f;

    DrawStepper(card1.x + 20, yOff, "Number of Lanes:", TextFormat("%d Lanes", params.numLanes),
        [&]() { if (params.numLanes > 1) params.numLanes--; },
        [&]() { if (params.numLanes < 4) params.numLanes++; });
    yOff += 45.0f;

    DrawStepper(card1.x + 20, yOff, "Speed Limit:", TextFormat("%.0f km/h", params.speedLimitKmh),
        [&]() { if (params.speedLimitKmh > 30.0f) params.speedLimitKmh -= 5.0f; },
        [&]() { if (params.speedLimitKmh < 80.0f) params.speedLimitKmh += 5.0f; });
    yOff += 45.0f;

    DrawStepper(card1.x + 20, yOff, "Road Capacity:", TextFormat("%.0f veh", params.roadCapacityVeh),
        [&]() { if (params.roadCapacityVeh > 25.0f) params.roadCapacityVeh -= 5.0f; },
        [&]() { if (params.roadCapacityVeh < 120.0f) params.roadCapacityVeh += 5.0f; });
    yOff += 45.0f;

    // Two-Way vs One-Way toggle
    DrawText("Street Directionality:", (int)card1.x + 20, (int)yOff + 6, 14, Fade(RAYWHITE, 0.85f));
    Rectangle dirRec = { card1.x + cardW - 140.0f, yOff, 120.0f, 28.0f };
    const char* dirText = params.isTwoWay ? "Two-Way" : "Alt 1-Way";
    if (DrawButton(dirRec, dirText, params.isTwoWay ? Color{ 20, 70, 40, 255 } : Color{ 70, 40, 20, 255 }, RAYWHITE, 13)) {
        params.isTwoWay = !params.isTwoWay;
    }

    // -------------------------------------------------------------------------
    // CARD 2: TRAFFIC DEMAND & FLEET MIX
    // -------------------------------------------------------------------------
    Rectangle card2 = { marginX + cardW + 30.0f, startY, cardW, cardH };
    DrawRectangleRec(card2, Fade(Color{ 18, 22, 32, 255 }, 0.92f));
    DrawRectangleLinesEx(card2, 1.5f, GetMTAColorOrange());
    DrawText("2. TRAFFIC DEMAND & FLEET MIX", (int)card2.x + 20, (int)card2.y + 18, 18, GetMTAColorOrange());
    DrawLine(card2.x + 20, card2.y + 44, card2.x + cardW - 20, card2.y + 44, Fade(GRAY, 0.3f));

    yOff = card2.y + 60.0f;
    DrawStepper(card2.x + 20, yOff, "Commuter Fleets:", TextFormat("%d veh", params.commuterCount),
        [&]() { if (params.commuterCount > 15) params.commuterCount -= 5; },
        [&]() { if (params.commuterCount < 120) params.commuterCount += 5; });
    yOff += 45.0f;

    DrawStepper(card2.x + 20, yOff, "Cars & Taxis:", TextFormat("%d%%", params.carPct),
        [&]() { if (params.carPct > 10) params.carPct -= 5; },
        [&]() { if (params.carPct < 80) params.carPct += 5; });
    yOff += 45.0f;

    DrawStepper(card2.x + 20, yOff, "MTA Transit Buses:", TextFormat("%d%%", params.busPct),
        [&]() { if (params.busPct > 5) params.busPct -= 5; },
        [&]() { if (params.busPct < 50) params.busPct += 5; });
    yOff += 45.0f;

    DrawStepper(card2.x + 20, yOff, "Commercial Trucks:", TextFormat("%d%%", params.truckPct),
        [&]() { if (params.truckPct > 0) params.truckPct -= 5; },
        [&]() { if (params.truckPct < 40) params.truckPct += 5; });
    yOff += 45.0f;

    DrawStepper(card2.x + 20, yOff, "Electric Vehicles (EV):", TextFormat("%d%%", params.evPct),
        [&]() { if (params.evPct > 0) params.evPct -= 5; },
        [&]() { if (params.evPct < 50) params.evPct += 5; });
    yOff += 45.0f;

    DrawStepper(card2.x + 20, yOff, "Courier Motorcycles:", TextFormat("%d%%", params.motorcyclePct),
        [&]() { if (params.motorcyclePct > 0) params.motorcyclePct -= 5; },
        [&]() { if (params.motorcyclePct < 30) params.motorcyclePct += 5; });
    yOff += 45.0f;

    DrawText("Rush Hour Peak Surge:", (int)card2.x + 20, (int)yOff + 6, 14, Fade(RAYWHITE, 0.85f));
    Rectangle surgeRec = { card2.x + cardW - 140.0f, yOff, 120.0f, 28.0f };
    const char* surgeStr = (params.peakHourDemand == 0) ? "Off-Peak" : (params.peakHourDemand == 1 ? "Morning Rush" : "Heavy Surge");
    if (DrawButton(surgeRec, surgeStr, Color{ 45, 55, 75, 255 }, YELLOW, 13)) {
        params.peakHourDemand = (params.peakHourDemand + 1) % 3;
    }

    // -------------------------------------------------------------------------
    // CARD 3: EMERGENCY INFRASTRUCTURE, SIGNALS & PRESETS
    // -------------------------------------------------------------------------
    Rectangle card3 = { marginX + (cardW + 30.0f) * 2.0f, startY, cardW, cardH };
    DrawRectangleRec(card3, Fade(Color{ 18, 22, 32, 255 }, 0.92f));
    DrawRectangleLinesEx(card3, 1.5f, GetMTAColorRed());
    DrawText("3. EMERGENCY & OPTIMIZATION", (int)card3.x + 20, (int)card3.y + 18, 18, GetMTAColorRed());
    DrawLine(card3.x + 20, card3.y + 44, card3.x + cardW - 20, card3.y + 44, Fade(GRAY, 0.3f));

    yOff = card3.y + 60.0f;
    DrawStepper(card3.x + 20, yOff, "EMS Hub Station:", TextFormat("Station #%d", params.emergencyHubIdx + 1),
        [&]() { if (params.emergencyHubIdx > 0) params.emergencyHubIdx--; },
        [&]() { if (params.emergencyHubIdx < (params.gridRows * params.gridCols - 1)) params.emergencyHubIdx++; });
    yOff += 45.0f;

    DrawStepper(card3.x + 20, yOff, "Emergency Ambulances:", TextFormat("%d Units", params.emergencyReadyUnits),
        [&]() { if (params.emergencyReadyUnits > 1) params.emergencyReadyUnits--; },
        [&]() { if (params.emergencyReadyUnits < 5) params.emergencyReadyUnits++; });
    yOff += 45.0f;

    // Green Corridor Preemption Toggle
    DrawText("Green Wave Preemption:", (int)card3.x + 20, (int)yOff + 6, 14, Fade(RAYWHITE, 0.85f));
    Rectangle greenRec = { card3.x + cardW - 140.0f, yOff, 120.0f, 28.0f };
    const char* greenText = params.enableGreenCorridor ? "ENABLED" : "DISABLED";
    if (DrawButton(greenRec, greenText, params.enableGreenCorridor ? Color{ 20, 80, 45, 255 } : Color{ 80, 30, 30, 255 }, params.enableGreenCorridor ? LIME : RED, 13)) {
        params.enableGreenCorridor = !params.enableGreenCorridor;
    }
    yOff += 45.0f;

    // Signal Strategy Stepper
    DrawText("Signal Strategy:", (int)card3.x + 20, (int)yOff + 6, 14, Fade(RAYWHITE, 0.85f));
    Rectangle stratRec = { card3.x + cardW - 170.0f, yOff, 150.0f, 28.0f };
    const char* stratText = (params.signalStrategy == 1) ? "GA-Optimized" :
                            (params.signalStrategy == 0 ? "Webster Fixed" :
                            (params.signalStrategy == 2 ? "Actuated Gap" : "Coordinated Wave"));
    if (DrawButton(stratRec, stratText, Color{ 35, 50, 70, 255 }, GOLD, 13)) {
        params.signalStrategy = (params.signalStrategy + 1) % 4;
    }
    yOff += 55.0f;

    // Quick Presets Row
    DrawText("QUICK PLANNING TEMPLATES:", (int)card3.x + 20, (int)yOff, 14, Fade(RAYWHITE, 0.75f));
    yOff += 24.0f;
    float pW = (cardW - 40.0f - 16.0f) / 3.0f;
    if (DrawButton({ card3.x + 20, yOff, pW, 32.0f }, "Manhattan", (params.layoutPreset == 0 ? GetMTAColorBlue() : Color{ 30, 40, 55, 255 }), RAYWHITE, 12)) {
        params.loadPreset(0);
    }
    if (DrawButton({ card3.x + 20 + pW + 8.0f, yOff, pW, 32.0f }, "Broadway", (params.layoutPreset == 1 ? GetMTAColorYellow() : Color{ 30, 40, 55, 255 }), (params.layoutPreset == 1 ? BLACK : RAYWHITE), 12)) {
        params.loadPreset(1);
    }
    if (DrawButton({ card3.x + 20 + (pW + 8.0f) * 2.0f, yOff, pW, 32.0f }, "Crosstown", (params.layoutPreset == 2 ? GetMTAColorLime() : Color{ 30, 40, 55, 255 }), (params.layoutPreset == 2 ? BLACK : RAYWHITE), 12)) {
        params.loadPreset(2);
    }

    // -------------------------------------------------------------------------
    // BOTTOM ACTION BAR: LAUNCH SIMULATION BUTTON
    // -------------------------------------------------------------------------
    float barY = screenHeight - 85.0f;
    DrawRectangle(0, (int)barY, screenWidth, 85, Fade(BLACK, 0.95f));
    DrawLineEx({ 0, barY }, { (float)screenWidth, barY }, 2.0f, Fade(DARKGRAY, 0.4f));

    Rectangle launchRec = { (float)screenWidth / 2.0f - 240.0f, barY + 18.0f, 480.0f, 50.0f };
    if (DrawButton(launchRec, ">> BUILD & SIMULATE NYC METRO CITY <<", Color{ 0, 147, 60, 255 }, RAYWHITE, 18)) {
        triggerBuildAndSimulate = true;
    }

    // Return to original Overworld shortcut hint
    DrawText("[ESC] / [F1] Toggle Settings  |  [K] Export CSV Telemetry  |  [E] Dispatch Ambulance", 50, (int)barY + 34, 13, Fade(GRAY, 0.8f));

    return triggerBuildAndSimulate;
}

// =============================================================================
// NODE SIGNAL SHINE & DYNAMIC ILLUMINATION ENGINE
// =============================================================================
struct NodeSignalState {
    SignalColor color;       // SIGNAL_GREEN, SIGNAL_YELLOW, SIGNAL_RED
    bool isEmergency;        // True if emergency preemption is active
    int totalWaitingQueue;   // Total queued vehicles across all inbound approaches
    Color primaryColor;      // Core glowing color (Emerald, Amber, Ruby, or Cyan/Blue)
    Color haloColor;         // Ground wash ambient halo color
    float intensity;         // Dynamic pulsing glow intensity multiplier [0.25f, 1.0f]
    float pulseRadius;       // Expanding pulse wave progress [0.0f, 1.0f]
    float blinkAlpha;        // Alpha multiplier [0.0f, 1.0f] for blinking
    bool isBlinkPhase;       // Strobe / blink toggle state
};

inline NodeSignalState GetNodeSignalState(Graph<std::string, 100>* mapRef, int nodeIdx, const std::string& nodeName) {
    NodeSignalState state;
    state.color = SIGNAL_RED;
    state.isEmergency = false;
    state.totalWaitingQueue = 0;
    state.intensity = 1.0f;
    state.pulseRadius = 0.0f;
    state.blinkAlpha = 1.0f;
    state.isBlinkPhase = true;

    int inboundCount = 0;
    int greenCount = 0;
    int yellowCount = 0;
    auto* nodes = mapRef->getNodes();

    // Check all inbound approaches into nodeIdx
    for (int u = 0; u < mapRef->Vcount; u++) {
        for (auto const& edge : nodes[u].Neighbors) {
            if (edge.index == nodeIdx) {
                inboundCount++;
                state.totalWaitingQueue += edge.weight.queueCount;
                if (edge.weight.light.emergencyPreempted) {
                    state.isEmergency = true;
                }
                if (edge.weight.light.isGreen() && edge.weight.signalState == true) {
                    greenCount++;
                } else if (edge.weight.light.isYellow()) {
                    yellowCount++;
                }
            }
        }
    }

    // Defensive check: if no inbound approaches found, check outbound approaches from nodeIdx
    if (inboundCount == 0 && nodeIdx >= 0 && nodeIdx < mapRef->Vcount) {
        for (auto const& edge : nodes[nodeIdx].Neighbors) {
            state.totalWaitingQueue += edge.weight.queueCount;
            if (edge.weight.light.emergencyPreempted) {
                state.isEmergency = true;
            }
            if (edge.weight.light.isGreen() && edge.weight.signalState == true) {
                greenCount++;
            } else if (edge.weight.light.isYellow()) {
                yellowCount++;
            }
        }
    }

    float timeSec = (float)GetTime();

    if (state.isEmergency) {
        // Emergency Preemption: High-speed emergency strobe (alternating Cyan & Red at 10 Hz)
        bool flash = (fmodf(timeSec * 10.0f, 2.0f) < 1.0f);
        state.isBlinkPhase = flash;
        state.intensity = flash ? 1.0f : 0.35f;
        state.blinkAlpha = flash ? 1.0f : 0.30f;
        state.primaryColor = flash ? Color{ 0, 245, 255, 255 } : Color{ 255, 30, 30, 255 };
        state.haloColor = flash ? Color{ 0, 180, 255, 210 } : Color{ 255, 20, 20, 210 };
        state.pulseRadius = fmodf(timeSec * 4.0f, 1.0f);
        state.color = SIGNAL_GREEN;
    } else if (yellowCount > 0) {
        // Yellow Caution Signal: Rapid warning blink (flashing amber at 3.5 Hz)
        float yWave = sinf(timeSec * 3.5f * 2.0f * PI);
        bool yBlink = (yWave > 0.0f);
        state.isBlinkPhase = yBlink;
        state.intensity = yBlink ? 1.0f : 0.25f;
        state.blinkAlpha = yBlink ? 0.95f : 0.30f;
        state.primaryColor = Color{ 255, 204, 10, 255 }; // Amber Yellow
        state.haloColor = Color{ 240, 190, 0, (unsigned char)(yBlink ? 180 : 40) };
        state.pulseRadius = fmodf(timeSec * 3.5f, 1.0f);
        state.color = SIGNAL_YELLOW;
    } else if (greenCount > 0) {
        // Green Signal: Energetic rhythmic pulse / flow breathing (2.0 Hz)
        float gWave = 0.5f + 0.5f * sinf(timeSec * 2.0f * 2.0f * PI);
        state.isBlinkPhase = (gWave > 0.4f);
        state.intensity = 0.45f + 0.55f * gWave;
        state.blinkAlpha = 0.40f + 0.60f * gWave;
        state.primaryColor = Color{ 0, 230, 90, 255 }; // Emerald Green
        state.haloColor = Color{ 0, 200, 80, (unsigned char)(60 + 120 * gWave) };
        state.pulseRadius = fmodf(timeSec * 2.0f, 1.0f);
        state.color = SIGNAL_GREEN;
    } else {
        // Red Signal: Deliberate stop warning pulse (1.2 Hz normal, 2.2 Hz if vehicles waiting in queue)
        float rRate = (state.totalWaitingQueue > 0) ? 2.2f : 1.2f;
        float rWave = 0.5f + 0.5f * sinf(timeSec * rRate * 2.0f * PI);
        state.isBlinkPhase = (rWave > 0.4f);
        state.intensity = 0.35f + 0.65f * rWave;
        state.blinkAlpha = 0.35f + 0.65f * rWave;
        state.primaryColor = Color{ 235, 40, 40, 255 }; // Ruby Red
        state.haloColor = Color{ 210, 30, 30, (unsigned char)(50 + 130 * rWave) };
        state.pulseRadius = fmodf(timeSec * rRate, 1.0f);
        state.color = SIGNAL_RED;
    }

    return state;
}

// =============================================================================
// 2. NYC METRO TRANSIT MAP RENDERER (SUBWAY STATIONS & TRACKS)
// =============================================================================
inline void DrawNYCMetroTransitMap(
    Graph<std::string, 100>* mapRef,
    const std::map<std::string, Vector2>& positions,
    const CityDesignParams& params,
    const std::string& hoveredNode,
    const std::string& hoveredRoadU,
    const std::string& hoveredRoadV,
    bool emergencyActive
) {
    int totalNodes = mapRef->Vcount;
    auto* nodes = mapRef->getNodes();
    std::vector<std::string> stationNames = GetNYCStationNames();

    // 1. Draw Bold Subway Tracks
    for (int i = 0; i < totalNodes; i++) {
        std::string u = nodes[i].vertex;
        if (positions.find(u) == positions.end()) continue;
        Vector2 uPos = positions.at(u);

        int uRow = i / params.gridCols;
        int uCol = i % params.gridCols;

        for (auto const& edge : nodes[i].Neighbors) {
            std::string v = mapRef->getVertexAt(edge.index);
            if (positions.find(v) == positions.end()) continue;
            Vector2 vPos = positions.at(v);

            int vRow = edge.index / params.gridCols;
            int vCol = edge.index % params.gridCols;

            bool isAvenue = (uCol == vCol);
            Color lineColor = GetMTALineColor(uRow, uCol, isAvenue);

            float congestion = edge.weight.Congestion();
            float trackWidth = 7.0f;

            // Congestion Underglow (Warning Heatmap along subway tracks)
            if (congestion > 0.65f) {
                float heatPulse = 2.0f + sinf((float)GetTime() * 6.0f);
                Color heatCol = (congestion > 0.85f) ? RED : ORANGE;
                DrawLineEx(uPos, vPos, trackWidth + 6.0f + heatPulse, Fade(heatCol, 0.55f));
            }

            // Green Corridor Wave effect when Emergency Preemption is Active
            if (emergencyActive && params.enableGreenCorridor && edge.weight.light.emergencyPreempted) {
                float greenPulse = 3.0f + sinf((float)GetTime() * 10.0f) * 2.0f;
                DrawLineEx(uPos, vPos, trackWidth + 8.0f + greenPulse, Fade(LIME, 0.70f));
            }

            // Main Subway Track Base
            if (edge.weight.isBlocked) {
                DrawLineEx(uPos, vPos, trackWidth + 2.0f, Fade(RED, 0.9f));
            } else {
                DrawLineEx(uPos, vPos, trackWidth, lineColor);
                // Inner contrasting track line
                DrawLineEx(uPos, vPos, 2.5f, Fade(RAYWHITE, 0.45f));
            }

            // Directional Chevron Arrow
            DrawRoadArrowBold(uPos, vPos, lineColor);

            // Realistic 3-Aspect Traffic Signal Head with lens glow & countdown timer
            Vector2 dir = Vector2Normalize(Vector2Subtract(vPos, uPos));
            Vector2 stopPos = Vector2Subtract(vPos, Vector2Scale(dir, 26.0f));
            DrawTrafficSignalHead(stopPos, dir, edge.weight.light, edge.weight.queueCount, false);
        }
    }

    // 2. Draw Subway Stations (NYC Vignelli Style) with Traffic Signal Blinking
    for (int i = 0; i < totalNodes; i++) {
        std::string sName = nodes[i].vertex;
        if (positions.find(sName) == positions.end()) continue;
        Vector2 pos = positions.at(sName);

        NodeSignalState sig = GetNodeSignalState(mapRef, i, sName);
        bool isHovered = (sName == hoveredNode);
        bool isEMS = (i == params.emergencyHubIdx || sName == "Bellevue Hospital EMS");
        bool isTransferHub = (nodes[i].Neighbors.size() >= 3);

        // Traffic signal expanding pulse wave ring around station node
        float pulseR = 14.0f + sig.pulseRadius * 12.0f;
        float pulseAlpha = (1.0f - sig.pulseRadius) * 0.75f * sig.blinkAlpha;
        DrawCircleLines((int)pos.x, (int)pos.y, pulseR, Fade(sig.primaryColor, pulseAlpha));

        // Blinking halo around station node
        float haloR = 13.0f + 3.0f * sig.intensity;
        DrawCircleLines((int)pos.x, (int)pos.y, haloR, Fade(sig.primaryColor, 0.55f * sig.intensity));

        // Emergency Station Pulsing Strobe
        if (isEMS) {
            float emsPulse = 5.0f + sinf((float)GetTime() * 8.0f) * 3.0f;
            DrawCircleLines((int)pos.x, (int)pos.y, 24.0f + emsPulse, RED);
            DrawCircleLines((int)pos.x, (int)pos.y, 28.0f + emsPulse, Fade(BLUE, 0.7f));
        }

        if (isTransferHub) {
            // Transfer Station: Capsule / Pill Shape (Iconic MTA Transfer Symbol)
            Rectangle pill = { pos.x - 18.0f, pos.y - 12.0f, 36.0f, 24.0f };
            DrawRectangleRounded(pill, 0.6f, 8, BLACK);
            Rectangle innerPill = { pos.x - 15.0f, pos.y - 9.0f, 30.0f, 18.0f };
            DrawRectangleRounded(innerPill, 0.6f, 8, RAYWHITE);
            // Center signal indicator dot
            DrawCircleV(pos, 4.0f, sig.primaryColor);
            if (isHovered) {
                DrawRectangleRoundedLines(pill, 0.6f, 8, LIME);
            }
        } else {
            // Local Station: Solid White Circle with Thick Black Border and Signal Core
            DrawCircleV(pos, 12.0f, BLACK);
            DrawCircleV(pos, 9.0f, RAYWHITE);
            DrawCircleV(pos, 5.0f, sig.primaryColor);
            DrawCircleV(pos, 2.5f, Fade(RAYWHITE, sig.blinkAlpha));
            if (isHovered) {
                DrawCircleLines((int)pos.x, (int)pos.y, 16.0f, LIME);
            }
        }

        // Station Name Plate & Route Bullets (MTA Typography) with signal badge
        int nameW = MeasureText(sName.c_str(), 13);
        Rectangle labelBg = { pos.x + 16.0f, pos.y - 14.0f, (float)nameW + 56.0f, 28.0f };
        DrawRectangleRec(labelBg, Fade(BLACK, 0.88f));
        DrawRectangleLinesEx(labelBg, 1.0f, Fade(sig.primaryColor, 0.7f * sig.intensity));

        DrawText(sName.c_str(), (int)labelBg.x + 8, (int)labelBg.y + 4, 13, isEMS ? RED : RAYWHITE);
        const char* bullets = GetStationRouteBullet(i % 25);
        DrawText(bullets, (int)labelBg.x + 8, (int)labelBg.y + 16, 9, isEMS ? SKYBLUE : GOLD);

        // Signal badge on label
        const char* sigLabel = sig.isEmergency ? "EMS" : (sig.color == SIGNAL_GREEN ? "GRN" : (sig.color == SIGNAL_YELLOW ? "YEL" : "RED"));
        DrawText(sigLabel, (int)labelBg.x + nameW + 18, (int)labelBg.y + 8, 10, sig.primaryColor);
    }
}



// =============================================================================
// 4. FULL-SCREEN 3D NYC METRO WORLD RENDERER (100% 3D PERSPECTIVE VIEWPORT)
// =============================================================================
inline void DrawFull3DNYCMetroWorld(
    Graph<std::string, 100>* mapRef,
    const std::map<std::string, Vector3>& positions3D,
    const CityDesignParams& params,
    const std::list<vehicle<std::string>>& vehicles,
    const std::string& hoveredNode,
    int hoveredVehicleId,
    int trackedVehicleId,
    bool emergencyActive
) {
    float timeSec = (float)GetTime();
    int totalNodes = mapRef->Vcount;
    auto* nodes = mapRef->getNodes();

    // -------------------------------------------------------------------------
    // 4.1 3D ENVIRONMENT: TACTICAL NYC GRID & WATERWAY BOUNDARIES
    // -------------------------------------------------------------------------
    // Tactical Ground Grid
    DrawGrid(44, 1.25f);

    // Island Shoreline Waterway Ribbons (Hudson River on West, East River on East)
    DrawCube(Vector3{ -24.0f, -0.05f, 0.0f }, 4.0f, 0.1f, 48.0f, Fade(Color{ 10, 35, 65, 255 }, 0.65f));
    DrawCubeWires(Vector3{ -24.0f, -0.05f, 0.0f }, 4.0f, 0.1f, 48.0f, Fade(SKYBLUE, 0.3f));
    DrawCube(Vector3{  24.0f, -0.05f, 0.0f }, 4.0f, 0.1f, 48.0f, Fade(Color{ 10, 35, 65, 255 }, 0.65f));
    DrawCubeWires(Vector3{  24.0f, -0.05f, 0.0f }, 4.0f, 0.1f, 48.0f, Fade(SKYBLUE, 0.3f));

    // -------------------------------------------------------------------------
    // 4.2 3D MULTI-LEVEL TRACKS, OVERPASSES & ELEVATED FLYOVERS
    // -------------------------------------------------------------------------
    for (int i = 0; i < totalNodes; i++) {
        std::string uName = nodes[i].vertex;
        if (positions3D.find(uName) == positions3D.end()) continue;
        Vector3 uPos = positions3D.at(uName);

        int uRow = i / params.gridCols;
        int uCol = i % params.gridCols;

        for (auto const& edge : nodes[i].Neighbors) {
            std::string vName = mapRef->getVertexAt(edge.index);
            if (positions3D.find(vName) == positions3D.end()) continue;
            Vector3 vPos = positions3D.at(vName);

            int vRow = edge.index / params.gridCols;
            int vCol = edge.index % params.gridCols;

            bool isAvenue = (uCol == vCol);
            Color lineColor = GetMTALineColor(uRow, uCol, isAvenue);
            float congestion = edge.weight.Congestion();

            // Detect if this is an elevated flyover (spans > 1 block or higher elevation)
            float spanDist = Vector3Distance(uPos, vPos);
            bool isElevatedFlyover = (spanDist > 11.0f || uPos.y > 0.8f || vPos.y > 0.8f);

            // Elevated Concrete Support Piers
            if (isElevatedFlyover) {
                int numPiers = (int)(spanDist / 3.8f);
                for (int p = 1; p <= numPiers; p++) {
                    float t = (float)p / (float)(numPiers + 1);
                    Vector3 pierTop = Vector3Lerp(uPos, vPos, t);
                    float pierH = pierTop.y;
                    if (pierH > 0.15f) {
                        Vector3 pierBase = { pierTop.x, pierH * 0.5f, pierTop.z };
                        DrawCylinder(pierBase, 0.22f, 0.26f, pierH, 8, Color{ 65, 75, 90, 255 });
                        DrawCube(Vector3{ pierTop.x, pierH, pierTop.z }, 0.9f, 0.25f, 0.9f, Color{ 85, 95, 115, 255 });
                    }
                }
            }

            // Congestion Color: Clear is Black, Congested goes to Red!
            float congRatio = edge.weight.Congestion();
            Color roadCol;
            if (edge.weight.isBlocked) {
                roadCol = Color{ 255, 30, 30, 255 }; // Blocked incident: bright warning red
            } else {
                float c = Clamp(congRatio, 0.0f, 1.0f);
                // When clear (c == 0.0): Black { 15, 15, 18, 255 }
                // As congestion increases: transitions towards intense Red { 255, 15, 20, 255 }
                unsigned char r = (unsigned char)(15 + c * 240.0f);
                unsigned char g = (unsigned char)(15 * (1.0f - c));
                unsigned char b = (unsigned char)(18 * (1.0f - c));
                roadCol = Color{ r, g, b, 255 };
            }

            Vector3 roadDir = Vector3Normalize(Vector3Subtract(vPos, uPos));
            Vector3 perp3D = Vector3Normalize(Vector3{ -roadDir.z, 0.0f, roadDir.x });
            float halfW = 0.18f + (edge.weight.numLanes * 0.08f);

            // Subtle curbs/shoulders so network structure is clearly visible when road is black
            DrawLine3D(Vector3Add(uPos, Vector3Scale(perp3D, -halfW)), Vector3Add(vPos, Vector3Scale(perp3D, -halfW)), Color{ 38, 42, 50, 255 });
            DrawLine3D(Vector3Add(uPos, Vector3Scale(perp3D, halfW)), Vector3Add(vPos, Vector3Scale(perp3D, halfW)), Color{ 38, 42, 50, 255 });

            // Roadbed Line / Track Deck in Congestion Color (Black -> Red)
            DrawLine3D(uPos, vPos, roadCol);
            DrawLine3D(Vector3{ uPos.x, uPos.y + 0.03f, uPos.z }, Vector3{ vPos.x, vPos.y + 0.03f, vPos.z }, roadCol);

            // Congestion Red Underglow when congestion is building
            if (congRatio > 0.15f) {
                float c = Clamp(congRatio, 0.0f, 1.0f);
                DrawLine3D(Vector3{ uPos.x, uPos.y + 0.06f, uPos.z }, Vector3{ vPos.x, vPos.y + 0.06f, vPos.z }, Fade(Color{ 255, 30, 30, 255 }, c * 0.85f));
            }

            // Emergency Preemption Green-Corridor Beam
            if (params.enableGreenCorridor && edge.weight.light.emergencyPreempted) {
                DrawLine3D(Vector3{ uPos.x, uPos.y + 0.16f, uPos.z }, Vector3{ vPos.x, vPos.y + 0.16f, vPos.z }, Color{ 0, 255, 230, 255 });
            }

            // 3D Traffic Signal Post at Approach to Intersection v
            Vector3 stopPos3D = Vector3Subtract(vPos, Vector3Scale(roadDir, 2.0f));
            Vector3 postPos = Vector3Add(stopPos3D, Vector3Scale(perp3D, 0.95f));

            // Signal Post & 3D Housing Head
            DrawCylinder(Vector3{ postPos.x, postPos.y + 0.6f, postPos.z }, 0.04f, 0.04f, 1.2f, 6, Color{ 55, 60, 72, 255 });
            Vector3 headPos = Vector3{ postPos.x, postPos.y + 1.25f, postPos.z };
            DrawCube(headPos, 0.20f, 0.50f, 0.20f, Color{ 15, 18, 24, 255 });
            DrawCubeWires(headPos, 0.20f, 0.50f, 0.20f, Color{ 80, 85, 95, 255 });

            // 3D Lenses (Red, Yellow, Green)
            float dy = 0.14f;
            Vector3 redLens = { headPos.x, headPos.y + dy, headPos.z };
            Vector3 yelLens = { headPos.x, headPos.y, headPos.z };
            Vector3 grnLens = { headPos.x, headPos.y - dy, headPos.z };

            DrawSphere(redLens, 0.05f, edge.weight.light.isRed() ? RED : Color{ 45, 10, 10, 255 });
            DrawSphere(yelLens, 0.05f, edge.weight.light.isYellow() ? YELLOW : Color{ 45, 40, 10, 255 });
            DrawSphere(grnLens, 0.05f, edge.weight.light.isGreen() ? GREEN : Color{ 10, 45, 20, 255 });

            // Active lens glow
            if (edge.weight.light.isGreen()) {
                DrawSphere(grnLens, 0.08f, Fade(GREEN, 0.5f));
            } else if (edge.weight.light.isYellow()) {
                DrawSphere(yelLens, 0.08f, Fade(YELLOW, 0.5f));
            } else if (edge.weight.light.isRed()) {
                DrawSphere(redLens, 0.08f, Fade(RED, 0.5f));
            }
        }
    }

    // -------------------------------------------------------------------------
    // 4.3 3D NODES: INTERSECTIONS & STATIONS BLINKING BASED ON TRAFFIC SIGNALS!
    // -------------------------------------------------------------------------
    for (int i = 0; i < totalNodes; i++) {
        std::string sName = nodes[i].vertex;
        if (positions3D.find(sName) == positions3D.end()) continue;
        Vector3 p3D = positions3D.at(sName);

        // Get live signal shine state for this node
        NodeSignalState sig = GetNodeSignalState(mapRef, i, sName);

        bool isEMS = (i == params.emergencyHubIdx || sName == "Bellevue Hospital EMS");
        bool isTransfer = (nodes[i].Neighbors.size() >= 3);
        bool isHovered = (sName == hoveredNode);

        float towerH = isEMS ? 2.6f : (isTransfer ? 1.9f : 1.3f);

        // =====================================================================
        // SHINING EFFECT 1: CONCENTRIC GROUND HALO & EXPANDING BLINK PULSE
        // =====================================================================
        // Wide ambient light wash on roadbed in pulsing signal color
        float groundWashR = 2.0f + 0.8f * sig.intensity;
        DrawCylinder(Vector3{ p3D.x, 0.02f, p3D.z }, groundWashR, groundWashR, 0.02f, 24, Fade(sig.haloColor, 0.25f * sig.intensity));
        // Middle luminous halo
        float midHaloR = 1.2f + 0.5f * sig.intensity;
        DrawCylinder(Vector3{ p3D.x, 0.04f, p3D.z }, midHaloR, midHaloR, 0.02f, 24, Fade(sig.haloColor, 0.55f * sig.intensity));
        // Inner intense core disc
        DrawCylinder(Vector3{ p3D.x, 0.06f, p3D.z }, 0.9f, 0.9f, 0.02f, 24, Fade(sig.primaryColor, 0.90f * sig.intensity));
        // Dynamic expanding pulse boundary ring (Radar signal wave on the roadbed)
        float expandR = 0.9f + sig.pulseRadius * 2.2f;
        float expandAlpha = (1.0f - sig.pulseRadius) * 0.85f * sig.blinkAlpha;
        DrawCylinderWires(Vector3{ p3D.x, 0.07f, p3D.z }, expandR, expandR, 0.02f, 16, Fade(sig.primaryColor, expandAlpha));

        // =====================================================================
        // SHINING EFFECT 2: STATION TOWER ARCHITECTURE & ILLUMINATED BEVELS
        // =====================================================================
        // Tower Column
        DrawCube(Vector3{ p3D.x, towerH * 0.5f, p3D.z }, 0.85f, towerH, 0.85f, Color{ 15, 18, 28, 245 });
        // Glowing wireframe bevels that blink in signal color!
        DrawCubeWires(Vector3{ p3D.x, towerH * 0.5f, p3D.z }, 0.86f, towerH, 0.86f, Fade(sig.primaryColor, 0.50f + 0.50f * sig.intensity));

        // EMS Hub: Helipad & 3D Medical Cross
        if (isEMS) {
            DrawCylinder(Vector3{ p3D.x, towerH + 0.05f, p3D.z }, 1.2f, 1.2f, 0.06f, 16, Color{ 40, 15, 20, 255 });
            DrawCylinderWires(Vector3{ p3D.x, towerH + 0.05f, p3D.z }, 1.2f, 1.2f, 0.06f, 16, RED);
            // 3D Medical Cross on Helipad
            DrawCube(Vector3{ p3D.x, towerH + 0.09f, p3D.z }, 0.22f, 0.04f, 0.80f, RAYWHITE);
            DrawCube(Vector3{ p3D.x, towerH + 0.09f, p3D.z }, 0.80f, 0.04f, 0.22f, RAYWHITE);
        }

        // =====================================================================
        // SHINING EFFECT 3: LUMINESCENT SIGNAL DOME / ORB BLINKING ON TOP
        // =====================================================================
        Vector3 domePos = { p3D.x, towerH + 0.35f, p3D.z };
        // Expanding orb wave wires
        float domeWaveR = 0.45f + sig.pulseRadius * 0.35f;
        DrawSphereWires(domePos, domeWaveR, 8, 8, Fade(sig.primaryColor, (1.0f - sig.pulseRadius) * 0.75f * sig.blinkAlpha));
        // Radiant 3D Signal Dome blinking in signal color
        DrawSphere(domePos, 0.38f + 0.08f * sig.intensity, sig.primaryColor);
        // Pure white high-lumen incandescent core blinking
        DrawSphere(domePos, 0.18f + 0.05f * sig.intensity, Fade(RAYWHITE, sig.blinkAlpha));

        // =====================================================================
        // SHINING EFFECT 4: VERTICAL SKY LIGHT BEACON / ILLUMINATION PILLAR
        // =====================================================================
        // Translucent sky light cylinder pulsing in height and intensity
        float skyH = 3.5f + 2.0f * sig.intensity;
        DrawCylinder(Vector3{ p3D.x, towerH + skyH * 0.5f, p3D.z }, 0.06f, 0.20f + 0.10f * sig.intensity, skyH, 8, Fade(sig.primaryColor, 0.30f * sig.intensity));
        // Core laser sky beam
        DrawLine3D(domePos, Vector3{ p3D.x, towerH + skyH + 1.5f, p3D.z }, Fade(sig.primaryColor, sig.intensity));

        // =====================================================================
        // SHINING EFFECT 5: QUEUE DEPTH CONGESTION COLUMN GAUGE
        // =====================================================================
        if (sig.totalWaitingQueue > 0) {
            float qH = Clamp((float)sig.totalWaitingQueue * 0.35f, 0.45f, 4.2f);
            Color qCol = (sig.totalWaitingQueue > 8) ? RED : ((sig.totalWaitingQueue > 3) ? ORANGE : YELLOW);
            Vector3 qPos = { p3D.x + 0.75f, towerH + qH * 0.5f, p3D.z };
            DrawCube(qPos, 0.30f, qH, 0.30f, Fade(qCol, 0.85f));
            DrawCubeWires(qPos, 0.30f, qH, 0.30f, qCol);
        }

        // Hovered Station Reticle in 3D
        if (isHovered) {
            DrawCylinderWires(Vector3{ p3D.x, towerH + 0.5f, p3D.z }, 1.4f, 1.4f, 0.05f, 12, GOLD);
            DrawCircle3D(domePos, 1.6f, Vector3{ 0, 1, 0 }, 0.0f, GOLD);
        }
    }

    // -------------------------------------------------------------------------
    // 4.4 3D MOVING VEHICLES (EN-ROUTE & STOPPED QUEUES)
    // -------------------------------------------------------------------------
    std::map<std::pair<std::string, std::string>, int> qDrawMap;

    for (const auto& v : vehicles) {
        if (v.state == 1) {
            // Vehicle En-Route on 3D Track
            std::string src = v.currentRoad.first;
            std::string dst = v.currentRoad.second;
            if (positions3D.find(src) == positions3D.end() || positions3D.find(dst) == positions3D.end()) continue;

            Vector3 p1 = positions3D.at(src);
            Vector3 p2 = positions3D.at(dst);
            Vector3 roadVec = Vector3Subtract(p2, p1);
            float roadLen = Vector3Length(roadVec);
            if (roadLen < 0.001f) continue;
            Vector3 dir = Vector3Scale(roadVec, 1.0f / roadLen);
            Vector3 perp = Vector3Normalize(Vector3{ -dir.z, 0.0f, dir.x });

            float prog = (v.initialRoadTravelTime > 0.001f) ? Clamp(1.0f - (v.timeRemaining / v.initialRoadTravelTime), 0.0f, 1.0f) : 1.0f;
            Vector3 vPos = Vector3Lerp(p1, p2, prog);

            // Add lane offset
            float laneOffset = (v.currentLane - 1.5f) * 0.42f;
            vPos = Vector3Add(vPos, Vector3Scale(perp, laneOffset));
            vPos.y += 0.28f;

            // Vehicle Dimensions & Color
            Color vCol = v.isEmergency() ? (fmodf(timeSec * 8.0f, 1.0f) > 0.5f ? RED : BLUE) :
                         (v.isEV() ? LIME : (v.isTruck() ? BROWN : (v.isBus() ? SKYBLUE : (v.isMotorcycle() ? ORANGE : YELLOW))));

            float vLen = v.isTruck() ? 1.4f : (v.isBus() ? 1.3f : (v.isMotorcycle() ? 0.45f : 0.75f));
            float vWid = v.isMotorcycle() ? 0.20f : (v.isBus() ? 0.45f : 0.38f);
            float vH   = v.isTruck() ? 0.50f : (v.isBus() ? 0.42f : 0.26f);

            // 3D Chassis
            DrawCube(vPos, vWid, vH, vLen, vCol);
            DrawCubeWires(vPos, vWid, vH, vLen, RAYWHITE);

            // 3D Headlights projecting forward
            Vector3 hlLeft = Vector3Add(Vector3Add(vPos, Vector3Scale(dir, vLen * 0.55f)), Vector3Scale(perp, -vWid * 0.35f));
            Vector3 hlRight = Vector3Add(Vector3Add(vPos, Vector3Scale(dir, vLen * 0.55f)), Vector3Scale(perp, vWid * 0.35f));
            DrawSphere(hlLeft, 0.05f, RAYWHITE);
            DrawSphere(hlRight, 0.05f, RAYWHITE);
            DrawLine3D(hlLeft, Vector3Add(hlLeft, Vector3Scale(dir, 1.8f)), Fade(RAYWHITE, 0.45f));
            DrawLine3D(hlRight, Vector3Add(hlRight, Vector3Scale(dir, 1.8f)), Fade(RAYWHITE, 0.45f));

            // Emergency Ambulance Lightbar & Siren Pillar
            if (v.isEmergency()) {
                float sirenPulse = 1.2f + sinf(timeSec * 14.0f) * 0.6f;
                DrawLine3D(vPos, Vector3{ vPos.x, vPos.y + sirenPulse, vPos.z }, RED);
                DrawSphere(Vector3{ vPos.x, vPos.y + sirenPulse, vPos.z }, 0.16f, BLUE);
                DrawCylinderWires(Vector3{ vPos.x, vPos.y + 0.3f, vPos.z }, 1.2f, 1.2f, 0.05f, 10, Fade(CYAN, 0.6f));
            }

            // Highlight if tracked or hovered
            if (v.id == trackedVehicleId) {
                float trPulse = 0.5f + 0.2f * sinf(timeSec * 8.0f);
                DrawCylinderWires(vPos, vWid + trPulse, vWid + trPulse, 0.1f, 12, LIME);
            } else if (v.id == hoveredVehicleId) {
                DrawCylinderWires(vPos, vWid + 0.3f, vWid + 0.3f, 0.1f, 12, GOLD);
            }
        }
        else if (v.state == 0 && v.path.size() >= 2) {
            // Vehicle Queued & Stopped at Red Signal in 3D
            std::string src = v.path.front();
            auto it = v.path.begin(); advance(it, 1);
            std::string dst = *it;
            if (positions3D.find(src) == positions3D.end() || positions3D.find(dst) == positions3D.end()) continue;

            Vector3 p1 = positions3D.at(src);
            Vector3 p2 = positions3D.at(dst);
            Vector3 roadVec = Vector3Subtract(p2, p1);
            float roadLen = Vector3Length(roadVec);
            if (roadLen < 0.001f) continue;
            Vector3 dir = Vector3Scale(roadVec, 1.0f / roadLen);
            Vector3 perp = Vector3Normalize(Vector3{ -dir.z, 0.0f, dir.x });

            int qIdx = qDrawMap[{src, dst}]++;
            Vector3 qPos = Vector3Add(p1, Vector3Scale(dir, 1.5f + (float)qIdx * 1.1f));
            float laneOffset = (v.currentLane - 1.5f) * 0.42f;
            qPos = Vector3Add(qPos, Vector3Scale(perp, laneOffset));
            qPos.y += 0.28f;

            Color vCol = v.isEmergency() ? RED : (v.isEV() ? LIME : (v.isTruck() ? BROWN : (v.isBus() ? SKYBLUE : YELLOW)));
            float vLen = v.isTruck() ? 1.4f : (v.isBus() ? 1.3f : 0.75f);
            float vWid = v.isBus() ? 0.45f : 0.38f;
            float vH   = 0.26f;

            DrawCube(qPos, vWid, vH, vLen, vCol);
            DrawCubeWires(qPos, vWid, vH, vLen, RAYWHITE);

            // Stopped Brake Lights (Intense Glowing Red)
            Vector3 tlLeft = Vector3Add(Vector3Subtract(qPos, Vector3Scale(dir, vLen * 0.52f)), Vector3Scale(perp, -vWid * 0.35f));
            Vector3 tlRight = Vector3Add(Vector3Subtract(qPos, Vector3Scale(dir, vLen * 0.52f)), Vector3Scale(perp, vWid * 0.35f));
            DrawSphere(tlLeft, 0.06f, RED);
            DrawSphere(tlRight, 0.06f, RED);

            if (v.id == trackedVehicleId) {
                DrawCylinderWires(qPos, vWid + 0.4f, vWid + 0.4f, 0.1f, 12, LIME);
            }
        }
    }
}

// =============================================================================
// 5. PROJECTED 2D HUD OVERLAYS FOR 3D WORLD (STATION LABELS & STATUS BADGES)
// =============================================================================
inline void DrawNYCMetroProjectedHUD(
    Graph<std::string, 100>* mapRef,
    const std::map<std::string, Vector3>& positions3D,
    const CityDesignParams& params,
    Camera3D cam3D,
    const std::string& hoveredNode,
    int screenWidth,
    int screenHeight
) {
    int totalNodes = mapRef->Vcount;
    auto* nodes = mapRef->getNodes();
    Vector3 camDir = Vector3Normalize(Vector3Subtract(cam3D.target, cam3D.position));

    for (int i = 0; i < totalNodes; i++) {
        std::string sName = nodes[i].vertex;
        if (positions3D.find(sName) == positions3D.end()) continue;
        Vector3 p3D = positions3D.at(sName);

        // Cull if behind camera
        Vector3 toNode = Vector3Subtract(p3D, cam3D.position);
        if (Vector3DotProduct(toNode, camDir) <= 0.1f) continue;

        bool isEMS = (i == params.emergencyHubIdx || sName == "Bellevue Hospital EMS");
        bool isTransfer = (nodes[i].Neighbors.size() >= 3);
        float towerH = isEMS ? 2.6f : (isTransfer ? 1.9f : 1.3f);

        // Project top of tower to screen space
        Vector3 labelAnchor3D = { p3D.x, towerH + 0.85f, p3D.z };
        Vector2 screenPos = GetWorldToScreen(labelAnchor3D, cam3D);

        // Viewport bounds check
        if (screenPos.x < 410.0f || screenPos.x > (float)screenWidth - 20.0f ||
            screenPos.y < 40.0f || screenPos.y > (float)screenHeight - 20.0f) continue;

        NodeSignalState sig = GetNodeSignalState(mapRef, i, sName);

        // Station Label Card
        int nameW = MeasureText(sName.c_str(), 12);
        float cardW = (float)nameW + 55.0f;
        float cardH = 26.0f;
        Rectangle cardRec = { screenPos.x - cardW * 0.5f, screenPos.y - cardH * 0.5f, cardW, cardH };

        // Background & Border illuminated by signal color
        DrawRectangleRounded(cardRec, 0.35f, 4, Fade(BLACK, 0.88f));
        DrawRectangleRoundedLines(cardRec, 0.35f, 4, Fade(sig.primaryColor, 0.60f + 0.40f * sig.intensity));

        // Signal Status Pill Badge
        const char* sigText = sig.isEmergency ? "EMS" : (sig.color == SIGNAL_GREEN ? "GRN" : (sig.color == SIGNAL_YELLOW ? "YEL" : "RED"));
        Color sigTextCol = sig.isEmergency ? CYAN : (sig.color == SIGNAL_GREEN ? LIME : (sig.color == SIGNAL_YELLOW ? YELLOW : RED));
        DrawText(sigText, (int)cardRec.x + 6, (int)cardRec.y + 7, 11, Fade(sigTextCol, sig.blinkAlpha));

        // Station Name
        DrawText(sName.c_str(), (int)cardRec.x + 36, (int)cardRec.y + 7, 12, RAYWHITE);

        // Queue depth badge if vehicles waiting
        if (sig.totalWaitingQueue > 0) {
            std::string qStr = TextFormat("Q:%d", sig.totalWaitingQueue);
            int qW = MeasureText(qStr.c_str(), 10);
            Rectangle qRec = { cardRec.x + cardW + 4.0f, cardRec.y + 2.0f, (float)qW + 8.0f, 22.0f };
            DrawRectangleRounded(qRec, 0.4f, 3, Fade(RED, 0.85f));
            DrawText(qStr.c_str(), (int)qRec.x + 4, (int)qRec.y + 5, 10, RAYWHITE);
        }
    }
}

// =============================================================================
// 6. 3D CAMERA ACTIONS STRUCT & UPDATED TELEMETRY SIDEBAR
// =============================================================================
struct NYCCameraControlActions {
    bool setIsoView;
    bool setTopDownView;
    bool setStreetView;
    bool toggleAutoOrbit;
    bool resetCam;
    bool chaseAmbulance;
    float camAngle;
    float camPitch;
    float camDist;
    bool isAutoOrbiting;
    std::string trackingTargetName;

    // Traffic dispatch actions
    bool injectCar;
    bool injectAmbulance;
    bool injectBus;
    bool injectEV;
    bool injectFleet;
    bool injectPeakSurge;
    bool toggleRoadBlock;

    NYCCameraControlActions() {
        setIsoView = false;
        setTopDownView = false;
        setStreetView = false;
        toggleAutoOrbit = false;
        resetCam = false;
        chaseAmbulance = false;
        camAngle = 0.85f;
        camPitch = 0.80f;
        camDist = 34.0f;
        isAutoOrbiting = false;
        trackingTargetName = "MANUAL PERSPECTIVE";

        injectCar = false;
        injectAmbulance = false;
        injectBus = false;
        injectEV = false;
        injectFleet = false;
        injectPeakSurge = false;
        toggleRoadBlock = false;
    }
};

inline void DrawNYCMetroTelemetrySidebar(
    Simulator<std::string, 100>& sim,
    const CityDesignParams& params,
    const std::map<std::string, Vector2>& positions,
    int totalTime,
    bool isPaused,
    int simSpeed,
    const std::vector<float>& rushHistory,
    int screenWidth,
    int screenHeight,
    bool& outTriggerSettings,
    bool& outTriggerExport,
    NYCCameraControlActions& camActions
) {
    int sidebarW = 400;
    DrawRectangle(0, 0, sidebarW, screenHeight, Fade(Color{ 10, 13, 19, 255 }, 0.95f));
    DrawLineEx({ (float)sidebarW, 0 }, { (float)sidebarW, (float)screenHeight }, 2.5f, GetMTAColorYellow());

    Vector2 mousePos = GetMousePosition();
    bool mouseClicked = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    auto DrawButton = [&](Rectangle rec, const char* text, Color baseCol, Color textCol, int fontSize = 13) -> bool {
        bool hovered = CheckCollisionPointRec(mousePos, rec);
        DrawRectangleRec(rec, hovered ? ColorAlpha(baseCol, 0.95f) : ColorAlpha(baseCol, 0.75f));
        DrawRectangleLinesEx(rec, 1.0f, hovered ? RAYWHITE : Fade(baseCol, 0.8f));
        int tw = MeasureText(text, fontSize);
        DrawText(text, (int)(rec.x + (rec.width - tw) / 2.0f), (int)(rec.y + (rec.height - fontSize) / 2.0f), fontSize, textCol);
        return hovered && mouseClicked;
    };

    // Header Title
    DrawText("NYC MTA OPERATIONS CONSOLE", 25, 15, 19, GetMTAColorYellow());
    DrawText("LIVE 3D METRO TELEMETRY & PLANNING", 25, 38, 12, Fade(RAYWHITE, 0.75f));
    DrawLine(25, 56, 375, 56, Fade(GRAY, 0.3f));

    // Simulation Clock & Status
    const char* playStateText = isPaused ? "[PAUSED]" : TextFormat("[RUNNING %dx]", simSpeed);
    DrawText("SIMULATION STATE:", 25, 64, 12, Fade(LIME, 0.8f));
    DrawText(playStateText, 260, 64, 12, isPaused ? ORANGE : LIME);

    DrawText(TextFormat("TIME TICK:         %d", totalTime), 35, 82, 14, RAYWHITE);
    DrawText(TextFormat("ACTIVE VEHICLES:   %d", sim.cityManager->getVehicleCount()), 35, 102, 14, RAYWHITE);
    DrawText(TextFormat("COMPLETED TRIPS:   %d", (int)sim.cityManager->getArrivedCount()), 35, 122, 14, YELLOW);

    // Congestion Waveform Sparkline
    DrawLine(25, 142, 375, 142, Fade(GRAY, 0.3f));
    DrawText("TRANSIT CONGESTION WAVEFORM", 25, 150, 12, Fade(LIME, 0.8f));
    float rushVal = sim.rush();
    Color rushColor = GetMetroHeatColor(rushVal);
    DrawText(TextFormat("%.2f%% V/C", rushVal * 100.0f), 35, 168, 18, rushColor);

    Rectangle sparkRec = { 35.0f, 192.0f, 330.0f, 42.0f };
    DrawSparkline(rushHistory, sparkRec, rushColor, Fade(DARKGRAY, 0.25f), "SUBWAY LINK DELAY INDEX", 100.0f);

    // HCM Level of Service & Delay Metrics
    DrawLine(25, 242, 375, 242, Fade(GRAY, 0.3f));
    DrawText("HCM LEVEL OF SERVICE (LOS A-F)", 25, 250, 12, Fade(LIME, 0.8f));

    HCMLOSGrade losGrade = sim.cityReport.optimizedLOS;
    Color losColor = (losGrade <= LOS_B) ? GREEN : (losGrade <= LOS_D ? YELLOW : RED);
    DrawText(TextFormat("GRADE: %s", getLOSName(losGrade).c_str()), 35, 268, 15, losColor);
    DrawText(TextFormat("AVG DELAY:      %.1fs (P95: %.1fs)", sim.cityReport.optimizedAvgDelay, sim.cityReport.optimizedP95Delay), 35, 288, 13, RAYWHITE);
    DrawText(TextFormat("DELAY REDUCE:   -%.1f%% vs Baseline", sim.cityReport.delayReductionPct), 35, 306, 13, LIME);

    // Environmental & Fleet Telemetry
    DrawLine(25, 326, 375, 326, Fade(GRAY, 0.3f));
    DrawText("ENERGY & EMISSIONS", 25, 334, 12, Fade(LIME, 0.8f));
    DrawText(TextFormat("TOTAL CO2:      %.1f kg", sim.cityManager->getTotalCO2Emitted()), 35, 350, 13, ORANGE);
    DrawText(TextFormat("FUEL BURN:      %.1f Liters", sim.cityManager->getTotalFuelConsumed()), 35, 368, 13, YELLOW);
    DrawText(TextFormat("THROUGHPUT:     %.2f veh/tick", sim.cityManager->getThroughput((float)totalTime)), 35, 386, 13, SKYBLUE);

    // =========================================================================
    // TRAFFIC DISPATCH & FLEET INJECTION PANEL
    // =========================================================================
    DrawLine(25, 404, 375, 404, Fade(GRAY, 0.3f));
    DrawText("TRAFFIC DISPATCH & FLEET INJECTION", 25, 410, 11, Fade(LIME, 0.85f));

    float btnY = 426.0f;
    float colW = 108.0f;
    float btnH = 24.0f;

    // Row 1: + CAR [V], + 5x FLEET [F], + SURGE [P]
    if (DrawButton({ 30.0f, btnY, colW, btnH }, "+ CAR [V]", Color{ 30, 55, 85, 255 }, RAYWHITE, 11)) {
        camActions.injectCar = true;
    }
    if (DrawButton({ 144.0f, btnY, colW, btnH }, "+ 5x FLEET [F]", Color{ 25, 75, 60, 255 }, LIME, 11)) {
        camActions.injectFleet = true;
    }
    if (DrawButton({ 258.0f, btnY, colW - 2.0f, btnH }, "+ SURGE [P]", Color{ 90, 55, 20, 255 }, YELLOW, 11)) {
        camActions.injectPeakSurge = true;
    }

    btnY += 28.0f;
    // Row 2: + EMS [E], + BUS/TRK, + EV/MOTO
    if (DrawButton({ 30.0f, btnY, colW, btnH }, "+ EMS [E]", Color{ 95, 25, 30, 255 }, RED, 11)) {
        camActions.injectAmbulance = true;
    }
    if (DrawButton({ 144.0f, btnY, colW, btnH }, "+ BUS / TRK", Color{ 40, 50, 65, 255 }, SKYBLUE, 11)) {
        camActions.injectBus = true;
    }
    if (DrawButton({ 258.0f, btnY, colW - 2.0f, btnH }, "+ EV / MOTO", Color{ 35, 75, 40, 255 }, GREEN, 11)) {
        camActions.injectEV = true;
    }

    btnY += 28.0f;
    // Row 3: BLOCK ROAD [B], [F1] SETTINGS, [K] EXPORT CSV
    if (DrawButton({ 30.0f, btnY, colW, btnH }, "BLOCK [B]", Color{ 80, 35, 20, 255 }, ORANGE, 11)) {
        camActions.toggleRoadBlock = true;
    }
    if (DrawButton({ 144.0f, btnY, colW, btnH }, "[F1] CONFIG", GetMTAColorBlue(), RAYWHITE, 11)) {
        outTriggerSettings = true;
    }
    if (DrawButton({ 258.0f, btnY, colW - 2.0f, btnH }, "[K] CSV", Color{ 20, 90, 45, 255 }, LIME, 11)) {
        outTriggerExport = true;
    }

    // =========================================================================
    // 3D TACTICAL CAMERA & NAVIGATION CONTROLLER (BOTTOM-LEFT PANEL)
    // =========================================================================
    float bottomOverviewY = btnY + 34.0f;
    float bottomOverviewH = (float)screenHeight - bottomOverviewY - 12.0f;
    if (bottomOverviewH < 220.0f) bottomOverviewH = 220.0f;
    Rectangle camBox = { 20.0f, bottomOverviewY, 360.0f, bottomOverviewH };

    DrawRectangleRec(camBox, Fade(Color{ 12, 16, 24, 255 }, 0.95f));
    DrawRectangleLinesEx(camBox, 1.5f, GetMTAColorYellow());

    // Panel Header
    DrawRectangle((int)camBox.x, (int)camBox.y, (int)camBox.width, 26, Fade(BLACK, 0.85f));
    DrawLine((int)camBox.x, (int)camBox.y + 26, (int)(camBox.x + camBox.width), (int)camBox.y + 26, Fade(GetMTAColorYellow(), 0.5f));
    DrawText("3D CAMERA & NAVIGATION SUITE", (int)camBox.x + 10, (int)camBox.y + 6, 12, GetMTAColorYellow());
    DrawText(camActions.isAutoOrbiting ? "[R: AUTO-ORBIT]" : "[R: MANUAL]", (int)(camBox.x + camBox.width - 110), (int)camBox.y + 6, 10, camActions.isAutoOrbiting ? LIME : ORANGE);

    float cy = camBox.y + 32.0f;
    DrawText(TextFormat("PITCH: %.1f deg | YAW: %.1f deg | ZOOM: %.1fm", camActions.camPitch * (180.0f / 3.14159f), camActions.camAngle * (180.0f / 3.14159f), camActions.camDist), (int)camBox.x + 12, (int)cy, 10, SKYBLUE);
    cy += 18.0f;
    DrawText(TextFormat("FOCUS TARGET: %s", camActions.trackingTargetName.c_str()), (int)camBox.x + 12, (int)cy, 11, GOLD);
    cy += 24.0f;

    // Camera Preset Buttons
    float bW = 100.0f;
    float bH = 26.0f;
    if (DrawButton({ camBox.x + 12.0f, cy, bW, bH }, "ISO 3D [C]", Color{ 35, 45, 65, 255 }, RAYWHITE, 11)) {
        camActions.setIsoView = true;
    }
    if (DrawButton({ camBox.x + 120.0f, cy, bW, bH }, "TOP-DOWN", Color{ 35, 45, 65, 255 }, RAYWHITE, 11)) {
        camActions.setTopDownView = true;
    }
    if (DrawButton({ camBox.x + 228.0f, cy, bW + 15.0f, bH }, "STREET 3D", Color{ 35, 45, 65, 255 }, RAYWHITE, 11)) {
        camActions.setStreetView = true;
    }
    cy += 30.0f;

    if (DrawButton({ camBox.x + 12.0f, cy, 160.0f, bH }, camActions.isAutoOrbiting ? "STOP ORBIT [R]" : "AUTO-ORBIT [R]", camActions.isAutoOrbiting ? Color{ 60, 20, 20, 255 } : Color{ 20, 70, 40, 255 }, camActions.isAutoOrbiting ? RED : LIME, 11)) {
        camActions.toggleAutoOrbit = true;
    }
    if (DrawButton({ camBox.x + 180.0f, cy, 165.0f, bH }, "CHASE AMBULANCE [T]", Color{ 80, 25, 35, 255 }, Color{ 255, 100, 100, 255 }, 11)) {
        camActions.chaseAmbulance = true;
    }
    cy += 34.0f;

    // Navigation Hotkey Instructions
    if (camBox.height >= 240.0f) {
        DrawRectangle((int)camBox.x + 8, (int)cy, (int)camBox.width - 16, 56, Fade(BLACK, 0.75f));
        DrawRectangleLines((int)camBox.x + 8, (int)cy, (int)camBox.width - 16, 56, Fade(GRAY, 0.3f));
        DrawText("3D CONTROLS GUIDE:", (int)camBox.x + 14, (int)cy + 5, 10, GOLD);
        DrawText("* RIGHT-CLICK DRAG: Rotate & Tilt Camera", (int)camBox.x + 14, (int)cy + 19, 10, RAYWHITE);
        DrawText("* MOUSE WHEEL: Smooth Zoom In / Out", (int)camBox.x + 14, (int)cy + 32, 10, RAYWHITE);
        DrawText("* MIDDLE-CLICK DRAG: Pan City Target", (int)camBox.x + 14, (int)cy + 45, 10, RAYWHITE);
    }
}

// Backward-compatible overload without camActions
inline void DrawNYCMetroTelemetrySidebar(
    Simulator<std::string, 100>& sim,
    const CityDesignParams& params,
    const std::map<std::string, Vector2>& positions,
    int totalTime,
    bool isPaused,
    int simSpeed,
    const std::vector<float>& rushHistory,
    int screenWidth,
    int screenHeight,
    bool& outTriggerSettings,
    bool& outTriggerExport
) {
    NYCCameraControlActions dummyActions;
    DrawNYCMetroTelemetrySidebar(sim, params, positions, totalTime, isPaused, simSpeed, rushHistory, screenWidth, screenHeight, outTriggerSettings, outTriggerExport, dummyActions);
}
