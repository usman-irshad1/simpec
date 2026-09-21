# Feature 10: 24-Hour Day/Night Simulation Clock & Ambient Headlights

## 1. Overview & Summary
Real-world traffic flows are fundamentally diurnal: demand peaks during the morning rush hour (7:00–9:30 AM) and evening rush hour (5:00–8:00 PM), while plummeting during deep night (midnight to 4:00 AM) when freight trucks dominate long-distance corridors.

Feature 10 implements a **continuous 24-hour astronomical simulation clock**, smooth daylight/dusk/night ambient darkness transitions, and vehicular illumination (headlights) for low-light conditions.

```
       00:00 (Midnight)                         12:00 (Noon)
     [Deep Night: 85% Dark]                [Full Daylight: 0% Dark]
             ^                                         ^
             |                                         |
     05:00 - 07:00 (Dawn)                     18:00 - 20:00 (Dusk)
  [Darkness fades 85% -> 0%]                [Darkness rises 0% -> 85%]
```

---

## 2. Mathematical Modeling of Diurnal Lighting

### 2.1 Astronomical Darkness Function
The ambient darkness factor $D(h) \in [0.0, 0.85]$ as a function of the continuous simulation hour $h \in [0.0, 24.0)$:

$$D(h) = \begin{cases}
0.0 & \text{if } 7.0 \le h \le 18.0 \quad (\text{Daylight}) \\
\frac{h - 18.0}{2.0} \times 0.85 & \text{if } 18.0 < h \le 20.0 \quad (\text{Dusk transition}) \\
0.85 & \text{if } 20.0 < h \text{ or } h < 5.0 \quad (\text{Night}) \\
\frac{7.0 - h}{2.0} \times 0.85 & \text{if } 5.0 \le h < 7.0 \quad (\text{Dawn transition})
\end{cases}$$

### 2.2 Modulo Clock Rollover
$$\text{clock}_{\text{new}} = (\text{clock}_{\text{old}} + \Delta t_{\text{hours}}) \pmod{24.0}$$

---

## 3. Implementation Details

### 3.1 Manager Diurnal State Machine (`Manger.h`)
In [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h):
- `float simClockHours`: Floating-point hour clock initialized at `8.0f` (08:00 AM).
- `advanceClock(float dtMinutes)`: Steps the clock forward per simulation tick.
- `setSimClock(float hour)`: Directly sets the simulation hour.
- `isNight()`: Returns `true` when $h < 6.0$ or $h > 19.5$.
- `getAmbientDarkness()`: Returns smooth interpolation factor $D(h)$.

### 3.2 Visual Atmosphere Rendering (`Graphics.cpp`)
In [`Graphics.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Graphics.cpp):
- Nighttime ambient overlay drawn over world space using `Fade(BLACK, darkness)`.
- Active vehicles at night illuminate the pavement with small forward-projected warm headlight cones.
- Ops Center sidebar displays the formatted military time (e.g. `14:45 [DAYLIGHT]` or `01:30 [NIGHT]`).

---

## 4. Dependencies
- [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h): Maintains `simClockHours` and darkness interpolation.
- [`Graphics.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Graphics.cpp): Renders ambient night shadows and headlights.
- [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp): Verified in Automated Test 24.
