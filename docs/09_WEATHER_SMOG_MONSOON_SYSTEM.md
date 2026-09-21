# Feature 9: Pakistani Seasonal Weather Presets (Winter Smog, Monsoon Floods, Dense Fog)

## 1. Overview & Summary
Seasonal environmental conditions in Pakistan profoundly affect road transportation safety, vehicular speeds, and network throughput. 
During winter months (November–January), **severe atmospheric smog (AQI > 500)** blankets the Lahore-Multan plains, and **dense radiation fog** reduces visibility to under 50 meters on the M-2 and M-3 motorways. During summer (July–August), **Monsoon downpours** lead to localized flooding, aquaplaning, and sharp reductions in highway capacity.

Feature 9 implements a **macroscopic meteorological simulation system** that dynamically adjusts road tire-pavement friction coefficients, advisory speed limits, and effective highway capacity across the transport network.

```
       +-----------------------------------------------------------+
       |             Pakistani Weather Preset Selector             |
       +-----------------------------------------------------------+
               |                    |                   |
               v                    v                   v
        [Monsoon Rain]        [Winter Smog]      [Dense Motorway Fog]
               |                    |                   |
       Grip: 0.72            Grip: 0.90          Grip: 0.85
       Cap: 85%              Cap: 75%            Cap: 50%
       SpeedCap: 80%         SpeedCap: 65%       SpeedCap: 45%
               \                    |                   /
                \                   |                  /
                 v                  v                 v
       +-----------------------------------------------------------+
       |   RoadDetails Weight Recalculated -> Dynamic Detours      |
       +-----------------------------------------------------------+
```

---

## 2. Weather Modes & Physical Impact Parameters

| Weather Preset | Meteorological Description | Friction Grip ($\mu$) | Capacity Factor ($C_{\text{factor}}$) | Speed Cap ($v_{\text{eff}}$) |
|---|---|---|---|---|
| `WEATHER_CLEAR` | Baseline sunny/dry conditions | $1.00$ | $1.00$ ($100\%$) | $1.00 \times v_{\text{max}}$ |
| `WEATHER_RAIN` | Monsoon rainstorms, slick pavement | $0.72$ | $0.85$ ($85\%$) | $0.80 \times v_{\text{max}}$ |
| `WEATHER_SMOG` | High particulate smog (AQI 400–600) | $0.90$ | $0.75$ ($75\%$) | $0.65 \times v_{\text{max}}$ |
| `WEATHER_DENSE_FOG` | Motorway zero-visibility fog (<50m) | $0.85$ | $0.50$ ($50\%$) | $0.45 \times v_{\text{max}}$ |

### 2.1 Impact on Travel Time & BPR Link Cost
Under adverse weather, both the free-flow travel time and the congested delay are magnified:
$$v_{\text{eff}} = \min(v_{\text{max}}, \; v_{\text{weatherCap}})$$
$$t_{\text{free}} = \frac{L}{v_{\text{eff}}}$$
$$\text{Cost}(e) = t_{\text{free}} \left[ 1 + \alpha \left(\frac{N_{\text{veh}}}{C \times C_{\text{factor}}}\right)^\beta \right] + \text{Penalty}$$

Because $C_{\text{factor}} < 1.0$, the effective saturation ratio skyrockets during fog, triggering dynamic rerouting detours across alternative links.

---

## 3. Implementation Details

### 3.1 RoadDetails Weather State (`RoadDetails.h`)
In [`RoadDetails.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/RoadDetails.h):
- `enum WeatherCondition`: Defines `WEATHER_CLEAR`, `WEATHER_RAIN`, `WEATHER_SMOG`, `WEATHER_DENSE_FOG`.
- `frictionGrip`, `weatherSpeedCap`, `weather`.
- `setWeather(WeatherCondition w)`: Recomputes friction, capacity factor, speed limit, and calls `NonIdealtime()`.
- `getEffectiveMaxSpeed()`: Dynamically caps link velocity.

### 3.2 Network-Wide Weather Modulation (`Manger.h` & `Simulator.h`)
In [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h):
- `setGlobalWeather(WeatherCondition w)`: Iterates through all vertices and edges in the graph, updates their weather configuration, and executes `ShortestPath()` to force all en-route vehicles to recalculate optimal routes given new travel times.

---

## 4. Dependencies
- [`RoadDetails.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/RoadDetails.h): Contains `WeatherCondition` enum and link degradation functions.
- [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h): Global weather dispatcher and route recalculation engine.
- [`Simulator.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Simulator.h): Exposes `setWeather()` to client applications.
- [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp): Verified in Automated Test 23.
