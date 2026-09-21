#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <list>
#include <cmath>
#include "raylib.h"
#include "raymath.h" 
#include "../core/Graph.h"
#include "../core/Header1.h"
#include "Graphics.h"
#include "../simulation/CityDesigner.h"

using namespace std;

#ifndef PI
#define PI 3.1415926535f
#endif

#ifndef CYAN
#define CYAN Color{ 0, 238, 238, 255 }
#endif

// --- HELPER: DRAW ARROWS ---
void DrawRoadArrowBold(Vector2 start, Vector2 end, Color col) {
    Vector2 dir = Vector2Normalize(Vector2Subtract(end, start));
    float length = Vector2Distance(start, end);
    if (length < 45.0f) return;

    Vector2 arrowPos = Vector2Subtract(end, Vector2Scale(dir, 24.0f));
    float arrowSize = 15.0f;
    Vector2 perp = { -dir.y, dir.x };

    Vector2 leftWing = Vector2Add(Vector2Subtract(arrowPos, Vector2Scale(dir, arrowSize)), Vector2Scale(perp, arrowSize * 0.75f));
    Vector2 rightWing = Vector2Subtract(Vector2Subtract(arrowPos, Vector2Scale(dir, arrowSize)), Vector2Scale(perp, arrowSize * 0.75f));

    DrawTriangle(arrowPos, leftWing, rightWing, col);
    DrawTriangleLines(arrowPos, leftWing, rightWing, Fade(RAYWHITE, 0.8f));
}

// --- HELPER: HEATMAP COLORS ---
Color GetMetroHeatColor(float rush) {
    if (rush < 0.15f) return GREEN;
    if (rush < 0.45f) return YELLOW;
    if (rush < 0.75f) return ORANGE;
    return RED;
}

// --- HELPER: DRAW SPARKLINE CHART ---
void DrawSparkline(const vector<float>& history, Rectangle bounds, Color lineCol, Color bgCol, const char* label, float maxVal) {
    DrawRectangleRec(bounds, bgCol);
    DrawRectangleLinesEx(bounds, 1.0f, Fade(RAYWHITE, 0.2f));

    DrawText(label, (int)bounds.x + 8, (int)bounds.y + 5, 11, Fade(RAYWHITE, 0.75f));

    int count = (int)history.size();
    if (count < 2) return;

    float stepX = bounds.width / (count - 1);
    float plotH = bounds.height - 20.0f;
    float bottomY = bounds.y + bounds.height - 4.0f;

    for (int i = 1; i < count; i++) {
        float val0 = Clamp(history[i - 1], 0.0f, maxVal);
        float val1 = Clamp(history[i], 0.0f, maxVal);

        float x0 = bounds.x + (i - 1) * stepX;
        float y0 = bottomY - (val0 / maxVal) * plotH;
        float x1 = bounds.x + i * stepX;
        float y1 = bottomY - (val1 / maxVal) * plotH;

        DrawLineEx({ x0, y0 }, { x1, y1 }, 2.0f, lineCol);
    }

    float curVal = history.back();
    DrawText(TextFormat("%.1f%%", curVal), (int)(bounds.x + bounds.width - 52), (int)bounds.y + 5, 11, lineCol);
}

// Fantasy City Theme Color Helper
Color GetFantasyCityColor(const string& name) {
    if (name == "Mount Olympus") return GOLD;
    if (name == "Tartarus")      return RED;
    if (name == "Atlantis")      return SKYBLUE;
    if (name == "Elysium")       return LIME;
    return RAYWHITE;
}

// =========================================================================
// 1. ADVANCED VISUAL TRAFFIC SIGNAL ENGINE (3-ASPECT HEAD, LENS GLOW & COUNTDOWN)
// =========================================================================
void DrawTrafficSignalHead(Vector2 postPos, Vector2 roadDir, const TrafficSignal& signal, int queueCount, bool isOverworld) {
    // 1. Transverse Stop Line across approach road
    float roadW = isOverworld ? 20.0f : 14.0f;
    Vector2 perp = { -roadDir.y, roadDir.x };
    Vector2 stopLineL = Vector2Subtract(postPos, Vector2Scale(perp, roadW * 0.6f));
    Vector2 stopLineR = Vector2Add(postPos, Vector2Scale(perp, roadW * 0.6f));
    DrawLineEx(stopLineL, stopLineR, 2.5f, Fade(RAYWHITE, 0.85f));

    // 2. Traffic Light Gantry & Housing Post (Offset to curb side)
    Vector2 headPos = Vector2Add(postPos, Vector2Scale(perp, roadW * 0.65f + 8.0f));

    // Signal Housing Dimensions (Sleek authentic 3-aspect casing)
    float houseW = isOverworld ? 14.0f : 12.0f;
    float houseH = isOverworld ? 34.0f : 30.0f;
    Rectangle houseRec = { headPos.x - houseW * 0.5f, headPos.y - houseH * 0.5f, houseW, houseH };

    // Housing base and metallic bezel border
    DrawRectangleRounded(houseRec, 0.4f, 4, { 18, 20, 26, 245 });
    DrawRectangleRoundedLines(houseRec, 0.4f, 4, { 85, 90, 100, 255 });

    // Lens centers
    float lensR = isOverworld ? 3.5f : 3.0f;
    float dy = isOverworld ? 9.5f : 8.0f;
    Vector2 redPos = { headPos.x, headPos.y - dy };
    Vector2 yelPos = { headPos.x, headPos.y };
    Vector2 grnPos = { headPos.x, headPos.y + dy };

    // Draw RED lens
    if (signal.isRed()) {
        DrawCircleV(redPos, lensR + 3.5f, Fade(RED, 0.45f));
        DrawCircleV(redPos, lensR, { 255, 30, 30, 255 });
        DrawCircleV({ redPos.x - 0.8f, redPos.y - 0.8f }, 1.2f, RAYWHITE);
    } else {
        DrawCircleV(redPos, lensR, { 55, 12, 12, 220 });
    }

    // Draw YELLOW lens
    if (signal.isYellow()) {
        DrawCircleV(yelPos, lensR + 3.5f, Fade(YELLOW, 0.50f));
        DrawCircleV(yelPos, lensR, { 255, 215, 0, 255 });
        DrawCircleV({ yelPos.x - 0.8f, yelPos.y - 0.8f }, 1.2f, RAYWHITE);
    } else {
        DrawCircleV(yelPos, lensR, { 55, 45, 8, 220 });
    }

    // Draw GREEN lens
    if (signal.isGreen()) {
        DrawCircleV(grnPos, lensR + 3.5f, Fade(GREEN, 0.45f));
        DrawCircleV(grnPos, lensR, { 0, 255, 110, 255 });
        DrawCircleV({ grnPos.x - 0.8f, grnPos.y - 0.8f }, 1.2f, RAYWHITE);
    } else {
        DrawCircleV(grnPos, lensR, { 8, 48, 18, 220 });
    }

    // 3. Real-Time Countdown Timer Display Pill
    int countdown = 0;
    Color timerCol = RED;
    if (signal.isGreen()) {
        countdown = (int)ceilf(signal.plannedGreenDuration - signal.greentimer);
        if (countdown < 0) countdown = 0;
        timerCol = LIME;
    } else if (signal.isYellow()) {
        countdown = (int)ceilf(signal.yellowDuration - signal.yellowtimer);
        if (countdown < 0) countdown = 0;
        timerCol = YELLOW;
    } else {
        countdown = (int)ceilf(signal.redtimer > 0.0f ? signal.redtimer : 6.0f);
        timerCol = RED;
    }

    Vector2 timerBadgePos = { headPos.x + houseW * 0.5f + 3.0f, headPos.y - 8.0f };
    DrawRectangleRounded({ timerBadgePos.x, timerBadgePos.y, 22.0f, 15.0f }, 0.35f, 3, Fade(BLACK, 0.85f));
    DrawRectangleRoundedLines({ timerBadgePos.x, timerBadgePos.y, 22.0f, 15.0f }, 0.35f, 3, timerCol);
    DrawText(TextFormat("%ds", countdown), (int)timerBadgePos.x + 3, (int)timerBadgePos.y + 2, 10, timerCol);

    // 4. Queued vehicles count badge
    if (queueCount > 0) {
        Vector2 qBadgePos = { headPos.x + houseW * 0.5f + 3.0f, headPos.y + 9.0f };
        DrawRectangleRounded({ qBadgePos.x, qBadgePos.y, 26.0f, 14.0f }, 0.35f, 3, Fade({ 40, 15, 15, 255 }, 0.90f));
        DrawRectangleRoundedLines({ qBadgePos.x, qBadgePos.y, 26.0f, 14.0f }, 0.35f, 3, ORANGE);
        DrawText(TextFormat("Q:%d", queueCount), (int)qBadgePos.x + 2, (int)qBadgePos.y + 2, 10, ORANGE);
    }

    // 5. Emergency Preemption Strobe Badge
    if (signal.emergencyPreempted) {
        bool flash = (fmodf((float)GetTime() * 10.0f, 2.0f) < 1.0f);
        if (flash) {
            Vector2 emBadge = { headPos.x - 22.0f, headPos.y - houseH * 0.5f - 14.0f };
            DrawRectangleRounded({ emBadge.x, emBadge.y, 44.0f, 13.0f }, 0.3f, 3, Fade(BLUE, 0.9f));
            DrawText("PREEMPT", (int)emBadge.x + 3, (int)emBadge.y + 2, 9, RAYWHITE);
        }
    }
}

