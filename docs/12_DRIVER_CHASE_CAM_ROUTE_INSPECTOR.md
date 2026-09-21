# Feature 12: Driver Chase-Cam & Route Breadcrumb Visualizer

## 1. Overview & Summary
Microscopic traffic simulators must provide macroscopic network visibility and individual driver-level inspection. 
Feature 12 introduces a **Driver Chase-Cam & Route Breadcrumb Visualizer** in Raylib GUI:
1. **Camera Tracking**: Smoothly locks and centers the 2D camera viewport onto any selected en-route vehicle, automatically gliding as the vehicle navigates roundabouts, toll plazas, and motorway interchanges.
2. **Glowing Route Breadcrumb Trail**: Renders the complete planned path polyline across intermediate city nodes with waypoint beacons, displaying past progress and upcoming maneuvers.
3. **Cockpit Telemetry HUD Card**: Displays real-time speed, estimated arrival time, fuel burn / battery SoC %, M-Tag electronic toll registration, and origin/destination data.

```
       [En-Route Vehicle Selected: Key [T] or Mouse Left-Click]
                                 |
                                 v
        +-------------------------------------------------+
        |   Camera Target Glides to Vehicle Coordinates   |
        |   Camera.target = Lerp(target, carPos, 0.12)    |
        |   Camera.zoom = Lerp(zoom, 1.9, 0.03)           |
        +-------------------------------------------------+
                                 |
                                 v
        +-------------------------------------------------+
        |   Route Polyline Rendered Across Planned Path   |
        |   (Pos -> Waypoint 1 -> Waypoint 2 -> Dest)     |
        +-------------------------------------------------+
                                 |
                                 v
        +-------------------------------------------------+
        |   Top-Right CHASE-CAM TELEMETRY Cockpit Card    |
        |   - Instant Speed (km/h)                        |
        |   - Fuel Consumed (L) / Battery SoC (%)         |
        |   - M-Tag RFID vs Cash Lane Status              |
        |   - Estimated Time Remaining                    |
        +-------------------------------------------------+
```

---

## 2. Technical Details & Camera Dynamics

### 2.1 Smooth Viewport Tracking
Camera panning uses frame-rate independent linear interpolation (`Vector2Lerp`) to track the vehicle smoothly without jitter:
$$\vec{P}_{\text{cam}}(t + \Delta t) = \text{Lerp}(\vec{P}_{\text{cam}}(t), \; \vec{P}_{\text{veh}}(t), \; 0.12)$$
$$\text{Zoom}(t + \Delta t) = \text{Lerp}(\text{Zoom}(t), \; 1.9, \; 0.03)$$

### 2.2 Interactive Controls
| Control | Action |
|---|---|
| **Key `[T]`** | Cycles camera focus to the next en-route active vehicle |
| **Mouse Left-Click** | Raycasts and locks chase camera onto clicked vehicle |
| **Key `[ESC]` / Right-Drag** | Breaks vehicle lock, returning to free-pan manual camera mode |

### 2.3 Route Breadcrumb Rendering
For vehicle $V$ with planned path $P = [n_1, n_2, \dots, n_k]$:
- Line segments are drawn connecting $\vec{P}_{\text{veh}} \rightarrow \vec{P}(n_1) \rightarrow \dots \rightarrow \vec{P}(n_k)$ in high-contrast neon lime (`Fade(LIME, 0.85)`).
- Circular waypoint beacons are rendered at each intermediate junction.
- A pulsating targeting reticle is drawn centered on the tracked vehicle:
  $$R_{\text{target}} = 9.0 + 2.5 \sin(8.0 \cdot t_{\text{time}})$$

---

## 3. Implementation Details
- [`Graphics.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Graphics.cpp):
  - State variable `trackedCarId`.
  - Mouse raycast and hotkey cycling.
  - Camera centering logic and polyline breadcrumb drawing.
  - Cockpit telemetry card in top-right HUD overlay.
- [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h):
  - Exposes `path`, `selected_path`, `currentSpeed`, `fuelConsumedLiters`, `batterySoCPercent`, and `hasMTag`.

---

## 4. Dependencies
- [`Graphics.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Graphics.cpp): Main rendering and input handling.
- [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h): Data source for vehicle kinematics and telemetry.
- [`RoadDetails.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/RoadDetails.h): Edge geometry.
