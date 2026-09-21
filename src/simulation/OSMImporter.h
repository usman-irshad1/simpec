#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <cmath>
#include <algorithm>
#include "raylib.h"
#include "raymath.h"
#include "../core/Graph.h"
#include "../core/RoadDetails.h"
#include "Simulator.h"
#include "CityDesigner.h"

// =============================================================================
// OPENSTREETMAP (OSM) DATA MODEL & XML PARSER
// =============================================================================

struct OSMNode {
    std::string id;
    double lat;
    double lon;
    std::string name;
    bool hasTrafficSignal;
    int referenceCount; // Number of ways that reference this node

    OSMNode() : lat(0.0), lon(0.0), hasTrafficSignal(false), referenceCount(0) {}
};

struct OSMWay {
    std::string id;
    std::string name;
    std::string highwayType; // motorway, trunk, primary, secondary, tertiary, residential
    int lanes;
    float maxSpeedKmh;
    bool isOneWay;
    std::vector<std::string> nodeRefs;

    OSMWay() : lanes(2), maxSpeedKmh(50.0f), isOneWay(false) {}
};

struct OSMMapData {
    std::string filePath;
    std::string title;
    double minLat, maxLat;
    double minLon, maxLon;
    std::unordered_map<std::string, OSMNode> nodes;
    std::vector<OSMWay> ways;
    int activeIntersectionsCount;
    int activeRoadsCount;

    OSMMapData() : minLat(90.0), maxLat(-90.0), minLon(180.0), maxLon(-180.0),
                   activeIntersectionsCount(0), activeRoadsCount(0) {}
};

// Helper: Trim whitespace
inline std::string OSMTrim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

// Helper: Extract XML attribute value by key
inline std::string OSMExtractAttribute(const std::string& line, const std::string& attr) {
    std::string needle = attr + "=\"";
    size_t start = line.find(needle);
    if (start == std::string::npos) {
        needle = attr + "='";
        start = line.find(needle);
        if (start == std::string::npos) return "";
    }
    start += needle.length();
    char quote = line[start - 1];
    size_t end = line.find(quote, start);
    if (end == std::string::npos) return "";
    return line.substr(start, end - start);
}