// =========================================================================
// 2. DETAILED 2D TOP-DOWN VEHICLE RENDERING ENGINE
// =========================================================================
void DrawDetailedVehicle(Vector2 pos, float angleDeg, const vehicle<string>& v, bool isQueued) {
    float rad = angleDeg * (PI / 180.0f);
    Vector2 fwd = { cosf(rad), sinf(rad) };
    Vector2 right = { -sinf(rad), cosf(rad) };

    VehicleType type = v.type;

    if (type == VEHICLE_CAR) {
        // --- PASSENGER CAR (SEDAN) ---
        float length = 18.0f;
        float width = 9.0f;
        Color bodyColors[6] = {
            { 35, 130, 240, 255 },  // Cobalt Blue
            { 225, 45, 50, 255 },   // Crimson Red
            { 210, 215, 225, 255 }, // Metallic Silver
            { 0, 205, 205, 255 },   // Cyber Cyan
            { 240, 185, 35, 255 },  // Amber Gold
            { 45, 60, 120, 255 }    // Midnight Navy
        };
        Color bodyColor = bodyColors[abs(v.id) % 6];

        // Main chassis
        DrawRectanglePro({ pos.x, pos.y, length, width }, { length * 0.5f, width * 0.5f }, angleDeg, bodyColor);
        // Roof contour
        DrawRectanglePro({ pos.x - fwd.x * 1.0f, pos.y - fwd.y * 1.0f, 9.0f, 6.5f }, { 4.5f, 3.25f }, angleDeg, Fade(BLACK, 0.25f));
        // Windshield (front)
        DrawRectanglePro({ pos.x + fwd.x * 3.0f, pos.y + fwd.y * 3.0f, 3.5f, 6.0f }, { 1.75f, 3.0f }, angleDeg, { 25, 35, 45, 255 });
        // Rear window
        DrawRectanglePro({ pos.x - fwd.x * 4.5f, pos.y - fwd.y * 4.5f, 2.8f, 5.5f }, { 1.4f, 2.75f }, angleDeg, { 25, 35, 45, 255 });

        // Headlights (Twin white/amber beams)
        Vector2 hlL = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, 8.5f), Vector2Scale(right, -3.0f)));
        Vector2 hlR = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, 8.5f), Vector2Scale(right, 3.0f)));
        DrawCircleV(hlL, 1.6f, RAYWHITE);
        DrawCircleV(hlR, 1.6f, RAYWHITE);

        // Taillights / Brake lights
        Vector2 tlL = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, -8.5f), Vector2Scale(right, -3.0f)));
        Vector2 tlR = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, -8.5f), Vector2Scale(right, 3.0f)));
        if (isQueued) {
            DrawCircleV(tlL, 2.5f, RED);
            DrawCircleV(tlR, 2.5f, RED);
            DrawCircleLines((int)tlL.x, (int)tlL.y, 4.0f, Fade(RED, 0.6f));
            DrawCircleLines((int)tlR.x, (int)tlR.y, 4.0f, Fade(RED, 0.6f));
        } else {
            DrawCircleV(tlL, 1.5f, { 200, 20, 20, 255 });
            DrawCircleV(tlR, 1.5f, { 200, 20, 20, 255 });
        }
    }
    else if (type == VEHICLE_TRUCK) {
        // --- HEAVY FREIGHT TRUCK (18-WHEELER / CONTAINER) ---
        float width = 12.0f;

        // Container trailer (rear)
        Vector2 trailerCenter = Vector2Subtract(pos, Vector2Scale(fwd, 4.5f));
        Color containerCol = ((v.id % 2 == 0) ? Color{ 40, 110, 180, 255 } : Color{ 55, 140, 95, 255 });
        DrawRectanglePro({ trailerCenter.x, trailerCenter.y, 18.0f, width }, { 9.0f, width * 0.5f }, angleDeg, containerCol);
        // Container rib markings
        for (float rib = -6.0f; rib <= 6.0f; rib += 4.0f) {
            Vector2 ribPos = Vector2Add(trailerCenter, Vector2Scale(fwd, rib));
            Vector2 rL = Vector2Subtract(ribPos, Vector2Scale(right, width * 0.45f));
            Vector2 rR = Vector2Add(ribPos, Vector2Scale(right, width * 0.45f));
            DrawLineEx(rL, rR, 1.0f, Fade(BLACK, 0.35f));
        }

        // Driver Cab (front)
        Vector2 cabCenter = Vector2Add(pos, Vector2Scale(fwd, 9.5f));
        DrawRectanglePro({ cabCenter.x, cabCenter.y, 9.0f, width - 1.0f }, { 4.5f, (width - 1.0f) * 0.5f }, angleDeg, { 50, 65, 80, 255 });
        // Windshield
        DrawRectanglePro({ cabCenter.x + fwd.x * 2.0f, cabCenter.y + fwd.y * 2.0f, 3.2f, 8.0f }, { 1.6f, 4.0f }, angleDeg, { 20, 30, 40, 255 });

        // Amber roof clearance lights
        Vector2 clL = Vector2Add(cabCenter, Vector2Add(Vector2Scale(fwd, 4.0f), Vector2Scale(right, -4.0f)));
        Vector2 clR = Vector2Add(cabCenter, Vector2Add(Vector2Scale(fwd, 4.0f), Vector2Scale(right, 4.0f)));
        DrawCircleV(clL, 1.4f, GOLD);
        DrawCircleV(clR, 1.4f, GOLD);

        // Headlights
        Vector2 hlL = Vector2Add(cabCenter, Vector2Add(Vector2Scale(fwd, 4.2f), Vector2Scale(right, -3.8f)));
        Vector2 hlR = Vector2Add(cabCenter, Vector2Add(Vector2Scale(fwd, 4.2f), Vector2Scale(right, 3.8f)));
        DrawCircleV(hlL, 1.7f, RAYWHITE);
        DrawCircleV(hlR, 1.7f, RAYWHITE);

        // Taillights
        Vector2 tlL = Vector2Subtract(trailerCenter, Vector2Add(Vector2Scale(fwd, 9.0f), Vector2Scale(right, 4.2f)));
        Vector2 tlR = Vector2Subtract(trailerCenter, Vector2Subtract(Vector2Scale(fwd, 9.0f), Vector2Scale(right, 4.2f)));
        if (isQueued) {
            DrawCircleV(tlL, 2.8f, RED);
            DrawCircleV(tlR, 2.8f, RED);
        } else {
            DrawCircleV(tlL, 1.8f, { 210, 25, 25, 255 });
            DrawCircleV(tlR, 1.8f, { 210, 25, 25, 255 });
        }
    }
    else if (type == VEHICLE_BUS) {
        // --- METRO TRANSIT COACH ---
        float length = 26.0f;
        float width = 11.0f;
        Color busColor = { 245, 185, 20, 255 }; // High-vis Transit Gold

        DrawRectanglePro({ pos.x, pos.y, length, width }, { length * 0.5f, width * 0.5f }, angleDeg, busColor);
        // White climate roof pod
        DrawRectanglePro({ pos.x - fwd.x * 1.5f, pos.y - fwd.y * 1.5f, 15.0f, 7.0f }, { 7.5f, 3.5f }, angleDeg, { 240, 245, 250, 255 });
        // Side passenger window strip (left & right)
        DrawRectanglePro({ pos.x - fwd.x * 0.5f, pos.y - fwd.y * 0.5f, 18.0f, 2.2f }, { 9.0f, 1.1f }, angleDeg, { 20, 30, 45, 255 });
        // Front destination visor
        DrawRectanglePro({ pos.x + fwd.x * 10.5f, pos.y + fwd.y * 10.5f, 2.5f, 8.0f }, { 1.25f, 4.0f }, angleDeg, { 15, 20, 30, 255 });
        // "BUS" sign glow
        DrawCircleV(Vector2Add(pos, Vector2Scale(fwd, 11.0f)), 1.5f, SKYBLUE);

        // Headlights & Taillights
        Vector2 hlL = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, 12.5f), Vector2Scale(right, -3.8f)));
        Vector2 hlR = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, 12.5f), Vector2Scale(right, 3.8f)));
        DrawCircleV(hlL, 1.7f, RAYWHITE);
        DrawCircleV(hlR, 1.7f, RAYWHITE);

        Vector2 tlL = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, -12.5f), Vector2Scale(right, -3.8f)));
        Vector2 tlR = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, -12.5f), Vector2Scale(right, 3.8f)));
        DrawCircleV(tlL, isQueued ? 2.6f : 1.7f, RED);
        DrawCircleV(tlR, isQueued ? 2.6f : 1.7f, RED);
    }
    else if (type == VEHICLE_EMERGENCY) {
        // --- PRIORITY AMBULANCE / EMERGENCY RESCUE ---
        float length = 22.0f;
        float width = 11.0f;

        // Pure white ambulance body with red trim
        DrawRectanglePro({ pos.x, pos.y, length, width }, { length * 0.5f, width * 0.5f }, angleDeg, { 250, 252, 255, 255 });
        DrawRectanglePro({ pos.x - fwd.x * 2.0f, pos.y - fwd.y * 2.0f, 12.0f, width }, { 6.0f, width * 0.5f }, angleDeg, Fade(RED, 0.25f));

        // Bold Red Cross (+) on roof
        DrawRectanglePro({ pos.x - fwd.x * 2.0f, pos.y - fwd.y * 2.0f, 6.0f, 2.2f }, { 3.0f, 1.1f }, angleDeg, RED);
        DrawRectanglePro({ pos.x - fwd.x * 2.0f, pos.y - fwd.y * 2.0f, 2.2f, 6.0f }, { 1.1f, 3.0f }, angleDeg, RED);

        // Active Emergency Strobe Lightbar on cab roof
        Vector2 barPos = Vector2Add(pos, Vector2Scale(fwd, 4.5f));
        bool strobe = (fmodf((float)GetTime() * 12.0f, 2.0f) < 1.0f);
        Color colL = strobe ? Color{ 255, 30, 30, 255 } : Color{ 30, 100, 255, 255 };
        Color colR = strobe ? Color{ 30, 100, 255, 255 } : Color{ 255, 30, 30, 255 };
        Vector2 bL = Vector2Subtract(barPos, Vector2Scale(right, 3.0f));
        Vector2 bR = Vector2Add(barPos, Vector2Scale(right, 3.0f));
        DrawCircleV(bL, 2.4f, colL);
        DrawCircleV(bR, 2.4f, colR);
        DrawCircleLines((int)bL.x, (int)bL.y, 4.0f, Fade(colL, 0.7f));
        DrawCircleLines((int)bR.x, (int)bR.y, 4.0f, Fade(colR, 0.7f));

        // Translucent acoustic siren wave pulse
        float sirenPulse = fmodf((float)GetTime() * 32.0f, 28.0f) + 8.0f;
        DrawCircleLines((int)pos.x, (int)pos.y, sirenPulse, Fade(RED, 0.45f * (1.0f - sirenPulse / 36.0f)));
        DrawCircleLines((int)pos.x, (int)pos.y, sirenPulse * 0.65f, Fade(SKYBLUE, 0.40f * (1.0f - sirenPulse / 36.0f)));

        // High-beam flashing headlights
        Vector2 hlL = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, 10.5f), Vector2Scale(right, -3.5f)));
        Vector2 hlR = Vector2Add(pos, Vector2Add(Vector2Scale(fwd, 10.5f), Vector2Scale(right, 3.5f)));
        DrawCircleV(hlL, 2.2f, RAYWHITE);
        DrawCircleV(hlR, 2.2f, RAYWHITE);
    }
    else if (type == VEHICLE_EV) {
        // --- ELECTRIC VEHICLE (EV) ---
        float length = 18.0f;
        float width = 9.5f;
        Color evBody = { 30, 220, 160, 255 }; // Electric Mint Turquoise

        DrawRectanglePro({ pos.x, pos.y, length, width }, { length * 0.5f, width * 0.5f }, angleDeg, evBody);
        // Panoramic tinted glass roof
        DrawRectanglePro({ pos.x - fwd.x * 0.5f, pos.y - fwd.y * 0.5f, 10.0f, 7.0f }, { 5.0f, 3.5f }, angleDeg, { 20, 45, 55, 255 });

        // Neon EV Lightning / Battery Badge (Center roof)
        DrawCircleV(pos, 2.2f, CYAN);
        DrawCircleLines((int)pos.x, (int)pos.y, 3.5f, Fade(CYAN, 0.75f));

        // Front LED Lightstrip & Rear LED Bar
        Vector2 fStrip = Vector2Add(pos, Vector2Scale(fwd, 8.5f));
        Vector2 fL = Vector2Subtract(fStrip, Vector2Scale(right, 3.2f));
        Vector2 fR = Vector2Add(fStrip, Vector2Scale(right, 3.2f));
        DrawLineEx(fL, fR, 1.8f, CYAN);

        Vector2 rStrip = Vector2Subtract(pos, Vector2Scale(fwd, 8.5f));
        Vector2 rL = Vector2Subtract(rStrip, Vector2Scale(right, 3.2f));
        Vector2 rR = Vector2Add(rStrip, Vector2Scale(right, 3.2f));
        DrawLineEx(rL, rR, isQueued ? 2.5f : 1.5f, isQueued ? RED : Fade(RED, 0.75f));

        // Low Battery indicator if applicable
        if (v.isLowBattery()) {
            float warnPulse = 1.0f + 0.4f * sinf((float)GetTime() * 8.0f);
            DrawCircleV(Vector2Subtract(pos, Vector2Scale(right, 8.0f)), 3.0f * warnPulse, ORANGE);
        }
    }
    else if (type == VEHICLE_MOTORCYCLE) {
        // --- MOTORCYCLE / SCOOTER ---
        float length = 12.0f;
        float width = 4.5f;
        Color bikeColor = (v.id % 2 == 0) ? Color{ 230, 40, 40, 255 } : Color{ 240, 180, 20, 255 };

        // Chassis
        DrawRectanglePro({ pos.x, pos.y, length, width }, { length * 0.5f, width * 0.5f }, angleDeg, { 35, 40, 45, 255 });
        // Fairing / Fuel tank
        DrawRectanglePro({ pos.x + fwd.x * 1.5f, pos.y + fwd.y * 1.5f, 5.0f, 3.8f }, { 2.5f, 1.9f }, angleDeg, bikeColor);
        // Rider helmet dot
        DrawCircleV(Vector2Subtract(pos, Vector2Scale(fwd, 1.0f)), 2.2f, RAYWHITE);
        DrawCircleLines((int)(pos.x - fwd.x * 1.0f), (int)(pos.y - fwd.y * 1.0f), 2.2f, BLACK);

        // Single front headlight
        Vector2 hl = Vector2Add(pos, Vector2Scale(fwd, 6.0f));
        DrawCircleV(hl, 1.8f, RAYWHITE);
        // Single rear brake light
        Vector2 tl = Vector2Subtract(pos, Vector2Scale(fwd, 6.0f));
        DrawCircleV(tl, isQueued ? 2.5f : 1.5f, RED);
    }
}

// =========================================================================
// 3. VEHICLE & TRAFFIC SIGNAL ON-SCREEN LEGEND HUD
// =========================================================================
void DrawVehicleAndSignalLegend(int screenWidth, int screenHeight) {
    int legW = 345;
    int legH = 205;
    int legX = screenWidth - 370;
    int legY = screenHeight - 225;

    DrawRectangle(legX, legY, legW, legH, Fade({ 12, 16, 26, 255 }, 0.94f));
    DrawRectangleLines(legX, legY, legW, legH, Fade(GOLD, 0.45f));

    DrawText("VEHICLE & SIGNAL LEGEND", legX + 15, legY + 10, 14, GOLD);
    DrawLine(legX + 15, legY + 28, legX + legW - 15, legY + 28, Fade(DARKGRAY, 0.5f));

    // Vehicle Rows
    // 1. Car
    DrawRectangle(legX + 18, legY + 36, 14, 8, { 35, 130, 240, 255 });
    DrawText("Car / Sedan (Private Commuter)", legX + 42, legY + 34, 12, RAYWHITE);

    // 2. Freight Truck
    DrawRectangle(legX + 15, legY + 54, 20, 9, { 40, 110, 180, 255 });
    DrawText("Freight Truck (18-Wheeler, 2.5 PCU)", legX + 42, legY + 52, 12, SKYBLUE);

    // 3. Transit Bus
    DrawRectangle(legX + 16, legY + 72, 18, 9, { 245, 185, 20, 255 });
    DrawText("Transit Bus (Public Coach)", legX + 42, legY + 70, 12, YELLOW);

    // 4. Emergency Ambulance
    DrawRectangle(legX + 16, legY + 90, 17, 9, RAYWHITE);
    DrawRectangle(legX + 22, legY + 91, 5, 7, RED);
    DrawText("Ambulance (Siren + Green Preempt)", legX + 42, legY + 88, 12, RED);

    // 5. Electric Vehicle (EV)
    DrawRectangle(legX + 18, legY + 108, 14, 8, { 30, 220, 160, 255 });
    DrawCircle(legX + 25, legY + 112, 2.0f, CYAN);
    DrawText("EV (Zero Emission, Battery Telemetry)", legX + 42, legY + 106, 12, LIME);

    // 6. Motorcycle
    DrawRectangle(legX + 20, legY + 126, 10, 4, ORANGE);
    DrawText("Motorcycle (Agile 2-Wheeler, 0.5 PCU)", legX + 42, legY + 122, 12, Fade(RAYWHITE, 0.9f));

    DrawLine(legX + 15, legY + 140, legX + legW - 15, legY + 140, Fade(DARKGRAY, 0.4f));

    // 7. Traffic Signals
    DrawCircle(legX + 20, legY + 158, 4.0f, RED);
    DrawCircle(legX + 32, legY + 158, 4.0f, YELLOW);
    DrawCircle(legX + 44, legY + 158, 4.0f, GREEN);
    DrawText("3-Aspect Signals: Red / Yellow / Green", legX + 58, legY + 150, 12, RAYWHITE);
    DrawText("Real-Time Countdown Timer & Queue Q:N", legX + 58, legY + 168, 11, Fade(RAYWHITE, 0.75f));
    DrawText("White Stop Line at Intersection Approach", legX + 58, legY + 184, 10, Fade(RAYWHITE, 0.60f));
}

