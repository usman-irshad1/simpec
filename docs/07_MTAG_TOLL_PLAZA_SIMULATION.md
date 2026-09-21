# Feature 7: M-Tag Electronic Toll Plazas & Cash-Lane Queuing Delay

## 1. Overview & Summary
On Pakistan's National Motorway and Highway network (e.g. M-1, M-2, M-3, M-5, and M-9), toll plazas represent major bottleneck friction points. The National Highway Authority (NHA) mandates the **M-Tag RFID automated electronic toll system** to minimize queueing delays at entry/exit interchanges.

Feature 7 simulates toll plaza physics by modeling:
1. **M-Tag Express Electronic Lanes**: Dedicated high-speed RFID readers allowing continuous or minimal barrier transit delay ($\sim 1.5\text{ seconds}$).
2. **Manual Cash Booths**: Manual ticket generation, cash exchange, change return, and gate opening, causing significant delay ($\sim 18.0\text{ seconds}$) and queue accumulation.
3. **Revenue Collection & Commercial Toll Auditing**: Real-time aggregation of Pakistani Rupee (PKR) revenues collected per motorway plaza.

```
                    +------------------------------------+
                    |  Vehicle Enters Motorway Toll Link |
                    +------------------------------------+
                                      |
                         [Is Toll Plaza Enabled?]
                                      |
                         +------------+------------+
                         |                         |
                    [Has M-Tag]              [No M-Tag / Cash]
                         |                         |
                         v                         v
               RFID Electronic Gate               Manual Cash Booth
                 Delay: 1.5s                      Delay: 18.0s
                         \                         /
                          \                       /
                           v                     v
              +-----------------------------------------------+
              |   Toll Fee Paid (PKR) & Vehicle Timespent     |
              |   Incremented; Plaza Metrics Updated          |
              +-----------------------------------------------+
```

---

## 2. Mathematical Formulation & Plaza Delay Dynamics

### 2.1 Plaza Delay Function
For a road edge $(u, v)$ equipped with a toll plaza ($T_{\text{rate}}$):

$$\Delta t_{\text{toll}} = \begin{cases}
1.5 \text{ seconds} & \text{if Vehicle possesses active M-Tag} \\
18.0 \text{ seconds} & \text{if Vehicle uses manual cash lane}
\end{cases}$$

### 2.2 Financial Revenue Accumulation
Total toll revenue collected across plaza $P$ across the simulation timeline:
$$\text{Revenue}_P = \sum_{k=1}^{N_{\text{transits}}} T_{\text{rate}}(P) \quad (\text{PKR})$$

### 2.3 Penetration Rate
In the simulation model, vehicles are assigned M-Tag capability with a realistic **70% penetration rate** based on current NHA adoption statistics:
$$P(\text{hasMTag} = \text{true}) = 0.70$$

---

## 3. Implementation Details

### 3.1 RoadDetails Configuration (`RoadDetails.h`)
In [`RoadDetails.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/RoadDetails.h):
- `hasTollPlaza`: Boolean flag indicating toll plaza presence.
- `tollRatePKR`: Base toll fee in PKR (e.g. PKR 120 - 150).
- `mTagVehiclesServed` & `cashVehiclesServed`: Lane throughput counters.
- `processToll(bool hasMTag)`: Calculates exact transit delay and increments revenue.

### 3.2 Vehicle Toll Experience (`vehicle.h`)
In [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h):
- `hasMTag`: Boolean flag assigned upon vehicle instantiation.
- `tollDelayPaid`: Cumulative time spent waiting at toll barriers.
- `payToll(float delay, float fee)`: Increments travel time and recorded toll delays.

### 3.3 Network Integration (`Manger.h` & `Simulator.h`)
- Plaza checkpoints are checked upon road entry in `entrance()`, `entraingfromQueetoEdge()`, and `arrivalAtIntersection()`.
- Default toll plazas deployed on:
  - Lahore $\leftrightarrow$ Islamabad (M-2 Motorway)
  - Karachi $\leftrightarrow$ Sukkur (M-9 Motorway)
  - Sukkur $\leftrightarrow$ Multan (M-5 Motorway)

---

## 4. Dependencies
- [`RoadDetails.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/RoadDetails.h): Implements `processToll`.
- [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h): Holds M-Tag status and `payToll`.
- [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h): Manages network-wide toll throughput and revenues.
- [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp): Verified in Automated Test 21.