// =============================================================================
// FAST XML OPENSTREETMAP (.OSM) PARSER
// =============================================================================
inline bool ParseOSMFile(const std::string& filePath, OSMMapData& outMap) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "OSMImporter: Failed to open " << filePath << std::endl;
        return false;
    }

    outMap = OSMMapData();
    outMap.filePath = filePath;

    // Extract title from filename (e.g. "data/osm/lahore_mall_road.osm" -> "Lahore Mall Road")
    std::string base = filePath;
    size_t slash = base.find_last_of("/\\");
    if (slash != std::string::npos) base = base.substr(slash + 1);
    size_t dot = base.find_last_of(".");
    if (dot != std::string::npos) base = base.substr(0, dot);
    for (size_t i = 0; i < base.length(); i++) {
        if (base[i] == '_') base[i] = ' ';
        else if (i == 0 || base[i - 1] == ' ') base[i] = (char)toupper(base[i]);
    }
    outMap.title = base;

    std::string line;
    bool inNode = false;
    bool inWay = false;
    OSMNode currentNode;
    OSMWay currentWay;

    while (std::getline(file, line)) {
        line = OSMTrim(line);
        if (line.empty()) continue;

        // 1. Parse Bounds
        if (line.find("<bounds") != std::string::npos) {
            std::string minlat = OSMExtractAttribute(line, "minlat");
            std::string maxlat = OSMExtractAttribute(line, "maxlat");
            std::string minlon = OSMExtractAttribute(line, "minlon");
            std::string maxlon = OSMExtractAttribute(line, "maxlon");
            if (!minlat.empty()) outMap.minLat = std::stod(minlat);
            if (!maxlat.empty()) outMap.maxLat = std::stod(maxlat);
            if (!minlon.empty()) outMap.minLon = std::stod(minlon);
            if (!maxlon.empty()) outMap.maxLon = std::stod(maxlon);
            continue;
        }

        // 2. Parse Node
        if (line.find("<node") != std::string::npos) {
            currentNode = OSMNode();
            currentNode.id = OSMExtractAttribute(line, "id");
            std::string latStr = OSMExtractAttribute(line, "lat");
            std::string lonStr = OSMExtractAttribute(line, "lon");
            if (!latStr.empty()) currentNode.lat = std::stod(latStr);
            if (!lonStr.empty()) currentNode.lon = std::stod(lonStr);

            // Update bounding box if not explicitly set
            if (currentNode.lat < outMap.minLat) outMap.minLat = currentNode.lat;
            if (currentNode.lat > outMap.maxLat) outMap.maxLat = currentNode.lat;
            if (currentNode.lon < outMap.minLon) outMap.minLon = currentNode.lon;
            if (currentNode.lon > outMap.maxLon) outMap.maxLon = currentNode.lon;

            // Self-closing node
            if (line.find("/>") != std::string::npos) {
                outMap.nodes[currentNode.id] = currentNode;
            } else {
                inNode = true;
            }
            continue;
        }

        if (inNode) {
            if (line.find("<tag") != std::string::npos) {
                std::string k = OSMExtractAttribute(line, "k");
                std::string v = OSMExtractAttribute(line, "v");
                if (k == "name") currentNode.name = v;
                else if (k == "highway" && v == "traffic_signals") currentNode.hasTrafficSignal = true;
            } else if (line.find("</node>") != std::string::npos) {
                inNode = false;
                outMap.nodes[currentNode.id] = currentNode;
            }
            continue;
        }

        // 3. Parse Way
        if (line.find("<way") != std::string::npos) {
            currentWay = OSMWay();
            currentWay.id = OSMExtractAttribute(line, "id");
            inWay = true;
            continue;
        }

        if (inWay) {
            if (line.find("<nd") != std::string::npos) {
                std::string ref = OSMExtractAttribute(line, "ref");
                if (!ref.empty()) {
                    currentWay.nodeRefs.push_back(ref);
                    if (outMap.nodes.find(ref) != outMap.nodes.end()) {
                        outMap.nodes[ref].referenceCount++;
                    }
                }
            } else if (line.find("<tag") != std::string::npos) {
                std::string k = OSMExtractAttribute(line, "k");
                std::string v = OSMExtractAttribute(line, "v");
                if (k == "name") {
                    currentWay.name = v;
                } else if (k == "highway") {
                    currentWay.highwayType = v;
                } else if (k == "lanes") {
                    try { currentWay.lanes = std::stoi(v); } catch (...) { currentWay.lanes = 2; }
                } else if (k == "maxspeed") {
                    try { currentWay.maxSpeedKmh = std::stof(v); } catch (...) { currentWay.maxSpeedKmh = 50.0f; }
                } else if (k == "oneway") {
                    currentWay.isOneWay = (v == "yes" || v == "1" || v == "true");
                }
            } else if (line.find("</way>") != std::string::npos) {
                inWay = false;
                // Only keep navigable vehicular highways
                if (!currentWay.highwayType.empty() && currentWay.nodeRefs.size() >= 2) {
                    if (currentWay.highwayType != "footway" && currentWay.highwayType != "pedestrian" &&
                        currentWay.highwayType != "steps" && currentWay.highwayType != "cycleway") {
                        outMap.ways.push_back(currentWay);
                    }
                }
            }
            continue;
        }
    }

    file.close();
    outMap.activeRoadsCount = (int)outMap.ways.size();
    return !outMap.nodes.empty() && !outMap.ways.empty();
}

