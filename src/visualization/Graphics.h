#pragma once
#include <string>
#include <vector>
#include <map>
#include "raylib.h"
#include "raymath.h"
#include "../core/vehicle.h"
#include "../core/TrafficSignal.h"

// Layout mode for the map visualization
enum MapLayoutMode {
    LAYOUT_RADIAL_MULTAN = 0,   // Multan hub at center with regional cities in radial circle
    LAYOUT_GEOGRAPHIC_GPS = 1   // Real Pakistan GPS Latitude/Longitude projection
};

// Real-world GPS Latitude & Longitude coordinate
struct GeoCoord {
    float lat;
    float lon;
};

// Helper: Return real GPS coordinates for all 11 Pakistani cities in the network
inline std::map<std::string, GeoCoord> GetPakistanGeoCoordinates() {
    std::map<std::string, GeoCoord> coords;
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
    coords["Gwadar"]     = { 25.1216f, 62.3254f };
    return coords;
}

// Helper: Return Urban City Planning Sector coordinates
inline std::map<std::string, GeoCoord> GetCityPlanningCoordinates() {
    std::map<std::string, GeoCoord> coords;
    coords["Downtown Central"]  = { 33.6844f, 73.0479f };
    coords["North Boulevard"]   = { 33.7250f, 73.0479f };
    coords["South Commercial"]  = { 33.6400f, 73.0479f };
    coords["East Gateway"]      = { 33.6844f, 73.1000f };
    coords["West Sector"]       = { 33.6844f, 72.9900f };
    coords["Tech Park"]         = { 33.7180f, 73.0950f };
    coords["University Town"]   = { 33.7200f, 72.9950f };
    coords["Industrial Estate"] = { 33.6450f, 73.0900f };
    coords["City Hospital"]     = { 33.6500f, 73.0050f };
    coords["Airport Avenue"]    = { 33.6700f, 73.1400f };
    coords["Old Town Circle"]   = { 33.7500f, 73.0479f };
    return coords;
}

inline std::map<std::string, Vector2> ComputeFantasyRealmPositions(int screenWidth, int screenHeight) {
    std::map<std::string, Vector2> positions;
    float viewX = 420.0f;
    float viewW = (float)(screenWidth - 460);
    float viewH = (float)(screenHeight - 140);
    float centerX = viewX + viewW / 2.0f;
    float centerY = 70.0f + viewH / 2.0f;

    float spanX = viewW * 0.35f;
    float spanY = viewH * 0.35f;

    positions["Mount Olympus"] = { centerX, centerY - spanY };       // North Celestial Peak
    positions["Tartarus"]      = { centerX, centerY + spanY };       // South Nether Abyss
    positions["Atlantis"]      = { centerX - spanX, centerY };       // West Sunken Sea
    positions["Elysium"]       = { centerX + spanX, centerY };       // East Golden Meadow

    return positions;
}

inline std::map<std::string, Vector2> ComputeFantasyCityPositions(int screenWidth, int screenHeight, const std::string& cityName) {
    std::map<std::string, Vector2> positions;
    float viewX = 420.0f;
    float viewW = (float)(screenWidth - 460);
    float viewH = (float)(screenHeight - 140);
    float cX = viewX + viewW / 2.0f;
    float cY = 70.0f + viewH / 2.0f;

    float dx = viewW * 0.17f;
    float dy = viewH * 0.18f;

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

    positions[urbanHubs[0]]  = { cX, cY };
    positions[urbanHubs[1]]  = { cX, cY - dy * 1.25f };
    positions[urbanHubs[2]]  = { cX, cY + dy * 1.25f };
    positions[urbanHubs[3]]  = { cX + dx * 1.35f, cY };
    positions[urbanHubs[4]]  = { cX - dx * 1.35f, cY };
    positions[urbanHubs[5]]  = { cX + dx * 1.15f, cY - dy * 1.05f };
    positions[urbanHubs[6]]  = { cX - dx * 1.15f, cY - dy * 1.05f };
    positions[urbanHubs[7]]  = { cX + dx * 1.15f, cY + dy * 1.05f };
    positions[urbanHubs[8]]  = { cX - dx * 1.15f, cY + dy * 1.05f };
    positions[urbanHubs[9]]  = { cX + dx * 2.1f, cY };
    positions[urbanHubs[10]] = { cX, cY - dy * 2.05f };

    return positions;
}

// Helper: Shortest distance from point P to line segment AB
inline float DistanceToSegment(Vector2 p, Vector2 a, Vector2 b) {
    Vector2 ab = Vector2Subtract(b, a);
    float l2 = Vector2LengthSqr(ab);
    if (l2 <= 0.0001f) return Vector2Distance(p, a);
    float t = Vector2DotProduct(Vector2Subtract(p, a), ab) / l2;
    t = Clamp(t, 0.0f, 1.0f);
    Vector2 projection = Vector2Add(a, Vector2Scale(ab, t));
    return Vector2Distance(p, projection);
}

// Helper to draw bold directional arrows on road links
void DrawRoadArrowBold(Vector2 start, Vector2 end, Color col);

// Helper to map congestion ratio [0.0, 1.0] to a Metro heatmap color (Green -> Yellow -> Orange -> Red)
Color GetMetroHeatColor(float rush);

// Helper to draw a sleek live sparkline chart inside the UI sidebar
void DrawSparkline(const std::vector<float>& history, Rectangle bounds, Color lineCol, Color bgCol, const char* label, float maxVal = 100.0f);

// Realistic 3-Aspect Traffic Signal Head with lens glow, visors, countdown timer & queue badge
void DrawTrafficSignalHead(Vector2 postPos, Vector2 roadDir, const TrafficSignal& signal, int queueCount, bool isOverworld);

// Detailed 2D Top-Down Vehicle Model with headlights, brake lights, sirens, trailers, and EV badges
void DrawDetailedVehicle(Vector2 pos, float angleDeg, const vehicle<std::string>& v, bool isQueued);

// On-screen Vehicle & Traffic Signal Legend HUD card
void DrawVehicleAndSignalLegend(int screenWidth, int screenHeight);