enum AppViewState {
    VIEW_MAIN_MENU = 0,       // Interactive landing portal: Go See City vs Run Optimization Model
    VIEW_CITY_DESIGNER = 1,   // Dedicated interactive settings page to design city parameters
    VIEW_NYC_METRO_CITY = 2,  // NYC Metro-style live simulation of the configured city
    VIEW_REALM_OVERWORLD = 3, // Inter-realm overworld transit view
    VIEW_CITY_METROPOLIS = 4, // Multi-district fantasy view
    VIEW_DATA_OPTIMIZER = 5   // Dedicated data-driven optimization studio
};

int main() {
    // 0. Initialize NYC Metro City Designer & Simulation Engine
    CityDesignParams nycParams;
    Simulator<string, 100> nycSim;
    map<string, Vector2> nycPositions;
    map<string, Vector3> nycPositions3D;

    // 1. Initialize Fantasy Realm Overworld Simulator (4 Mythical Cities)
    Simulator<string, 100> realmSim;
    realmSim.clear();
    realmSim.setupFantasyRealmsNetwork();
    realmSim.addFantasyRealmCaravans(40);

    // 2. Initialize 4 Individual Fantasy City Metropolises
    string fantasyCityNames[4] = { "Mount Olympus", "Tartarus", "Atlantis", "Elysium" };
    Simulator<string, 100> citySims[4];
    for (int k = 0; k < 4; k++) {
        citySims[k].clear();
        citySims[k].setupCityLevelNetwork(fantasyCityNames[k]);
        citySims[k].addCityCommuterTraffic(35);
    }

    // --- FULL SCREEN INITIALIZATION ---
    InitWindow(0, 0, "NYC METRO CITY DESIGNER & URBAN TRAFFIC PLANNING SIMULATOR");

    int screenWidth = GetMonitorWidth(0);
    int screenHeight = GetMonitorHeight(0);

    SetWindowSize(screenWidth, screenHeight);
    ToggleFullscreen();
    SetTargetFPS(60);

    float centerX = 400.0f + (screenWidth - 400.0f) / 2.0f;
    float centerY = screenHeight / 2.0f;

    // Build default NYC Metro network with 3D positions
    SetupNYCMetroCustomCity(nycSim, nycParams, nycPositions, nycPositions3D, screenWidth, screenHeight);

    // View State: Start in the Main Menu Hub!
    AppViewState currentView = VIEW_MAIN_MENU;
    int selectedModelTab = 0; // 0=Genetic Algorithm, 1=Webster, 2=PyTorch ResNet
    int selectedCityIdx = 0; // 0=Olympus, 1=Tartarus, 2=Atlantis, 3=Elysium

    // Compute Position Layouts
    map<string, Vector2> realmPositions = ComputeFantasyRealmPositions(screenWidth, screenHeight);
    map<string, Vector2> cityPositions[4];
    for (int k = 0; k < 4; k++) {
        cityPositions[k] = ComputeFantasyCityPositions(screenWidth, screenHeight, fantasyCityNames[k]);
    }

    // Dynamic Interpolated positions
    map<string, Vector2> currentPositions = nycPositions;

    // 2D Camera (for Fantasy Overworld & Metropolis)
    Camera2D camera = { 0 };
    camera.target = { centerX, centerY };
    camera.offset = { centerX, centerY };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    // 3D Perspective Camera (for NYC Metro 3D World)
    Camera3D mainCam3D = { 0 };
    mainCam3D.position = Vector3{ 0.0f, 22.0f, 32.0f };
    mainCam3D.target = Vector3{ 0.0f, 0.5f, 0.0f };
    mainCam3D.up = Vector3{ 0.0f, 1.0f, 0.0f };
    mainCam3D.fovy = 45.0f;
    mainCam3D.projection = CAMERA_PERSPECTIVE;

    float camOrbitAngle = 0.85f;
    float camPitch = 0.80f;
    float camDistance = 34.0f;
    Vector3 camTarget = { 0.0f, 0.5f, 0.0f };
    bool autoOrbit = false;
    int chaseCarId = -1;
    NYCCameraControlActions camActions;

    // Simulation playback state
    int total_time = 0;
    bool simulationFinished = false;
    bool isPaused = false;
    int simSpeed = 1;
    bool isRoadBlocked = false;
    int emergCarID = 90000;
    vector<float> rushHistory;

    // City Planning Helper & Optimization State
    bool isGAOptimizedActive = true;
    int activeStrategyIdx = 1; // 0=Webster, 1=GA, 2=Actuated, 3=Coordinated
    int trackedCarId = -1;

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();
        bool inViewport = (mousePos.x > 400.0f);
        Vector2 mouseWorld = GetScreenToWorld2D(mousePos, camera);

        // =====================================================================
        // VIEW_MAIN_MENU: HUB PORTAL (GO SEE THE CITY vs RUN OPTIMIZATION MODEL)
        // =====================================================================
        if (currentView == VIEW_MAIN_MENU) {
            BeginDrawing();
            int totalActiveVehicles = nycSim.cityManager ? nycSim.cityManager->getVehicleCount() : 35;
            float avgDelay = (nycSim.cityReport.optimizedAvgDelay > 0.0f) ? nycSim.cityReport.optimizedAvgDelay : 16.8f;
            float congestion = nycSim.rush();
            MainMenuAction menuAction = DrawMainMenuHub(screenWidth, screenHeight, totalActiveVehicles, avgDelay, congestion);
            EndDrawing();

            if (menuAction == MENU_ACTION_SEE_CITY) {
                currentPositions = nycPositions;
                currentView = VIEW_NYC_METRO_CITY;
            } else if (menuAction == MENU_ACTION_RUN_OPTIMIZER) {
                currentView = VIEW_DATA_OPTIMIZER;
            } else if (menuAction == MENU_ACTION_EVAL_ML) {
                selectedModelTab = 2;
                currentView = VIEW_DATA_OPTIMIZER;
            } else if (menuAction == MENU_ACTION_CONFIG_CITY) {
                currentView = VIEW_CITY_DESIGNER;
            } else if (menuAction == MENU_ACTION_REALM_OVERWORLD) {
                currentPositions = realmPositions;
                currentView = VIEW_REALM_OVERWORLD;
            }
            continue;
        }

        // =====================================================================
        // VIEW_DATA_OPTIMIZER: DATA-DRIVEN MODEL RUNNER STUDIO
        // =====================================================================
        if (currentView == VIEW_DATA_OPTIMIZER) {
            BeginDrawing();
            bool applyAndSeeCity = false;
            bool returnToMenu = DrawDataOptimizerInterface(nycSim, nycParams, total_time, screenWidth, screenHeight, selectedModelTab, applyAndSeeCity);
            EndDrawing();

            if (applyAndSeeCity) {
                isGAOptimizedActive = true;
                currentPositions = nycPositions;
                currentView = VIEW_NYC_METRO_CITY;
            } else if (returnToMenu || IsKeyPressed(KEY_ESCAPE)) {
                currentView = VIEW_MAIN_MENU;
            }
            continue;
        }

        // Dedicated City Designer Settings Screen
        if (currentView == VIEW_CITY_DESIGNER) {
            BeginDrawing();
            bool triggerBuild = DrawCityDesignerSettingsPage(nycParams, screenWidth, screenHeight);
            EndDrawing();

            if (triggerBuild || IsKeyPressed(KEY_ENTER)) {
                SetupNYCMetroCustomCity(nycSim, nycParams, nycPositions, nycPositions3D, screenWidth, screenHeight);
                currentPositions = nycPositions;
                currentView = VIEW_NYC_METRO_CITY;
                camera.target = { centerX, centerY };
                camera.offset = { centerX, centerY };
                camera.zoom = 1.0f;
                total_time = 0;
                simulationFinished = false;
                isPaused = false;
                trackedCarId = -1;
            } else if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_F1)) {
                currentPositions = nycPositions;
                currentView = VIEW_MAIN_MENU;
            }
            continue;
        }

        // Active Simulator reference
        Simulator<string, 100>& activeSim = (currentView == VIEW_NYC_METRO_CITY) ? nycSim :
                                            ((currentView == VIEW_REALM_OVERWORLD) ? realmSim : citySims[selectedCityIdx]);
        Graph<string, 100>* mapRef = activeSim.getMap();
        int totalNodes = mapRef->Vcount;

        // Target positions for current view
        const map<string, Vector2>& targetPositions = (currentView == VIEW_NYC_METRO_CITY) ? nycPositions :
                                                      ((currentView == VIEW_REALM_OVERWORLD) ? realmPositions : cityPositions[selectedCityIdx]);

        // Navigation buttons in City View
        Rectangle btnMenuRec   = { 420.0f, 16.0f, 130.0f, 40.0f };
        Rectangle btnOptRec    = { 560.0f, 16.0f, 210.0f, 40.0f };
        Rectangle nycConfigBtnRec = { 780.0f, 16.0f, 150.0f, 40.0f };
        Rectangle backBtnRec   = { 420.0f, 16.0f, 290.0f, 40.0f };

        // =====================================================================
        // 1. INTERACTIVE KEYBOARD & MOUSE CONTROLS
        // =====================================================================
        // NYC Metro Navigation: Return to Main Menu [M], Optimizer [O], Config [F1]
        if (currentView == VIEW_NYC_METRO_CITY) {
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE) ||
               (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, btnMenuRec))) {
                currentView = VIEW_MAIN_MENU;
            }
            if (IsKeyPressed(KEY_O) || IsKeyPressed(KEY_D) ||
               (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, btnOptRec))) {
                currentView = VIEW_DATA_OPTIMIZER;
            }
            if (IsKeyPressed(KEY_F1) ||
               (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, nycConfigBtnRec))) {
                currentView = VIEW_CITY_DESIGNER;
            }
            if (IsKeyPressed(KEY_K)) {
                ExportCityPlanningMetrics(nycSim, nycParams, total_time);
            }
        }

        if (currentView == VIEW_REALM_OVERWORLD) {
            if (IsKeyPressed(KEY_M) || IsKeyPressed(KEY_ESCAPE)) {
                currentView = VIEW_MAIN_MENU;
            }
        }

        // View Navigation: ESC / Backspace / Back Button to return to Realm Overworld
        if (currentView == VIEW_CITY_METROPOLIS) {
            if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_BACKSPACE) || 
               (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(mousePos, backBtnRec))) {
                currentView = VIEW_REALM_OVERWORLD;
                currentPositions = realmPositions;
                camera.target = { centerX, centerY };
                camera.offset = { centerX, centerY };
                camera.zoom = 1.0f;
                trackedCarId = -1;
            }
        }

        // Quick NYC Planning Presets (Keys 1-3)
        if (IsKeyPressed(KEY_ONE)) {
            nycParams.loadPreset(0); // Manhattan Midtown
            SetupNYCMetroCustomCity(nycSim, nycParams, nycPositions, nycPositions3D, screenWidth, screenHeight);
            currentPositions = nycPositions;
            currentView = VIEW_NYC_METRO_CITY;
            camera.target = { centerX, centerY };
            camera.zoom = 1.0f;
            total_time = 0;
            simulationFinished = false;
            trackedCarId = -1;
        }
        if (IsKeyPressed(KEY_TWO)) {
            nycParams.loadPreset(1); // Broadway Diagonal Express
            SetupNYCMetroCustomCity(nycSim, nycParams, nycPositions, nycPositions3D, screenWidth, screenHeight);
            currentPositions = nycPositions;
            currentView = VIEW_NYC_METRO_CITY;
            camera.target = { centerX, centerY };
            camera.zoom = 1.0f;
            total_time = 0;
            simulationFinished = false;
            trackedCarId = -1;
        }
        if (IsKeyPressed(KEY_THREE)) {
            nycParams.loadPreset(2); // Crosstown Transit Hub
            SetupNYCMetroCustomCity(nycSim, nycParams, nycPositions, nycPositions3D, screenWidth, screenHeight);
            currentPositions = nycPositions;
            currentView = VIEW_NYC_METRO_CITY;
            camera.target = { centerX, centerY };
            camera.zoom = 1.0f;
            total_time = 0;
            simulationFinished = false;
            trackedCarId = -1;
        }

        // Playback: Pause / Resume toggle
        if (IsKeyPressed(KEY_SPACE)) {
            isPaused = !isPaused;
        }

        // Playback: Single-step advance
        bool stepSingleTick = false;
        if (IsKeyPressed(KEY_N)) {
            isPaused = true;
            stepSingleTick = true;
        }

        // Speed Multipliers: [Z]=1x, [X]=2x, [C]=5x
        if (IsKeyPressed(KEY_Z)) simSpeed = 1;
        if (IsKeyPressed(KEY_X)) simSpeed = 2;
        if (IsKeyPressed(KEY_C)) simSpeed = 5;

        // City Planning Helper: Run Genetic Algorithm Signal Optimizer Live (Key O)
        if (IsKeyPressed(KEY_O)) {
            isGAOptimizedActive = !isGAOptimizedActive;
            activeSim.optimizeCitySignals(isGAOptimizedActive);
            activeStrategyIdx = isGAOptimizedActive ? 1 : 0;
        }

        // City Planning Helper: Cycle Signal Strategy (Key S)
        if (IsKeyPressed(KEY_S)) {
            activeStrategyIdx = (activeStrategyIdx + 1) % 4;
            auto* nodes = mapRef->getNodes();
            for (int i = 0; i < mapRef->Vcount; i++) {
                for (auto& edge : nodes[i].Neighbors) {
                    edge.weight.light.strategy = static_cast<SignalControlStrategy>(activeStrategyIdx);
                }
            }
        }

        // 3D & 2D Camera Navigation & Viewport Controls
        if (currentView == VIEW_NYC_METRO_CITY) {
            // Process camera actions from sidebar
            if (camActions.setIsoView) {
                camOrbitAngle = 0.85f;
                camPitch = 0.80f;
                camDistance = 34.0f;
                camTarget = Vector3{ 0.0f, 0.5f, 0.0f };
                autoOrbit = false;
                chaseCarId = -1;
                camActions.setIsoView = false;
            }
            if (camActions.setTopDownView) {
                camPitch = 1.48f;
                camDistance = 44.0f;
                autoOrbit = false;
                camActions.setTopDownView = false;
            }
            if (camActions.setStreetView) {
                camPitch = 0.22f;
                camDistance = 16.0f;
                autoOrbit = false;
                camActions.setStreetView = false;
            }
            if (camActions.toggleAutoOrbit) {
                autoOrbit = !autoOrbit;
                camActions.toggleAutoOrbit = false;
            }
            if (camActions.chaseAmbulance) {
                for (const auto& v : activeSim.cityManager->getVehicles()) {
                    if (v.isEmergency()) {
                        chaseCarId = v.id;
                        trackedCarId = v.id;
                        break;
                    }
                }
                camActions.chaseAmbulance = false;
            }

            // Process traffic dispatch actions from sidebar
            if (camActions.injectCar) {
                static int customCarSeq = 70000;
                if (mapRef->Vcount >= 2) {
                    int u = rand() % mapRef->Vcount;
                    int v = (u + 1 + rand() % (mapRef->Vcount - 1)) % mapRef->Vcount;
                    activeSim.cityManager->addVehicle(customCarSeq++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), VEHICLE_CAR);
                }
                camActions.injectCar = false;
            }
            if (camActions.injectFleet) {
                static int customFleetSeq = 75000;
                for (int k = 0; k < 5 && mapRef->Vcount >= 2; k++) {
                    int u = rand() % mapRef->Vcount;
                    int v = (u + 1 + rand() % (mapRef->Vcount - 1)) % mapRef->Vcount;
                    VehicleType vt = (VehicleType)(rand() % 6);
                    activeSim.cityManager->addVehicle(customFleetSeq++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), vt);
                }
                camActions.injectFleet = false;
            }
            if (camActions.injectAmbulance) {
                if (mapRef->Vcount >= 2) {
                    int u = nycParams.emergencyHubIdx % totalNodes;
                    int v = (u + 1 + rand() % (totalNodes - 1)) % totalNodes;
                    activeSim.cityManager->addVehicle(emergCarID++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), VEHICLE_EMERGENCY);
                }
                camActions.injectAmbulance = false;
            }
            if (camActions.injectBus) {
                static int busSeq = 80000;
                if (mapRef->Vcount >= 2) {
                    int u = rand() % mapRef->Vcount;
                    int v = (u + 1 + rand() % (mapRef->Vcount - 1)) % mapRef->Vcount;
                    VehicleType vt = (rand() % 2 == 0) ? VEHICLE_BUS : VEHICLE_TRUCK;
                    activeSim.cityManager->addVehicle(busSeq++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), vt);
                }
                camActions.injectBus = false;
            }
            if (camActions.injectEV) {
                static int evSeq = 85000;
                if (mapRef->Vcount >= 2) {
                    int u = rand() % mapRef->Vcount;
                    int v = (u + 1 + rand() % (mapRef->Vcount - 1)) % mapRef->Vcount;
                    VehicleType vt = (rand() % 2 == 0) ? VEHICLE_EV : VEHICLE_MOTORCYCLE;
                    activeSim.cityManager->addVehicle(evSeq++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), vt);
                }
                camActions.injectEV = false;
            }
            if (camActions.injectPeakSurge) {
                activeSim.cityManager->injectPeakDemand(total_time, true);
                camActions.injectPeakSurge = false;
            }
            if (camActions.toggleRoadBlock) {
                isRoadBlocked = !isRoadBlocked;
                string u = mapRef->getVertexAt(0);
                string v = mapRef->getVertexAt(1);
                if (isRoadBlocked) activeSim.blockRoad(u, v);
                else               activeSim.unblockRoad(u, v);
                camActions.toggleRoadBlock = false;
            }

            // Keyboard Shortcuts for 3D Camera
            if (IsKeyPressed(KEY_C)) {
                camOrbitAngle = 0.85f;
                camPitch = 0.80f;
                camDistance = 34.0f;
                camTarget = Vector3{ 0.0f, 0.5f, 0.0f };
                autoOrbit = false;
                chaseCarId = -1;
            }
            if (IsKeyPressed(KEY_R)) {
                autoOrbit = !autoOrbit;
            }
            if (IsKeyPressed(KEY_T)) {
                if (trackedCarId != -1) {
                    chaseCarId = (chaseCarId == trackedCarId) ? -1 : trackedCarId;
                } else {
                    for (const auto& v : activeSim.cityManager->getVehicles()) {
                        if (v.isEmergency()) {
                            chaseCarId = v.id;
                            trackedCarId = v.id;
                            break;
                        }
                    }
                }
            }

            // Mouse Controls in 3D Viewport
            if (inViewport) {
                // Right Mouse Drag: Orbit & Tilt
                if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && !IsKeyDown(KEY_LEFT_SHIFT)) {
                    Vector2 delta = GetMouseDelta();
                    camOrbitAngle += delta.x * 0.006f;
                    camPitch = Clamp(camPitch + delta.y * 0.006f, 0.12f, 1.50f);
                    autoOrbit = false;
                }

                // Middle Mouse Drag (or Shift + Right Drag): Pan Camera Target
                if (IsMouseButtonDown(MOUSE_BUTTON_MIDDLE) || (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && IsKeyDown(KEY_LEFT_SHIFT))) {
                    Vector2 delta = GetMouseDelta();
                    Vector3 forward = Vector3Normalize(Vector3{ sinf(camOrbitAngle), 0.0f, cosf(camOrbitAngle) });
                    Vector3 right = Vector3Normalize(Vector3{ cosf(camOrbitAngle), 0.0f, -sinf(camOrbitAngle) });
                    camTarget = Vector3Add(camTarget, Vector3Scale(right, -delta.x * 0.035f));
                    camTarget = Vector3Add(camTarget, Vector3Scale(forward, -delta.y * 0.035f));
                    chaseCarId = -1;
                }

                // Mouse Wheel: Zoom
                float wheel = GetMouseWheelMove();
                if (wheel != 0.0f) {
                    camDistance = Clamp(camDistance - wheel * 2.5f, 6.0f, 85.0f);
                }
            }

            // Auto Orbit Update
            if (autoOrbit) {
                camOrbitAngle += GetFrameTime() * 0.18f;
            }

            // Chase Camera Follow Update
            if (chaseCarId != -1) {
                for (const auto& v : activeSim.cityManager->getVehicles()) {
                    if (v.id == chaseCarId && v.state == 1) {
                        string src = v.currentRoad.first;
                        string dst = v.currentRoad.second;
                        if (nycPositions3D.find(src) != nycPositions3D.end() && nycPositions3D.find(dst) != nycPositions3D.end()) {
                            Vector3 p1 = nycPositions3D.at(src);
                            Vector3 p2 = nycPositions3D.at(dst);
                            float prog = (v.initialRoadTravelTime > 0.001f) ? Clamp(1.0f - (v.timeRemaining / v.initialRoadTravelTime), 0.0f, 1.0f) : 1.0f;
                            Vector3 vPos = Vector3Lerp(p1, p2, prog);
                            camTarget = Vector3Lerp(camTarget, vPos, 0.10f);
                        }
                        break;
                    }
                }
            }

            // Update 3D Camera Position & Target
            mainCam3D.target = camTarget;
            mainCam3D.position = Vector3{
                camTarget.x + sinf(camOrbitAngle) * cosf(camPitch) * camDistance,
                camTarget.y + sinf(camPitch) * camDistance,
                camTarget.z + cosf(camOrbitAngle) * cosf(camPitch) * camDistance
            };

            // Sync Actions State
            camActions.camAngle = camOrbitAngle;
            camActions.camPitch = camPitch;
            camActions.camDist = camDistance;
            camActions.isAutoOrbiting = autoOrbit;
            camActions.trackingTargetName = (chaseCarId != -1) ? TextFormat("VEHICLE #%d", chaseCarId) : "MANUAL PERSPECTIVE";
        } else {
            // Camera: Reset to default view for 2D
            if (IsKeyPressed(KEY_R)) {
                camera.target = { centerX, centerY };
                camera.offset = { centerX, centerY };
                camera.zoom = 1.0f;
                trackedCarId = -1;
            }

            // Driver Chase-Cam Tracking (Key T) for 2D
            if (IsKeyPressed(KEY_T)) {
                auto& vList = activeSim.cityManager->getVehicles();
                if (trackedCarId == -1) {
                    for (const auto& v : vList) {
                        if (v.state == 1) {
                            trackedCarId = v.id;
                            break;
                        }
                    }
                } else {
                    bool foundCur = false;
                    int nextId = -1;
                    for (const auto& v : vList) {
                        if (v.state == 1) {
                            if (foundCur) {
                                nextId = v.id;
                                break;
                            }
                            if (v.id == trackedCarId) foundCur = true;
                        }
                    }
                    trackedCarId = (nextId != -1) ? nextId : -1;
                }
            }

            // Camera Zoom (Mouse Wheel) for 2D
            if (inViewport) {
                float wheel = GetMouseWheelMove();
                if (wheel != 0.0f) {
                    Vector2 mouseWorldBefore = GetScreenToWorld2D(mousePos, camera);
                    camera.zoom = Clamp(camera.zoom + wheel * 0.12f, 0.45f, 3.5f);
                    Vector2 mouseWorldAfter = GetScreenToWorld2D(mousePos, camera);
                    camera.target = Vector2Add(camera.target, Vector2Subtract(mouseWorldBefore, mouseWorldAfter));
                }
            }

            // Camera Pan (Right Mouse Button Drag) for 2D
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && inViewport) {
                Vector2 delta = GetMouseDelta();
                delta = Vector2Scale(delta, -1.0f / camera.zoom);
                camera.target = Vector2Add(camera.target, delta);
                trackedCarId = -1;
            }
        }

        // Weather Cycle (Key W)
        if (IsKeyPressed(KEY_W)) {
            WeatherCondition curW = activeSim.cityManager->getWeather();
            WeatherCondition nextW = (WeatherCondition)(((int)curW + 1) % 4);
            activeSim.cityManager->setGlobalWeather(nextW);
        }

        // Road Incident Blockage & Rerouting (Key B)
        if (IsKeyPressed(KEY_B)) {
            isRoadBlocked = !isRoadBlocked;
            if (currentView == VIEW_REALM_OVERWORLD) {
                if (isRoadBlocked) activeSim.blockRoad("Mount Olympus", "Tartarus");
                else               activeSim.unblockRoad("Mount Olympus", "Tartarus");
            } else {
                string u = mapRef->getVertexAt(0);
                string v = mapRef->getVertexAt(1);
                if (isRoadBlocked) activeSim.blockRoad(u, v);
                else               activeSim.unblockRoad(u, v);
            }
        }

        // Emergency Vehicle Dispatch (Key E)
        if (IsKeyPressed(KEY_E)) {
            if (mapRef->Vcount >= 2) {
                if (currentView == VIEW_NYC_METRO_CITY) {
                    int u = nycParams.emergencyHubIdx % totalNodes;
                    int v = (u + 1 + rand() % (totalNodes - 1)) % totalNodes;
                    activeSim.cityManager->addVehicle(emergCarID++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), VEHICLE_EMERGENCY);
                } else {
                    string src = mapRef->getVertexAt(mapRef->Vcount - 1);
                    string dst = mapRef->getVertexAt(0);
                    activeSim.cityManager->addVehicle(emergCarID++, src, dst, VEHICLE_EMERGENCY);
                }
            }
        }

        // Emergency Signal Preemption ("Green Corridor" Wave)
        bool hasActiveEmergency = false;
        auto* currentNodes = mapRef->getNodes();
        for (const auto& v : activeSim.cityManager->getVehicles()) {
            if (v.isEmergency() && v.state == 1) {
                hasActiveEmergency = true;
                if (currentView == VIEW_NYC_METRO_CITY && nycParams.enableGreenCorridor) {
                    string src = v.currentRoad.first;
                    string dst = v.currentRoad.second;
                    int uIdx = mapRef->getIndex(src);
                    int vIdx = mapRef->getIndex(dst);
                    if (uIdx != -1 && vIdx != -1) {
                        for (auto& edge : currentNodes[uIdx].Neighbors) {
                            if (edge.index == vIdx) {
                                edge.weight.light.emergencyPreempted = true;
                                edge.weight.light.color = SIGNAL_GREEN; // Immediate Green Signal
                                edge.weight.light.greentimer = 10.0f;
                            }
                        }
                    }
                }
            }
        }

        // Commuter Wave Surge (Key P)
        if (IsKeyPressed(KEY_P)) {
            activeSim.cityManager->injectPeakDemand(total_time, true);
        }

        // Interactive Vehicle Dispatch: [V]/[I] = Single Vehicle/Car, [F] = 5x Fleet, [E] = Ambulance, [P] = Surge
        if (IsKeyPressed(KEY_V) || IsKeyPressed(KEY_I)) {
            static int customVehSeq = 65000;
            if (mapRef->Vcount >= 2) {
                int u = rand() % mapRef->Vcount;
                int v = (u + 1 + rand() % (mapRef->Vcount - 1)) % mapRef->Vcount;
                VehicleType randType = (IsKeyPressed(KEY_I)) ? VEHICLE_CAR : (VehicleType)(rand() % 6);
                activeSim.cityManager->addVehicle(customVehSeq++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), randType);
            }
        }
        if (IsKeyPressed(KEY_F)) {
            static int customFleetSeq = 75000;
            for (int k = 0; k < 5 && mapRef->Vcount >= 2; k++) {
                int u = rand() % mapRef->Vcount;
                int v = (u + 1 + rand() % (mapRef->Vcount - 1)) % mapRef->Vcount;
                VehicleType vt = (VehicleType)(rand() % 6);
                activeSim.cityManager->addVehicle(customFleetSeq++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), vt);
            }
        }

        // Realm View: Left-Click on City Node to Enter City View!
        if (currentView == VIEW_REALM_OVERWORLD && IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && inViewport) {
            for (int i = 0; i < 4; i++) {
                if (realmPositions.find(fantasyCityNames[i]) != realmPositions.end()) {
                    Vector2 p = realmPositions[fantasyCityNames[i]];
                    if (Vector2Distance(mouseWorld, p) <= 45.0f) {
                        selectedCityIdx = i;
                        currentView = VIEW_CITY_METROPOLIS;
                        currentPositions = cityPositions[i];
                        camera.target = { centerX, centerY };
                        camera.zoom = 1.0f;
                        trackedCarId = -1;
                        break;
                    }
                }
            }
        }

        // Smoothly interpolate positions
        for (const auto& pair : targetPositions) {
            if (currentPositions.find(pair.first) == currentPositions.end()) {
                currentPositions[pair.first] = pair.second;
            } else {
                currentPositions[pair.first] = Vector2Lerp(currentPositions[pair.first], pair.second, 0.22f);
            }
        }

        // =====================================================================
        // 2. SIMULATION ENGINE TICKS
        // =====================================================================
        int ticksToRun = 0;
        if (!isPaused) {
            ticksToRun = simSpeed;
        } else if (stepSingleTick) {
            ticksToRun = 1;
        }

        auto& vehicles = activeSim.cityManager->getVehicles();
        for (int step = 0; step < ticksToRun; step++) {
            // Continuous dynamic commuter traffic generation: maintain active traffic flow across network
            if (total_time % 18 == 0 && mapRef->Vcount >= 2 && vehicles.size() < 65) {
                static int autoVehSeq = 50000;
                int u = rand() % mapRef->Vcount;
                int v = rand() % mapRef->Vcount;
                int attempts = 0;
                while (attempts < 15 && (u == v || abs(u - v) < 2)) {
                    v = rand() % mapRef->Vcount;
                    attempts++;
                }
                if (u != v) {
                    VehicleType vType = (VehicleType)(rand() % 6);
                    activeSim.cityManager->addVehicle(autoVehSeq++, mapRef->getVertexAt(u), mapRef->getVertexAt(v), vType);
                }
            }

            activeSim.cityManager->advanceClock(0.08f);
            activeSim.cityManager->updateSignals();

            for (auto& v : vehicles) {
                if (v.state == 1) { 
                    float dt = 0.1f * v.speedMultiplier;
                    v.timeRemaining -= dt; 
                    v.timespent += 0.1f; 
                    v.updateFuelAndEmissions(0.1f, false, v.currentSpeed);
                }
                else if (v.state == 0) { 
                    v.timespent += 0.1f; 
                    v.updateFuelAndEmissions(0.1f, true, 0.0f);
                }
                if (v.timeRemaining < 0.0f) v.timeRemaining = 0.0f;
            }

            activeSim.cityManager->reached();
            activeSim.cityManager->arrivalAtIntersection();
            activeSim.cityManager->entraingfromQueetoEdge();
            activeSim.cityManager->entrance();
            activeSim.cityManager->checkBrokenPath();

            total_time++;
            if (total_time == 1 || total_time % 50 == 0) {
                activeSim.metric((float)total_time, activeSim.rush());
            }
        }

        // Telemetry sparkline recording
        static int sparkFrameCounter = 0;
        if (++sparkFrameCounter % 2 == 0) {
            rushHistory.push_back(activeSim.rush() * 100.0f);
            if (rushHistory.size() > 60) rushHistory.erase(rushHistory.begin());
        }

        // =====================================================================
        // 3. HOVER & INSPECTION DETECTION
        // =====================================================================
        string hoveredNode = "";
        float closestNodeDist = 1e9f;
        int hoveredVehicleId = -1;
        float closestVehDist = 1e9f;
        string hoveredRoadU = "";
        string hoveredRoadV = "";
        float closestRoadDist = 1e9f;
        auto* nodes = mapRef->getNodes();

        if (inViewport) {
            if (currentView == VIEW_NYC_METRO_CITY) {
                // 3D Screen-Space Hover Detection using projected coordinates
                for (auto const& pair : nycPositions3D) {
                    Vector2 sPos = GetWorldToScreen(Vector3{ pair.second.x, pair.second.y + 1.0f, pair.second.z }, mainCam3D);
                    float d = Vector2Distance(mousePos, sPos);
                    if (d <= 32.0f && d < closestNodeDist) {
                        closestNodeDist = d;
                        hoveredNode = pair.first;
                    }
                }
                if (hoveredNode.empty()) {
                    for (const auto& v : vehicles) {
                        if (v.state == 1) {
                            string src = v.currentRoad.first;
                            string dst = v.currentRoad.second;
                            if (nycPositions3D.find(src) != nycPositions3D.end() && nycPositions3D.find(dst) != nycPositions3D.end()) {
                                float prog = (v.initialRoadTravelTime > 0.001f) ? Clamp(1.0f - (v.timeRemaining / v.initialRoadTravelTime), 0.0f, 1.0f) : 1.0f;
                                Vector3 vPos = Vector3Lerp(nycPositions3D.at(src), nycPositions3D.at(dst), prog);
                                Vector2 sPos = GetWorldToScreen(vPos, mainCam3D);
                                float d = Vector2Distance(mousePos, sPos);
                                if (d <= 22.0f && d < closestVehDist) {
                                    closestVehDist = d;
                                    hoveredVehicleId = v.id;
                                }
                            }
                        }
                    }
                    if (hoveredVehicleId != -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        trackedCarId = hoveredVehicleId;
                        chaseCarId = hoveredVehicleId;
                    }
                }
            } else {
                // 2D World-Space Hover Detection for Fantasy Overworld & Metropolis
                for (auto const& pair : currentPositions) {
                    float d = Vector2Distance(mouseWorld, pair.second);
                    float hitR = (currentView == VIEW_REALM_OVERWORLD ? 35.0f : 20.0f) + 8.0f;
                    if (d <= hitR && d < closestNodeDist) {
                        closestNodeDist = d;
                        hoveredNode = pair.first;
                    }
                }

                if (hoveredNode.empty()) {
                    map<pair<string, string>, int> qDetectMap;
                    for (const auto& v : vehicles) {
                        Vector2 vPos = { 0, 0 };
                        bool hasP = false;
                        if (v.state == 1) {
                            string src = v.currentRoad.first;
                            string dst = v.currentRoad.second;
                            if (currentPositions.find(src) != currentPositions.end() && currentPositions.find(dst) != currentPositions.end()) {
                                float prog = (v.initialRoadTravelTime > 0.001f) ? Clamp(1.0f - (v.timeRemaining / v.initialRoadTravelTime), 0.0f, 1.0f) : 1.0f;
                                vPos = Vector2Lerp(currentPositions[src], currentPositions[dst], prog);
                                hasP = true;
                            }
                        } else if (v.state == 0 && v.path.size() >= 2) {
                            string src = v.path.front();
                            auto it = v.path.begin(); advance(it, 1);
                            string dst = *it;
                            if (currentPositions.find(src) != currentPositions.end() && currentPositions.find(dst) != currentPositions.end()) {
                                Vector2 sP = currentPositions[src];
                                Vector2 eP = currentPositions[dst];
                                Vector2 dir = Vector2Normalize(Vector2Subtract(eP, sP));
                                int qIdx = qDetectMap[{src, dst}]++;
                                vPos = Vector2Add(sP, Vector2Scale(dir, 16.0f + qIdx * 18.0f));
                                hasP = true;
                            }
                        }
                        if (hasP) {
                            float dist = Vector2Distance(mouseWorld, vPos);
                            if (dist <= 18.0f && dist < closestVehDist) {
                                closestVehDist = dist;
                                hoveredVehicleId = v.id;
                            }
                        }
                    }

                    if (hoveredVehicleId != -1 && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        trackedCarId = hoveredVehicleId;
                    }

                    if (hoveredVehicleId == -1) {
                        for (int i = 0; i < totalNodes; i++) {
                            string u = nodes[i].vertex;
                            if (currentPositions.find(u) == currentPositions.end()) continue;
                            Vector2 uPos = currentPositions[u];
                            for (auto const& edge : nodes[i].Neighbors) {
                                string v = mapRef->getVertexAt(edge.index);
                                if (currentPositions.find(v) == currentPositions.end()) continue;
                                Vector2 vPos = currentPositions[v];
                                float d = DistanceToSegment(mouseWorld, uPos, vPos);
                                if (d <= 12.0f && d < closestRoadDist) {
                                    closestRoadDist = d;
                                    hoveredRoadU = u;
                                    hoveredRoadV = v;
                                }
                            }
                        }
                    }
                }
            }
        }

        // =====================================================================
        // 4. RENDERING PIPELINE
        // =====================================================================
        BeginDrawing();
        ClearBackground({ 10, 14, 22, 255 }); // Dark mystical slate

        if (currentView == VIEW_NYC_METRO_CITY) {
            // =================================================================
            // 4.1 100% PURE 3D PERSPECTIVE WORLD RENDERING (NO 2D MAP)
            // =================================================================
            BeginMode3D(mainCam3D);
            DrawFull3DNYCMetroWorld(mapRef, nycPositions3D, nycParams, vehicles, hoveredNode, hoveredVehicleId, (chaseCarId != -1 ? chaseCarId : trackedCarId), hasActiveEmergency);
            EndMode3D();

            // 4.2 PROJECTED 2D HUD (Station names, route bullets, live signal countdown)
            DrawNYCMetroProjectedHUD(mapRef, nycPositions3D, nycParams, mainCam3D, hoveredNode, screenWidth, screenHeight);

            // 4.3 TOP NAVIGATION BANNER
            bool hovMenu = CheckCollisionPointRec(mousePos, btnMenuRec);
            DrawRectangleRec(btnMenuRec, Fade(Color{ 30, 42, 60, 255 }, hovMenu ? 0.95f : 0.85f));
            DrawRectangleLinesEx(btnMenuRec, 1.5f, hovMenu ? RAYWHITE : Fade(GOLD, 0.7f));
            DrawText("< MENU [M]", (int)btnMenuRec.x + 20, (int)btnMenuRec.y + 12, 14, RAYWHITE);

            bool hovOpt = CheckCollisionPointRec(mousePos, btnOptRec);
            DrawRectangleRec(btnOptRec, Fade(Color{ 210, 90, 20, 255 }, hovOpt ? 0.95f : 0.85f));
            DrawRectangleLinesEx(btnOptRec, 1.5f, hovOpt ? RAYWHITE : Fade(GOLD, 0.7f));
            DrawText("RUN OPTIMIZER [O]", (int)btnOptRec.x + 22, (int)btnOptRec.y + 12, 14, RAYWHITE);

            bool hovCfg = CheckCollisionPointRec(mousePos, nycConfigBtnRec);
            DrawRectangleRec(nycConfigBtnRec, Fade(GetMTAColorBlue(), hovCfg ? 0.95f : 0.85f));
            DrawRectangleLinesEx(nycConfigBtnRec, 1.5f, hovCfg ? RAYWHITE : GetMTAColorYellow());
            DrawText("CONFIG [F1]", (int)nycConfigBtnRec.x + 25, (int)nycConfigBtnRec.y + 12, 14, RAYWHITE);

            int bannerX = 945;
            int bannerW = screenWidth - bannerX - 20;
            if (bannerW > 100) {
                DrawRectangle(bannerX, 16, bannerW, 40, Fade(Color{ 14, 17, 24, 255 }, 0.92f));
                DrawRectangleLines(bannerX, 16, bannerW, 40, Fade(GetMTAColorYellow(), 0.5f));
                DrawText("NYC METRO 3D TRANSIT | BLINKING SIGNAL NODES", bannerX + 15, 27, 15, GetMTAColorYellow());
            }

            // 4.4 LEFT TELEMETRY SIDEBAR WITH 3D CAMERA SUITE
            bool triggerSettings = false;
            bool triggerExport = false;
            DrawNYCMetroTelemetrySidebar(nycSim, nycParams, currentPositions, total_time, isPaused, simSpeed, rushHistory, screenWidth, screenHeight, triggerSettings, triggerExport, camActions);
            if (triggerSettings) {
                currentView = VIEW_CITY_DESIGNER;
            }
            if (triggerExport) {
                ExportCityPlanningMetrics(nycSim, nycParams, total_time);
            }
        } else {
            // --- 4.1 2D CAMERA WORLD RENDERING (FOR REALM OVERWORLD & FANTASY METROPOLIS) ---
            BeginMode2D(camera);

            // Faint tactical radar grid
            for (int gx = 200; gx <= screenWidth + 1200; gx += 160) {
                DrawLine(gx, -1200, gx, screenHeight + 1200, Fade(DARKGRAY, 0.12f));
            }
            for (int gy = -1200; gy <= screenHeight + 1200; gy += 160) {
                DrawLine(200, gy, screenWidth + 1200, gy, Fade(DARKGRAY, 0.12f));
            }

            // Draw Roads & Highways
            for (int i = 0; i < totalNodes; i++) {
                string u = nodes[i].vertex;
                if (currentPositions.find(u) == currentPositions.end()) continue;
                Vector2 uPos = currentPositions[u];

                for (auto const& edge : nodes[i].Neighbors) {
                    string v = mapRef->getVertexAt(edge.index);
                    if (currentPositions.find(v) == currentPositions.end()) continue;
                    Vector2 vPos = currentPositions[v];

                    float congestion = edge.weight.Congestion();
                    Color heatCol = GetMetroHeatColor(congestion);
                    float roadThick = (currentView == VIEW_REALM_OVERWORLD) ? 5.5f : (edge.weight.numLanes >= 3 ? 4.5f : 3.0f);

                    if (edge.weight.isBlocked) {
                        DrawLineEx(uPos, vPos, roadThick + 2.0f, Fade(RED, 0.85f));
                    } else {
                        DrawLineEx(uPos, vPos, roadThick, Fade(heatCol, 0.70f));
                    }

                    DrawRoadArrowBold(uPos, vPos, heatCol);

                    // Realistic 3-Aspect Traffic Signal Head with lens glow, visors, countdown timer & queue badge
                    Vector2 dir = Vector2Normalize(Vector2Subtract(vPos, uPos));
                    float stopDist = (currentView == VIEW_REALM_OVERWORLD ? 45.0f : 28.0f);
                    Vector2 stopPos = Vector2Subtract(vPos, Vector2Scale(dir, stopDist));
                    DrawTrafficSignalHead(stopPos, dir, edge.weight.light, edge.weight.queueCount, (currentView == VIEW_REALM_OVERWORLD));
                }
            }

            // Draw Vehicles (En Route + Queued Stopped at Signals)
            map<pair<string, string>, int> roadQueueDrawCount;

            for (const auto& v : vehicles) {
                if (v.state == 1) {
                    string src = v.currentRoad.first;
                    string dst = v.currentRoad.second;
                    if (currentPositions.find(src) == currentPositions.end() || currentPositions.find(dst) == currentPositions.end()) continue;

                    Vector2 startPos = currentPositions[src];
                    Vector2 endPos = currentPositions[dst];
                    Vector2 roadVec = Vector2Subtract(endPos, startPos);
                    float roadLen = Vector2Length(roadVec);
                    if (roadLen < 0.001f) continue;
                    Vector2 dir = Vector2Scale(roadVec, 1.0f / roadLen);
                    Vector2 perp = { -dir.y, dir.x };

                    float progress = 1.0f;
                    if (v.initialRoadTravelTime > 0.001f) {
                        progress = 1.0f - (v.timeRemaining / v.initialRoadTravelTime);
                        progress = Clamp(progress, 0.0f, 1.0f);
                    }

                    float stopDist = (currentView == VIEW_REALM_OVERWORLD ? 45.0f : 28.0f);
                    float usableLength = roadLen - stopDist;
                    if (usableLength < 10.0f) usableLength = roadLen * 0.85f;
                    float distTraveled = progress * usableLength;
                    Vector2 pos = Vector2Add(startPos, Vector2Scale(dir, distTraveled));

                    float laneOffset = (v.currentLane - 1.0f) * (currentView == VIEW_REALM_OVERWORLD ? 7.0f : 5.0f);
                    pos = Vector2Add(pos, Vector2Scale(perp, laneOffset));

                    float angleDeg = atan2f(dir.y, dir.x) * (180.0f / PI);

                    if (v.id == trackedCarId) {
                        float pulse = 2.0f * sinf((float)GetTime() * 8.0f);
                        DrawCircleLines((int)pos.x, (int)pos.y, 16.0f + pulse, LIME);
                        DrawCircleLines((int)pos.x, (int)pos.y, 20.0f + pulse, Fade(LIME, 0.4f));
                    } else if (v.id == hoveredVehicleId) {
                        DrawCircleLines((int)pos.x, (int)pos.y, 15.0f, GOLD);
                    }

                    DrawDetailedVehicle(pos, angleDeg, v, false);
                }
                else if (v.state == 0 && v.path.size() >= 2) {
                    string src = v.path.front();
                    auto it = v.path.begin(); advance(it, 1);
                    string dst = *it;
                    if (currentPositions.find(src) == currentPositions.end() || currentPositions.find(dst) == currentPositions.end()) continue;

                    Vector2 startPos = currentPositions[src];
                    Vector2 endPos = currentPositions[dst];
                    Vector2 roadVec = Vector2Subtract(endPos, startPos);
                    float roadLen = Vector2Length(roadVec);
                    if (roadLen < 0.001f) continue;
                    Vector2 dir = Vector2Scale(roadVec, 1.0f / roadLen);
                    Vector2 perp = { -dir.y, dir.x };

                    int qIdx = roadQueueDrawCount[{src, dst}]++;
                    Vector2 qPos = Vector2Add(startPos, Vector2Scale(dir, 16.0f + qIdx * 18.0f));
                    float laneOffset = (v.currentLane - 1.0f) * (currentView == VIEW_REALM_OVERWORLD ? 7.0f : 5.0f);
                    qPos = Vector2Add(qPos, Vector2Scale(perp, laneOffset));

                    float angleDeg = atan2f(dir.y, dir.x) * (180.0f / PI);

                    if (v.id == trackedCarId) {
                        float pulse = 2.0f * sinf((float)GetTime() * 8.0f);
                        DrawCircleLines((int)qPos.x, (int)qPos.y, 16.0f + pulse, LIME);
                    } else if (v.id == hoveredVehicleId) {
                        DrawCircleLines((int)qPos.x, (int)qPos.y, 15.0f, GOLD);
                    }

                    DrawDetailedVehicle(qPos, angleDeg, v, true);
                }
            }

            // Draw Nodes for Realm Overworld & Fantasy Metropolis
            if (currentView == VIEW_REALM_OVERWORLD) {
                for (int i = 0; i < 4; i++) {
                    string cityName = fantasyCityNames[i];
                    if (currentPositions.find(cityName) == currentPositions.end()) continue;
                    Vector2 pos = currentPositions[cityName];
                    bool isHovered = (cityName == hoveredNode);
                    Color cityThemeCol = GetFantasyCityColor(cityName);

                    int nodeIdx = mapRef->getIndex(cityName);
                    NodeSignalState sig = GetNodeSignalState(mapRef, nodeIdx, cityName);

                    // Dynamic radar pulse wave ring expanding based on traffic signal
                    float pulseR = 35.0f + sig.pulseRadius * 20.0f;
                    float pulseAlpha = (1.0f - sig.pulseRadius) * 0.80f * sig.blinkAlpha;
                    DrawCircleLines((int)pos.x, (int)pos.y, pulseR, Fade(sig.primaryColor, pulseAlpha));

                    // Blinking signal halo ring
                    float haloR = 34.0f + 6.0f * sig.intensity;
                    DrawCircleLines((int)pos.x, (int)pos.y, haloR, Fade(sig.primaryColor, 0.65f * sig.intensity));

                    if (isHovered) {
                        DrawCircleV(pos, 38.0f, Fade(cityThemeCol, 0.35f));
                        DrawCircleLines((int)pos.x, (int)pos.y, 40.0f, RAYWHITE);
                    }

                    // Node body with city theme & signal illumination
                    DrawCircleV(pos, 28.0f, Fade(cityThemeCol, 0.85f));
                    DrawCircleV(pos, 22.0f, { 15, 20, 30, 255 });

                    // Center traffic signal beacon that actively blinks
                    DrawCircleV(pos, 12.0f, Fade(sig.primaryColor, 0.90f * sig.intensity));
                    DrawCircleV(pos, 7.0f, sig.primaryColor);
                    DrawCircleV(pos, 3.5f, Fade(RAYWHITE, sig.blinkAlpha));

                    DrawText(cityName.c_str(), (int)pos.x - 60, (int)pos.y + 35, 20, RAYWHITE);
                    const char* sigPill = sig.isEmergency ? "EMS PREEMPT" : (sig.color == SIGNAL_GREEN ? "SIGNAL: GREEN" : (sig.color == SIGNAL_YELLOW ? "SIGNAL: YELLOW" : "SIGNAL: RED"));
                    DrawText(sigPill, (int)pos.x - 52, (int)pos.y + 58, 12, sig.primaryColor);
                }
            } else {
                for (auto const& pair : currentPositions) {
                    bool isHovered = (pair.first == hoveredNode);
                    float baseRadius = 16.0f;

                    int nodeIdx = mapRef->getIndex(pair.first);
                    NodeSignalState sig = GetNodeSignalState(mapRef, nodeIdx, pair.first);

                    // 1. Dynamic expanding pulse wave ring based on traffic signal
                    float pulseR = baseRadius + 4.0f + sig.pulseRadius * 16.0f;
                    float pulseAlpha = (1.0f - sig.pulseRadius) * 0.80f * sig.blinkAlpha;
                    DrawCircleLines((int)pair.second.x, (int)pair.second.y, pulseR, Fade(sig.primaryColor, pulseAlpha));

                    // 2. Concentric blinking halo in traffic signal color
                    float haloR = baseRadius + 3.0f + 4.0f * sig.intensity;
                    DrawCircleLines((int)pair.second.x, (int)pair.second.y, haloR, Fade(sig.primaryColor, 0.60f * sig.intensity));
                    DrawCircleV(pair.second, baseRadius + 2.0f, Fade(sig.haloColor, 0.35f * sig.intensity));

                    if (isHovered) {
                        DrawCircleLines((int)pair.second.x, (int)pair.second.y, baseRadius + 8.0f, LIME);
                        DrawCircleV(pair.second, baseRadius + 4.0f, Fade(LIME, 0.35f));
                    }

                    // 3. Node chassis & outer rim illuminated by traffic signal
                    DrawCircleV(pair.second, baseRadius, Fade(sig.primaryColor, 0.85f * sig.intensity));
                    DrawCircleV(pair.second, baseRadius - 4.0f, { 15, 20, 30, 255 });
                    DrawCircleLines((int)pair.second.x, (int)pair.second.y, baseRadius, RAYWHITE);

                    // 4. Center traffic signal blinking lens
                    DrawCircleV(pair.second, 7.0f, sig.primaryColor);
                    DrawCircleV(pair.second, 3.5f, Fade(RAYWHITE, sig.blinkAlpha));

                    // 5. Node name label and live signal badge
                    DrawText(pair.first.c_str(), (int)pair.second.x + (int)baseRadius + 8, (int)pair.second.y - 10, 15, RAYWHITE);
                    const char* sigBadge = sig.isEmergency ? "EMS" : (sig.color == SIGNAL_GREEN ? "GRN" : (sig.color == SIGNAL_YELLOW ? "YEL" : "RED"));
                    if (sig.totalWaitingQueue > 0) {
                        DrawText(TextFormat("[%s | Q:%d]", sigBadge, sig.totalWaitingQueue), (int)pair.second.x + (int)baseRadius + 8, (int)pair.second.y + 6, 12, sig.primaryColor);
                    } else {
                        DrawText(TextFormat("[%s]", sigBadge), (int)pair.second.x + (int)baseRadius + 8, (int)pair.second.y + 6, 12, sig.primaryColor);
                    }
                }
            }

            // Weather & Night Overlay
            WeatherCondition activeWeather = activeSim.cityManager->getWeather();
            if (activeWeather == WEATHER_RAIN) {
                DrawRectangle(-1200, -1200, screenWidth + 2400, screenHeight + 2400, Fade(BLUE, 0.12f));
            } else if (activeWeather == WEATHER_SMOG) {
                DrawRectangle(-1200, -1200, screenWidth + 2400, screenHeight + 2400, Fade(BROWN, 0.22f));
            } else if (activeWeather == WEATHER_DENSE_FOG) {
                DrawRectangle(-1200, -1200, screenWidth + 2400, screenHeight + 2400, Fade(LIGHTGRAY, 0.32f));
            }

            float darkness = activeSim.cityManager->getAmbientDarkness();
            if (darkness > 0.05f) {
                DrawRectangle(-1200, -1200, screenWidth + 2400, screenHeight + 2400, Fade(BLACK, darkness));
            }

            EndMode2D(); // World rendering complete

            // 4.2 Screen Space Top Nav Bar for Overworld / Metropolis
            if (currentView == VIEW_CITY_METROPOLIS) {
                DrawRectangleRec(backBtnRec, Fade({ 20, 35, 55, 255 }, 0.95f));
                DrawRectangleLinesEx(backBtnRec, 1.5f, Fade(LIME, 0.8f));
                DrawText("< BACK TO REALM OVERWORLD (ESC)", (int)backBtnRec.x + 12, (int)backBtnRec.y + 12, 14, LIME);

                string title = fantasyCityNames[selectedCityIdx] + " METROPOLIS | URBAN PLANNING & SIGNAL OPTIMIZER";
                DrawText(title.c_str(), (int)backBtnRec.x + 315, 26, 20, GetFantasyCityColor(fantasyCityNames[selectedCityIdx]));
            } else {
                DrawRectangle(420, 16, screenWidth - 440, 44, Fade({ 15, 22, 35, 255 }, 0.92f));
                DrawRectangleLines(420, 16, screenWidth - 440, 44, Fade(GOLD, 0.5f));
                DrawText("MYTHICAL REALM OVERWORLD | 4 FANTASY CAPITALS | CLICK ANY CITY NODE TO ENTER METROPOLITAN VIEW", 440, 28, 17, GOLD);
            }

            // 4.3 Left Operations Sidebar for Overworld / Metropolis
            int sidebarW = 400;
            DrawRectangle(0, 0, sidebarW, screenHeight, Fade(BLACK, 0.92f));
            DrawLineEx({ (float)sidebarW, 0 }, { (float)sidebarW, (float)screenHeight }, 2.0f, Fade(GREEN, 0.4f));

            if (currentView == VIEW_REALM_OVERWORLD) {
                // Realm Overworld Sidebar
                DrawText("REALM OVERWORLD OPS", 35, 25, 24, GOLD);
                DrawText("INTER-PLANAR TRANSIT", 35, 52, 14, Fade(YELLOW, 0.7f));
                DrawLineEx({ 25, 75 }, { 375, 75 }, 1.0f, Fade(GOLD, 0.3f));

                DrawText("THE 4 MYTHICAL REALMS:", 25, 90, 16, Fade(RAYWHITE, 0.85f));
                DrawText("[1] Mount Olympus  - Celestial Peak", 45, 120, 15, GOLD);
                DrawText("[2] Tartarus       - Abyssal Underworld", 45, 148, 15, RED);
                DrawText("[3] Atlantis       - Sunken Sea Citadel", 45, 176, 15, SKYBLUE);
                DrawText("[4] Elysium        - Blessed Golden Plains", 45, 204, 15, LIME);

                DrawLineEx({ 25, 235 }, { 375, 235 }, 1.0f, Fade(GRAY, 0.3f));

                DrawText("OVERWORLD SIMULATION:", 25, 248, 15, Fade(LIME, 0.7f));
                const char* playStateText = isPaused ? "[PAUSED]" : TextFormat("[RUNNING %dx]", simSpeed);
                DrawText(playStateText, 250, 248, 15, isPaused ? ORANGE : LIME);

                DrawText(TextFormat("TIME TICK:       %d", total_time), 45, 275, 18, RAYWHITE);
                DrawText(TextFormat("ACTIVE FLEETS:   %d", (int)vehicles.size()), 45, 302, 18, RAYWHITE);
                DrawText(TextFormat("ARRIVALS:        %d", (int)activeSim.cityManager->getArrivedCount()), 45, 329, 18, YELLOW);

                DrawLineEx({ 25, 360 }, { 375, 360 }, 1.0f, Fade(GRAY, 0.3f));
                DrawText("OVERWORLD RUSH TELEMETRY", 25, 372, 15, Fade(LIME, 0.7f));
                float rushVal = activeSim.rush();
                Color rushColor = GetMetroHeatColor(rushVal);
                DrawText(TextFormat("%.2f%%", rushVal * 100.0f), 45, 395, 22, rushColor);

                Rectangle sparklineBounds = { 45.0f, 425.0f, 310.0f, 55.0f };
                DrawSparkline(rushHistory, sparklineBounds, rushColor, Fade(DARKGRAY, 0.25f), "INTER-REALM WAVEFORM", 100.0f);

                DrawLineEx({ 25, 495 }, { 375, 495 }, 1.0f, Fade(GRAY, 0.3f));
                DrawText("INTER-REALM TRAVEL ANALYTICS", 25, 508, 15, Fade(LIME, 0.7f));
                DrawText(TextFormat("AVG TRAVEL:   %.1f ticks", activeSim.cityManager->getAvgTravelTime()), 45, 532, 17, RAYWHITE);
                DrawText(TextFormat("THROUGHPUT:   %.2f v/tick", activeSim.cityManager->getThroughput((float)total_time)), 45, 558, 17, SKYBLUE);
                DrawText(TextFormat("CO2 EMITTED:  %.1f kg", activeSim.cityManager->getTotalCO2Emitted()), 45, 584, 17, ORANGE);

                DrawLineEx({ 25, 615 }, { 375, 615 }, 1.0f, Fade(GOLD, 0.3f));
                DrawRectangle(25, 628, 350, 75, Fade({ 20, 30, 45, 255 }, 0.85f));
                DrawRectangleLines(25, 628, 350, 75, Fade(GOLD, 0.6f));
                DrawText("HOW TO NAVIGATE:", 38, 638, 14, GOLD);
                DrawText("* Click any city node on the map", 38, 658, 13, RAYWHITE);
                DrawText("* Or press Keys [1], [2], [3], [4]", 38, 678, 13, YELLOW);

                DrawLineEx({ 25, 715 }, { 375, 715 }, 1.0f, Fade(GRAY, 0.3f));
                DrawText("INTERACTIVE CONTROLS", 25, 725, 15, Fade(LIME, 0.7f));
                DrawText("[SPACE] Play/Pause  [N] Step 1 Tick", 45, 746, 13, RAYWHITE);
                DrawText("[Z/X/C] Speed (1x, 2x, 5x)", 45, 766, 13, RAYWHITE);
                DrawText("[B] Block Mythical Road Link", 45, 786, 13, ORANGE);
                DrawText("[P] Inject Commuter Rush Wave", 45, 806, 13, GREEN);
                DrawText("[W] Cycle Weather Presets", 45, 826, 13, SKYBLUE);
            } else {
                // City Metropolis Planning Sidebar
                Color cityCol = GetFantasyCityColor(fantasyCityNames[selectedCityIdx]);
                DrawText("CITY PLANNING HELPER", 35, 25, 22, cityCol);
                DrawText(fantasyCityNames[selectedCityIdx].c_str(), 35, 52, 15, Fade(RAYWHITE, 0.8f));
                DrawLineEx({ 25, 75 }, { 375, 75 }, 1.0f, Fade(cityCol, 0.3f));

                DrawText("METROPOLIS SIMULATION", 25, 85, 15, Fade(LIME, 0.7f));
                const char* playStateText = isPaused ? "[PAUSED]" : TextFormat("[RUNNING %dx]", simSpeed);
                DrawText(playStateText, 250, 85, 15, isPaused ? ORANGE : LIME);

                DrawText(TextFormat("TIME TICK:       %d", total_time), 45, 110, 18, RAYWHITE);
                DrawText(TextFormat("ACTIVE VEHICLES: %d", (int)vehicles.size()), 45, 135, 18, RAYWHITE);
                DrawText(TextFormat("COMPLETED TRIPS: %d", (int)activeSim.cityManager->getArrivedCount()), 45, 160, 18, YELLOW);

                DrawLineEx({ 25, 188 }, { 375, 188 }, 1.0f, Fade(GRAY, 0.3f));
                DrawText("URBAN CONGESTION TELEMETRY", 25, 198, 15, Fade(LIME, 0.7f));
                float rushVal = activeSim.rush();
                Color rushColor = GetMetroHeatColor(rushVal);
                DrawText(TextFormat("%.2f%%", rushVal * 100.0f), 45, 220, 22, rushColor);

                Rectangle sparklineBounds = { 45.0f, 250.0f, 310.0f, 52.0f };
                DrawSparkline(rushHistory, sparklineBounds, rushColor, Fade(DARKGRAY, 0.25f), "CITY RUSH WAVEFORM", 100.0f);

                DrawLineEx({ 25, 315 }, { 375, 315 }, 1.0f, Fade(GRAY, 0.3f));
                DrawText("SIGNAL OPTIMIZATION & HCM LOS", 25, 326, 15, Fade(LIME, 0.7f));
                const char* stratName = (activeStrategyIdx == 1) ? "GA-OPTIMIZED (GENETIC)" : 
                                        (activeStrategyIdx == 0 ? "WEBSTER FIXED SPLITS" : 
                                        (activeStrategyIdx == 2 ? "ACTUATED (GAP-OUT)" : "COORDINATED ARTERIAL"));
                DrawText(stratName, 45, 348, 15, GOLD);

                DrawText(TextFormat("HCM GRADE:    %s", getLOSName(activeSim.cityReport.optimizedLOS).c_str()), 45, 372, 16, GREEN);
                DrawText(TextFormat("AVG DELAY:    %.1fs (P95: %.1fs)", activeSim.cityReport.optimizedAvgDelay, activeSim.cityReport.optimizedP95Delay), 45, 396, 14, RAYWHITE);
                DrawText(TextFormat("DELAY REDUCE: -%.1f%% vs Baseline", activeSim.cityReport.delayReductionPct), 45, 418, 14, LIME);
                DrawText(TextFormat("CO2 REDUCE:   -%.1f%% (-%.1f kg)", activeSim.cityReport.co2ReductionPct, activeSim.cityReport.baselineCO2kg - activeSim.cityReport.optimizedCO2kg), 45, 440, 14, SKYBLUE);

                DrawLineEx({ 25, 468 }, { 375, 468 }, 1.0f, Fade(GRAY, 0.3f));
                DrawText("ENVIRONMENT & FLEET", 25, 478, 15, Fade(LIME, 0.7f));
                DrawText(TextFormat("TOTAL CO2:    %.1f kg", activeSim.cityManager->getTotalCO2Emitted()), 45, 500, 15, ORANGE);
                DrawText(TextFormat("FUEL BURN:    %.1f Liters", activeSim.cityManager->getTotalFuelConsumed()), 45, 522, 15, YELLOW);
                DrawText(TextFormat("THROUGHPUT:   %.2f v/tick", activeSim.cityManager->getThroughput((float)total_time)), 45, 544, 15, SKYBLUE);

                DrawLineEx({ 25, 575 }, { 375, 575 }, 1.0f, Fade(GRAY, 0.3f));
                DrawText("METROPOLIS HOTKEYS", 25, 586, 15, Fade(LIME, 0.7f));
                DrawText("[ESC] Return to Realm Overworld", 45, 608, 14, LIME);
                DrawText("[O] Run Genetic Algorithm Optimizer", 45, 628, 14, YELLOW);
                DrawText("[S] Cycle Signal Strategy", 45, 648, 14, RAYWHITE);
                DrawText("[P] Inject Peak Commuter Surge", 45, 668, 14, GREEN);
                DrawText("[B] Road Construction Blockage", 45, 688, 14, ORANGE);
                DrawText("[E] Priority Hospital Ambulance", 45, 708, 14, RED);
                DrawText("[T] Driver Chase-Cam Tracking", 45, 728, 14, SKYBLUE);
                DrawText("[W] Cycle Weather Presets", 45, 748, 14, Fade(RAYWHITE, 0.8f));
                DrawText("[SPACE] Pause/Play  [N] Single Tick", 45, 768, 14, GRAY);
            }
        }

        // --- 4.4 FLOATING TOP-RIGHT INSPECTION HUD CARD ---
        int hudX = screenWidth - 370;
        int hudY = 70;
        int hudW = 345;
        int hudH = 220;

        DrawRectangle(hudX, hudY, hudW, hudH, Fade({ 10, 15, 25, 255 }, 0.92f));
        DrawRectangleLines(hudX, hudY, hudW, hudH, Fade(LIME, 0.5f));

        if (trackedCarId != -1) {
            const vehicle<string>* trackedV = nullptr;
            for (const auto& v : vehicles) {
                if (v.id == trackedCarId) {
                    trackedV = &v;
                    break;
                }
            }
            if (trackedV) {
                DrawText("CHASE-CAM TELEMETRY", hudX + 15, hudY + 12, 15, LIME);
                const char* typeName = trackedV->isEmergency() ? "AMBULANCE" : (trackedV->isEV() ? "ELECTRIC (EV)" : (trackedV->isTruck() ? "FREIGHT TRUCK" : (trackedV->isBus() ? "PASSENGER BUS" : (trackedV->isMotorcycle() ? "MOTORCYCLE" : "PRIVATE CAR"))));
                DrawText(TextFormat("UNIT #%d: %s", trackedV->id, typeName), hudX + 15, hudY + 36, 16, GOLD);
                DrawText(TextFormat("%s -> %s", trackedV->source.c_str(), trackedV->dest.c_str()), hudX + 15, hudY + 58, 14, SKYBLUE);
                DrawLine(hudX + 15, hudY + 76, hudX + hudW - 15, hudY + 76, Fade(DARKGRAY, 0.5f));

                float curSpeedKmh = trackedV->currentSpeed * 3.6f;
                DrawText(TextFormat("Speed:         %.1f km/h", curSpeedKmh), hudX + 15, hudY + 84, 14, RAYWHITE);
                DrawText(TextFormat("Time Left:     %.1f ticks", trackedV->timeRemaining), hudX + 15, hudY + 104, 14, YELLOW);
                if (trackedV->isEV()) {
                    DrawText(TextFormat("Battery SoC:   %.1f%%", trackedV->batterySoCPercent), hudX + 15, hudY + 124, 14, GREEN);
                } else {
                    DrawText(TextFormat("Fuel Burned:   %.2f Liters", trackedV->fuelConsumedLiters), hudX + 15, hudY + 124, 14, ORANGE);
                    DrawText(TextFormat("CO2 Emitted:   %.2f kg", trackedV->co2EmittedKg), hudX + 15, hudY + 144, 14, Fade(RED, 0.9f));
                }
                DrawText("[PRESS T / ESC TO EXIT CHASE-CAM]", hudX + 15, hudY + 175, 12, Fade(GRAY, 0.8f));
            } else {
                trackedCarId = -1;
            }
        }
        else if (hoveredVehicleId != -1) {
            const vehicle<string>* hV = nullptr;
            for (const auto& v : vehicles) {
                if (v.id == hoveredVehicleId) {
                    hV = &v;
                    break;
                }
            }
            if (hV) {
                DrawText("VEHICLE RADAR INSPECTION", hudX + 15, hudY + 12, 15, GOLD);
                const char* typeName = hV->isEmergency() ? "AMBULANCE" : (hV->isEV() ? "ELECTRIC (EV)" : (hV->isTruck() ? "FREIGHT TRUCK" : (hV->isBus() ? "PASSENGER BUS" : (hV->isMotorcycle() ? "MOTORCYCLE" : "PRIVATE CAR"))));
                DrawText(TextFormat("UNIT #%d: %s", hV->id, typeName), hudX + 15, hudY + 36, 16, GOLD);
                DrawText(TextFormat("%s -> %s", hV->source.c_str(), hV->dest.c_str()), hudX + 15, hudY + 58, 14, SKYBLUE);
                DrawLine(hudX + 15, hudY + 76, hudX + hudW - 15, hudY + 76, Fade(DARKGRAY, 0.5f));

                const char* stStr = (hV->state == 1) ? "EN ROUTE (HIGHWAY)" : (hV->state == 0 ? "QUEUED AT RED SIGNAL" : "ARRIVED");
                Color stCol = (hV->state == 1) ? LIME : (hV->state == 0 ? RED : YELLOW);
                DrawText(TextFormat("Status:        %s", stStr), hudX + 15, hudY + 84, 13, stCol);
                DrawText(TextFormat("Speed:         %.1f km/h", hV->currentSpeed * 3.6f), hudX + 15, hudY + 104, 13, RAYWHITE);
                DrawText(TextFormat("Lane:          Lane %d / Multi-lane", hV->currentLane + 1), hudX + 15, hudY + 124, 13, YELLOW);
                if (hV->isEV()) {
                    DrawText(TextFormat("Battery SoC:   %.1f%% (Green Eco)", hV->batterySoCPercent), hudX + 15, hudY + 144, 13, GREEN);
                } else {
                    DrawText(TextFormat("CO2 Emitted:   %.2f kg", hV->co2EmittedKg), hudX + 15, hudY + 144, 13, ORANGE);
                }
                DrawText("[CLICK VEHICLE TO LOCK CHASE-CAM]", hudX + 15, hudY + 175, 12, Fade(LIME, 0.9f));
            }
        }
        else if (!hoveredNode.empty()) {
            if (currentView == VIEW_REALM_OVERWORLD) {
                DrawText("REALM CITADEL INSPECTOR", hudX + 15, hudY + 12, 15, GOLD);
                DrawText(hoveredNode.c_str(), hudX + 15, hudY + 36, 18, GetFantasyCityColor(hoveredNode));
                DrawLine(hudX + 15, hudY + 60, hudX + hudW - 15, hudY + 60, Fade(DARKGRAY, 0.5f));

                DrawText("Click to enter inner metropolitan grid", hudX + 15, hudY + 72, 13, LIME);
                DrawText("11 Urban Planning Intersections", hudX + 15, hudY + 94, 13, RAYWHITE);
                DrawText("C++ Genetic Algorithm Signal Timing", hudX + 15, hudY + 116, 13, YELLOW);
                DrawText("Microscopic IDM Vehicle Flow", hudX + 15, hudY + 138, 13, SKYBLUE);
                DrawText("[CLICK NODE TO OPEN CITY VIEW]", hudX + 15, hudY + 175, 13, GOLD);
            } else {
                DrawText("INTERSECTION INSPECTOR", hudX + 15, hudY + 12, 15, LIME);
                DrawText(TextFormat("DISTRICT: %s", hoveredNode.c_str()), hudX + 15, hudY + 36, 16, GOLD);
                DrawLine(hudX + 15, hudY + 60, hudX + hudW - 15, hudY + 60, Fade(DARKGRAY, 0.5f));

                // Summarize inbound links
                int inboundCount = 0;
                int totalQ = 0;
                for (int i = 0; i < totalNodes; i++) {
                    for (auto const& edge : nodes[i].Neighbors) {
                        if (mapRef->getVertexAt(edge.index) == hoveredNode) {
                            inboundCount++;
                            totalQ += edge.weight.queueCount;
                        }
                    }
                }
                DrawText(TextFormat("Inbound Approaches: %d", inboundCount), hudX + 15, hudY + 72, 14, RAYWHITE);
                DrawText(TextFormat("Total Queue Depth:  %d vehicles", totalQ), hudX + 15, hudY + 94, 14, ORANGE);
                DrawText("Adaptive Signal Controller Active", hudX + 15, hudY + 116, 13, GREEN);
                DrawText("[PRESS S TO CYCLE STRATEGY]", hudX + 15, hudY + 175, 12, Fade(GRAY, 0.8f));
            }
        }
        else if (!hoveredRoadU.empty() && !hoveredRoadV.empty()) {
            RoadDetails& rd = mapRef->getEdgeDetails(hoveredRoadU, hoveredRoadV);
            float bprFactor = 1.0f + rd.a * powf(rd.Congestion(), rd.b);
            float dynSpeed = (bprFactor > 0.0f) ? (rd.max_speed / bprFactor) : rd.max_speed;

            DrawText("HIGHWAY LINK INSPECTOR", hudX + 15, hudY + 12, 15, LIME);
            DrawText(TextFormat("%s -> %s", hoveredRoadU.c_str(), hoveredRoadV.c_str()), hudX + 15, hudY + 36, 16, SKYBLUE);

            const char* statusText = rd.isBlocked ? "BLOCKED (DETOUR ACTIVE)" : "OPEN (FLOWING)";
            Color statusColor = rd.isBlocked ? MAROON : GREEN;
            DrawText(TextFormat("Status: %s", statusText), hudX + 15, hudY + 60, 14, statusColor);
            DrawLine(hudX + 15, hudY + 80, hudX + hudW - 15, hudY + 80, Fade(DARKGRAY, 0.5f));

            DrawText(TextFormat("Length:        %.0f km", rd.length), hudX + 15, hudY + 90, 13, RAYWHITE);
            DrawText(TextFormat("Speed Limit:   %.0f km/h", rd.max_speed), hudX + 15, hudY + 110, 13, RAYWHITE);
            DrawText(TextFormat("Dynamic Speed: %.1f km/h", dynSpeed), hudX + 15, hudY + 130, 13, YELLOW);
            DrawText(TextFormat("Vehicles:      %d / %.0f", rd.currentVehicles, rd.capacity), hudX + 15, hudY + 150, 13, GetMetroHeatColor(rd.Congestion()));
            DrawText(TextFormat("Queue Depth:   %d vehicles", rd.queueCount), hudX + 15, hudY + 170, 13, ORANGE);
        }
        else {
            if (currentView == VIEW_NYC_METRO_CITY) {
                DrawText("NETWORK RADAR SCANNER", hudX + 15, hudY + 12, 15, LIME);
                DrawText("NYC METRO ARTERIAL GRID", hudX + 15, hudY + 36, 16, GetMTAColorYellow());
                DrawLine(hudX + 15, hudY + 58, hudX + hudW - 15, hudY + 58, Fade(DARKGRAY, 0.5f));
                DrawText("Hover over any vehicle, station", hudX + 15, hudY + 70, 13, RAYWHITE);
                DrawText("or subway track for telemetry.", hudX + 15, hudY + 90, 13, RAYWHITE);
                DrawText("3D Tactical Overview: Bottom-Left", hudX + 15, hudY + 115, 13, SKYBLUE);
                DrawText("Press [E] for EMS Ambulance", hudX + 15, hudY + 135, 13, RED);
                DrawText("[PRESS F1 / ESC FOR SETTINGS]", hudX + 15, hudY + 175, 12, Fade(GRAY, 0.8f));
            } else if (currentView == VIEW_REALM_OVERWORLD) {
                DrawText("SYSTEM OVERVIEW", hudX + 15, hudY + 12, 15, LIME);
                DrawText("REALM TRANSIT NETWORK", hudX + 15, hudY + 36, 16, GOLD);
                DrawLine(hudX + 15, hudY + 58, hudX + hudW - 15, hudY + 58, Fade(DARKGRAY, 0.5f));
                DrawText("4 Mythical Realms Connected", hudX + 15, hudY + 70, 13, RAYWHITE);
                DrawText("Inter-Planar Highways Active", hudX + 15, hudY + 92, 13, SKYBLUE);
                DrawText("Click a Realm Node or press [1-4]", hudX + 15, hudY + 114, 13, YELLOW);
                DrawText("to enter Metropolitan City Planning", hudX + 15, hudY + 136, 13, LIME);
            } else {
                DrawText("SYSTEM OVERVIEW", hudX + 15, hudY + 12, 15, LIME);
                DrawText(fantasyCityNames[selectedCityIdx].c_str(), hudX + 15, hudY + 36, 16, GetFantasyCityColor(fantasyCityNames[selectedCityIdx]));
                DrawLine(hudX + 15, hudY + 58, hudX + hudW - 15, hudY + 58, Fade(DARKGRAY, 0.5f));
                DrawText(TextFormat("City LOS:      %s", getLOSName(activeSim.cityReport.optimizedLOS).c_str()), hudX + 15, hudY + 70, 14, GREEN);
                DrawText(TextFormat("Avg Delay:     %.1fs", activeSim.cityReport.optimizedAvgDelay), hudX + 15, hudY + 92, 13, RAYWHITE);
                DrawText(TextFormat("Delay Gain:    -%.1f%%", activeSim.cityReport.delayReductionPct), hudX + 15, hudY + 114, 13, LIME);
                DrawText(TextFormat("CO2 Saved:     -%.1f%%", activeSim.cityReport.co2ReductionPct), hudX + 15, hudY + 136, 13, SKYBLUE);
                DrawText("[PRESS ESC TO RETURN TO REALM]", hudX + 15, hudY + 175, 12, Fade(GRAY, 0.8f));
            }
        }

        // --- 4.5 SCREEN SPACE VEHICLE & SIGNAL LEGEND HUD ---
        DrawVehicleAndSignalLegend(screenWidth, screenHeight);

        // Center-top paused banner
        if (isPaused) {
            int bannerW = 380;
            int bannerH = 34;
            int bannerX = (int)centerX - bannerW / 2;
            int bannerY = 20;
            DrawRectangle(bannerX, bannerY, bannerW, bannerH, Fade(BLACK, 0.85f));
            DrawRectangleLines(bannerX, bannerY, bannerW, bannerH, ORANGE);
            DrawText("PAUSED [PRESS SPACE TO RESUME]", bannerX + 22, bannerY + 8, 16, ORANGE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}