// =============================================================================
// CONVERT OSM DATA TO LIVE SIMULATOR & RAYLIB 3D/2D COORDINATES
// =============================================================================
inline bool ConvertOSMToSimulator(
    const OSMMapData& osmData,
    Simulator<std::string, 100>& sim,
    std::map<std::string, Vector2>& outPositions2D,
    std::map<std::string, Vector3>& outPositions3D,
    int screenWidth,
    int screenHeight,
    int commuterCount = 45
) {
    if (osmData.nodes.empty() || osmData.ways.empty()) return false;

    sim.resetSimulation();
    outPositions2D.clear();
    outPositions3D.clear();

    Graph<std::string, 100>* mapRef = sim.getMap();

    // 1. Identify Critical Intersection Nodes
    // A node is an intersection if it is referenced by multiple ways, or is an endpoint,
    // or has a traffic signal, or has a custom name.
    std::unordered_map<std::string, bool> isIntersection;
    for (const auto& way : osmData.ways) {
        if (way.nodeRefs.empty()) continue;
        isIntersection[way.nodeRefs.front()] = true;
        isIntersection[way.nodeRefs.back()] = true;
        for (const auto& ref : way.nodeRefs) {
            auto it = osmData.nodes.find(ref);
            if (it != osmData.nodes.end()) {
                if (it->second.referenceCount >= 2 || it->second.hasTrafficSignal || !it->second.name.empty()) {
                    isIntersection[ref] = true;
                }
            }
        }
    }

    // Limit active intersections to graph capacity (size = 100)
    const int MAX_NODES = 85;
    std::vector<std::string> activeNodeIds;
    for (const auto& pair : osmData.nodes) {
        if (isIntersection[pair.first]) {
            activeNodeIds.push_back(pair.first);
            if ((int)activeNodeIds.size() >= MAX_NODES) break;
        }
    }
    // Fallback: if too few intersections detected, include more nodes
    if (activeNodeIds.size() < 4) {
        for (const auto& pair : osmData.nodes) {
            if (std::find(activeNodeIds.begin(), activeNodeIds.end(), pair.first) == activeNodeIds.end()) {
                activeNodeIds.push_back(pair.first);
                if ((int)activeNodeIds.size() >= MAX_NODES) break;
            }
        }
    }

    std::unordered_map<std::string, std::string> nodeIdToVertexName;
    int namedCounter = 1;

    // Center of bounding box for Cartesian projection
    double centerLat = (osmData.minLat + osmData.maxLat) * 0.5;
    double centerLon = (osmData.minLon + osmData.maxLon) * 0.5;
    double latRad = centerLat * (3.14159265358979323846 / 180.0);

    // Find local Cartesian metric span (in km)
    float minX = 1e9f, maxX = -1e9f, minZ = 1e9f, maxZ = -1e9f;
    std::unordered_map<std::string, Vector2> localMetricCoords;

    for (const auto& id : activeNodeIds) {
        const OSMNode& node = osmData.nodes.at(id);
        // Equirectangular metric projection (km)
        float xKm = (float)((node.lon - centerLon) * cos(latRad) * 111.32);
        float zKm = (float)((node.lat - centerLat) * 110.57);
        localMetricCoords[id] = { xKm, zKm };

        if (xKm < minX) minX = xKm;
        if (xKm > maxX) maxX = xKm;
        if (zKm < minZ) minZ = zKm;
        if (zKm > maxZ) maxZ = zKm;

        // Generate friendly vertex name
        std::string vName = node.name;
        if (vName.empty()) {
            vName = "Junction " + std::to_string(namedCounter++);
        }
        nodeIdToVertexName[id] = vName;

        // Insert into Graph with real GPS coordinates
        mapRef->insertVertex(vName);
        mapRef->setVertexCoordinates(vName, (float)node.lat, (float)node.lon);
    }

    // 2. Compute 2D Screen and 3D World Layout Projections
    float spanX = maxX - minX;
    float spanZ = maxZ - minZ;
    if (spanX < 0.05f) spanX = 0.05f;
    if (spanZ < 0.05f) spanZ = 0.05f;
    float maxSpan = std::max(spanX, spanZ);

    // 2D Viewport Bounds
    float viewX = 420.0f;
    float viewW = (float)(screenWidth - 460);
    float viewH = (float)(screenHeight - 140);
    float centerX = viewX + viewW * 0.5f;
    float centerY = 90.0f + viewH * 0.5f;
    float scale2D = (std::min(viewW, viewH) - 120.0f) / maxSpan;

    // 3D Perspective World Bounds
    float scale3D = 36.0f / maxSpan;

    for (const auto& id : activeNodeIds) {
        std::string vName = nodeIdToVertexName[id];
        Vector2 m = localMetricCoords[id];

        // 2D: Centered and scaled
        Vector2 pos2D = {
            centerX + m.x * scale2D,
            centerY - m.y * scale2D // Invert Y for screen space
        };
        outPositions2D[vName] = pos2D;

        // 3D: Ground plane X-Z, Y elevation
        const OSMNode& node = osmData.nodes.at(id);
        float pY = node.hasTrafficSignal ? 0.65f : 0.25f;
        Vector3 pos3D = {
            m.x * scale3D,
            pY,
            -m.y * scale3D // Invert Z for OpenGL camera convention
        };
        outPositions3D[vName] = pos3D;
    }

    // 3. Connect Roads & Highways along Way Node Sequences
    for (const auto& way : osmData.ways) {
        std::string lastActiveId = "";
        for (const auto& ref : way.nodeRefs) {
            if (nodeIdToVertexName.find(ref) != nodeIdToVertexName.end()) {
                if (!lastActiveId.empty()) {
                    std::string uName = nodeIdToVertexName[lastActiveId];
                    std::string vName = nodeIdToVertexName[ref];
                    int uIdx = mapRef->getIndex(uName);
                    int vIdx = mapRef->getIndex(vName);

                    if (uIdx != -1 && vIdx != -1 && uIdx != vIdx) {
                        // Calculate real geodetic distance in km
                        const OSMNode& n1 = osmData.nodes.at(lastActiveId);
                        const OSMNode& n2 = osmData.nodes.at(ref);
                        float lenKm = Graph<std::string, 100>::haversineDistanceKm((float)n1.lat, (float)n1.lon, (float)n2.lat, (float)n2.lon);
                        if (lenKm < 0.25f) lenKm = 0.25f; // Min link length for realistic physics

                        // Infer speed limit and capacity based on OSM highway classification
                        float spd = way.maxSpeedKmh;
                        int lanes = way.lanes;
                        float cap = 45.0f;
                        float alpha = 0.15f;
                        float beta = 4.0f;

                        if (way.highwayType == "motorway" || way.highwayType == "trunk") {
                            spd = std::max(spd, 80.0f);
                            lanes = std::max(lanes, 3);
                            cap = 95.0f;
                            alpha = 0.08f;
                        } else if (way.highwayType == "primary") {
                            spd = std::max(spd, 60.0f);
                            lanes = std::max(lanes, 3);
                            cap = 75.0f;
                            alpha = 0.10f;
                        } else if (way.highwayType == "secondary") {
                            spd = std::max(spd, 50.0f);
                            lanes = std::max(lanes, 2);
                            cap = 60.0f;
                        } else if (way.highwayType == "tertiary") {
                            spd = std::max(spd, 40.0f);
                            lanes = std::max(lanes, 2);
                            cap = 45.0f;
                        } else {
                            spd = std::max(spd, 35.0f);
                            lanes = 1;
                            cap = 35.0f;
                        }

                        mapRef->makeEdge(uIdx, vIdx, lenKm, spd, cap, alpha, beta);
                        if (!way.isOneWay) {
                            mapRef->makeEdge(vIdx, uIdx, lenKm, spd, cap, alpha, beta);
                        }
                    }
                }
                lastActiveId = ref;
            }
        }
    }

    // 4. Configure Traffic Signals & Approaches
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

    // Run Initial Genetic Algorithm Signal Optimization
    sim.optimizeCitySignals(true);

    // 5. Spawn Configured Multi-Class Fleet across the Real-World Network
    static int osmCarSeq = 92000;
    int totalV = mapRef->Vcount;
    if (totalV >= 2) {
        for (int k = 0; k < commuterCount; k++) {
            int u = rand() % totalV;
            int v = rand() % totalV;
            int tries = 0;
            while (tries < 20 && (u == v || abs(u - v) < 2)) {
                v = rand() % totalV;
                tries++;
            }
            if (u == v) v = (u + 1) % totalV;

            int roll = rand() % 100;
            VehicleType vt = VEHICLE_CAR;
            if (roll < 55) vt = VEHICLE_CAR;
            else if (roll < 75) vt = VEHICLE_BUS;
            else if (roll < 85) vt = VEHICLE_TRUCK;
            else if (roll < 95) vt = VEHICLE_EV;
            else vt = VEHICLE_MOTORCYCLE;

            sim.cityManager->addVehicle(osmCarSeq++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), vt);
        }

        // Add standby ambulance
        sim.cityManager->addVehicle(99999, mapRef->getVertexAt(0), mapRef->getVertexAt(totalV - 1), VEHICLE_EMERGENCY);
    }

    return true;
}

// =============================================================================
// HELPER: LIST AVAILABLE OSM MAPS IN data/osm/
// =============================================================================
inline std::vector<std::pair<std::string, std::string>> GetAvailableOSMMaps() {
    std::vector<std::pair<std::string, std::string>> maps;
    maps.push_back({ "data/osm/lahore_mall_road.osm", "Lahore Mall Road (Pakistan)" });
    maps.push_back({ "data/osm/london_westminster.osm", "London Westminster (UK)" });
    return maps;
}
