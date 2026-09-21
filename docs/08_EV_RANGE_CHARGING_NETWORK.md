# Feature 8: Electric Vehicle (EV) Fleet & Highway Fast-Charging Network

## 1. Overview & Summary
As part of the National Electric Vehicle Policy (NEVP) and modernization efforts in Pakistan, the adoption of Electric Vehicles (EVs) on inter-city motorways is accelerating. 
Feature 8 integrates **battery electrochemistry, range consumption, and fast-charging infrastructure** into the microscopic traffic simulator.

Key capabilities:
1. **Zero Tailpipe Emissions**: EVs emit $0.0\text{ g } CO_2/\text{km}$, significantly driving down network carbon footprint.
2. **Dynamic Battery State of Charge (SoC %)**: Models battery discharge based on distance traveled, vehicle mass, and cruising speed.
3. **National Motorway Fast-Charging Infrastructure**: Strategically placed charging hubs (at Sukkur, Multan, Lahore, Islamabad) where low-battery EVs autonomously recharge during inter-city transit.

```
+-------------------------------------------------------------+
|            Electric Vehicle (EV) Powertrain Model           |
+-------------------------------------------------------------+
                              |
                     [Longitudinal Motion]
                              |
                              v
                DistTraveled = (Speed * dt) / 1000  (km)
                EnergyKWh = DistTraveled * 0.18 kWh/km
                              |
                              v
                BatterySoC% -= (EnergyKWh / CapacityKWh) * 100%
                              |
                              v
                  [Is Battery SoC < 25%?]
                              |
                 +------------+------------+
                 |                         |
               [YES]                      [NO]
                 |                         |
                 v                         v
       [At Charging Hub?]            Continue Driving
                 |
                 v
      Recharge Battery (+75% SoC)
```

---

## 2. Mathematical Models & Electrochemical Formulas

### 2.1 Battery Energy Consumption
For an Electric Vehicle traveling at speed $v$ (m/s) over time $\Delta t$ (seconds):
$$\Delta d = \frac{v \times \Delta t}{1000} \quad (\text{km})$$
$$E_{\text{consumed}} = \Delta d \times R_{\text{electric}} \quad (\text{kWh})$$
Where:
- $R_{\text{electric}} = 0.18\text{ kWh/km}$ (Standard EV efficiency, e.g. BYD / Deepal / MG4).
- $C_{\text{battery}} = 65.0\text{ kWh}$ (Usable battery pack capacity).

### 2.2 State of Charge (SoC) Dynamics
$$\Delta \text{SoC} = \left( \frac{E_{\text{consumed}}}{C_{\text{battery}}} \right) \times 100\%$$
$$\text{SoC}(t + \Delta t) = \max\left(0\%, \; \text{SoC}(t) - \Delta \text{SoC}\right)$$

### 2.3 Fast-Charging Recovery
When an EV reaches a city or service area with installed charging infrastructure with $\text{SoC} < 25\%$:
$$\text{SoC}_{\text{new}} = \min\left(100\%, \; \text{SoC} + 75\%\right)$$

---

## 3. Implementation Details

### 3.1 Vehicle Electrification (`vehicle.h`)
In [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h):
- Enum value: `VEHICLE_EV` added to `VehicleType`.
- Fields: `batteryCapacityKWh` (65 kWh), `batterySoCPercent` (100%), `electricConsumptionRateKWhPerKm` (0.18 kWh/km).
- Methods: `isEV()`, `isLowBattery()`, `rechargeBattery(float amountPercent)`.
- `updateFuelAndEmissions()`: If vehicle is EV, $CO_2$ generated is strictly $0.0\text{ kg}$, and battery SoC is depleted proportional to distance.

### 3.2 Highway Charging Stations (`Manger.h` & `Simulator.h`)
In [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h):
- `vector<t> chargingStations`: List of nodes equipped with 120kW DC fast chargers.
- `addChargingStation(t node)` & `hasChargingStation(t node)`.
- Recharging trigger executed in `arrivalAtIntersection()` whenever an EV with $\text{SoC} < 25\%$ arrives at a charging node.

---

## 4. Dependencies
- [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h): Battery storage and SoC consumption math.
- [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h): Network charging station registry and auto-recharging logic.
- [`Simulator.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Simulator.h): Initial fleet generation with 15% EV penetration.
- [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp): Verified in Automated Test 22.
