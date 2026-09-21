# Feature 6: Real-Time Carbon Emissions ($CO_2$, $NO_x$) & Fuel Burn Tracker

## 1. Overview & Summary
Urban congestion and motorway bottlenecks in Pakistan (such as the M-2 and N-5 corridors) result in millions of liters of wasted fuel and severe atmospheric pollution. 
Feature 6 introduces an **environmental physics and chemical combustion model** that dynamically tracks fuel consumption, carbon dioxide ($CO_2$) emissions, and fuel wasted specifically in queues/congestion on a per-vehicle, per-road, and network-wide level.

```
+-------------------------------------------------------------+
|                 Vehicle Movement / Queue State              |
+-------------------------------------------------------------+
               |                               |
       [Speed > 0.5 m/s]              [Idle / In Queue]
               |                               |
               v                               v
    Cruising Fuel Burn              Idle Fuel Burn Rate
   (Liters/km * DistKm)             (Liters/sec * dtSec)
               \                               /
                \                             /
                 v                           v
     +--------------------------------------------------+
     |   Total Fuel Consumed & Fuel Wasted in Jam (L)   |
     +--------------------------------------------------+
                          |
                          v
         * CO2 Factor (Gasoline: 2.31 kg/L, Diesel: 2.68 kg/L)
                          |
                          v
     +--------------------------------------------------+
     |      Real-Time CO2 Atmospheric Footprint (kg)    |
     +--------------------------------------------------+
```

---

## 2. Mathematical Models & Combustion Equations

### 2.1 Fuel Consumption Regimes
Fuel consumption is partitioned into two distinct physical domains:
1. **Idle/Congestion Domain ($v < 0.5\text{ m/s}$)**:
   $$\text{Fuel}_{\text{idle}} = R_{\text{idle}} \times \Delta t$$
   Where $R_{\text{idle}}$ is the idle fuel rate in Liters per second.
   Any fuel burned during standstill is logged as **wasted fuel in traffic congestion** ($\text{Fuel}_{\text{wasted}}$).

2. **Cruising Domain ($v \ge 0.5\text{ m/s}$)**:
   $$\Delta d = \frac{v \times \Delta t}{1000} \quad (\text{km})$$
   $$\text{Fuel}_{\text{cruise}} = \Delta d \times R_{\text{cruise}}$$
   Where $R_{\text{cruise}}$ is the consumption rate in Liters per km.

### 2.2 Vehicle Fleet Parameters
| Vehicle Type | Powertrain / Fuel | Idle Rate ($R_{\text{idle}}$) | Cruise Rate ($R_{\text{cruise}}$) | $CO_2$ Factor ($\text{kg } CO_2/\text{L}$) |
|---|---|---|---|---|
| **Passenger Car** | Gasoline | $0.00025\text{ L/s}$ ($\sim 0.9\text{ L/h}$) | $0.070\text{ L/km}$ ($7.0\text{ L}/100\text{km}$) | $2.31$ |
| **Bus** | Diesel | $0.000556\text{ L/s}$ ($\sim 2.0\text{ L/h}$) | $0.220\text{ L/km}$ ($22.0\text{ L}/100\text{km}$) | $2.68$ |
| **Heavy Truck** | Heavy Diesel | $0.000694\text{ L/s}$ ($\sim 2.5\text{ L/h}$) | $0.280\text{ L/km}$ ($28.0\text{ L}/100\text{km}$) | $2.68$ |
| **Electric Vehicle (EV)** | Battery / Electric | $0.0\text{ L/s}$ | $0.0\text{ L/km}$ ($0.18\text{ kWh/km}$) | $0.00$ (Zero Tailpipe) |

### 2.3 Stoichiometric Carbon Dioxide Emission Formula
$$\text{CO}_{2\text{ emitted}} = \text{Fuel Consumed} \times C_{\text{factor}} \quad (\text{kg})$$

---

## 3. Implementation Details

### 3.1 Vehicle Microscopic Tracking (`vehicle.h`)
In [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h):
- Added fields: `fuelConsumedLiters`, `co2EmittedKg`, `fuelWastedInJamLiters`.
- Added method `updateFuelAndEmissions(float dtSeconds, bool isIdleOrQueued, float speedMps)`.

### 3.2 System Telemetry & Metrics (`Manger.h`)
In [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h):
- Cumulative counters: `totalFuelConsumedLiters`, `totalCO2EmittedKg`, `totalFuelWastedInJamLiters`.
- Logged directly to `performance_metrics.txt` and exported via getters for HUD visualization.

---

## 4. Dependencies
- [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h): Implements vehicle emissions physics.
- [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h): Aggregates system metrics upon vehicle arrival and loop ticks.
- [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp): Verified in Automated Test 20.
