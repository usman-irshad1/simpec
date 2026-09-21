# Adaptive Traffic Simulator — System Context & Architecture Documentation

## 1. Executive Summary

The **Adaptive Traffic Flow Optimization System** is a high-performance C++ Intelligent Transportation System (ITS) simulation. It models urban and intercity road networks, dynamically simulates traffic congestion using the **Bureau of Public Roads (BPR)** function, optimizes travel routes using **Dynamic Dijkstra Shortest Path routing**, and coordinates intersection signals using an **Adaptive Signal Controller (Longest Queue First with starvation avoidance)**.

The system is equipped with a 2D real-time graphical dashboard powered by **Raylib 5.5**, visually displaying traffic density heatmaps, individual vehicle progress, queue lengths, and network performance indicators (P95 latency, delay, throughput, and system cost).

---

## 2. System Architecture & Component Diagram

```
+-----------------------------------------------------------------------------------+
|                                  USER / RAYLIB GUI                                |
|       (Graphics.cpp - Metro Dashboard, Heatmaps, Vehicle Interpolation, Stats)     |
+------------------------------------------+----------------------------------------+
                                           | Drives loop / Polls state
                                           v
+-----------------------------------------------------------------------------------+
|                             SIMULATOR (Simulator.h)                               |
|        - Owns Graph and Manager instances                                         |
|        - setupNetwork(): Defines 11 Pakistani cities, highways, motorways, bypasses|
|        - addMassiveTraffic(): Generates randomized origin-destination travel demand|
+-------------------+--------------------------------------+------------------------+
                    |                                      |
                    v                                      v
+--------------------------------------+   +----------------------------------------+
|          MANAGER (Manger.h)          |   |           GRAPH (Graph.h)              |
| - Vehicle lifecycle (Queue/EnRoute)  |   | - Fixed-size Adjacency List:           |
| - entrance() / reached() transitions |   |   Gnode<t> array[size]                 |
| - arrivalAtIntersection()            |   | - Dijkstra Routing with dynamic BPR    |
| - updateSignals(): LQF controller    |   | - Prim's MST, BFS, DFS                 |
| - injectSinusoidalTraffic()          |   | - AverageRush(), total_System_Cost()   |
| - Metrics (CSV, txt, rolling window) |   +-------------------+--------------------+
+-------------------+------------------+                       |
                    |                                          |
                    v                                          v
+--------------------------------------+   +----------------------------------------+
|          VEHICLE (vehicle.h)         |   |         ROAD DETAILS (RoadDetails.h)   |
| - id, source, dest, current          |   | - length, max_speed, capacity          |
| - path (std::list), selected_path    |   | - currentVehicles, queueCount          |
| - state (-1=Init, 0=Queued,          |   | - BPR calculateWeight(), NonIdealtime()|
|          1=EnRoute, 2=Arrived)       |   | - DischargeAllowed() backpressure flow |
| - timeRemaining, timespent           |   | - TrafficSignal light instance         |
+--------------------------------------+   +-------------------+--------------------+
                                                               |
                                                               v
                                           +----------------------------------------+
                                           |      TRAFFIC SIGNAL (TrafficSignal.h)  |
                                           | - greentimer, redtimer, min/max times  |
                                           | - starvationCost() = redtimer * 3.5    |
                                           | - canSwitch(): Queue vs starvation     |
                                           +----------------------------------------+
```

---

## 3. Detailed File-by-File Analysis

### 3.1 [`README.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/README.md)
* **Purpose**: Project documentation, conceptual background, and optimization problem formulation.
* **Core Concepts**:
  * **Objective Function**:
    $$\min \sum_t \sum_{(i,j)} \left[ \alpha Q_{ij}(t) + \beta \left(\frac{f_{ij}(t)}{c_{ij}}\right)^2 \right]$$
    Balances waiting time ($Q_{ij}$) and network saturation ($f_{ij} / c_{ij}$).
  * **Congestion Model**: Bureau of Public Roads (BPR) delay formulation.
  * **Routing**: Congestion-aware Dijkstra rerouting.
  * **Traffic Signal Control**: Longest Queue First (LQF) heuristic.
  * **Evaluation Metrics**: Network utilization, cumulative delay, throughput, P95 travel times.

---

### 3.2 [`TrafficSignal.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/TrafficSignal.h)
* **Purpose**: Models an individual edge traffic signal controller and switching policy.
* **Key Members**:
  * `greentimer`: Elapsed time the signal has been continuously green.
  * `redtimer`: Elapsed time the signal has been continuously red.
  * `minGreenTime` ($2.0$ ticks): Minimum time a light must remain green before preemption.
  * `maxGreenTime` ($8.0$ ticks): Maximum green duration before mandatory phase termination.
  * `queueThreshold` ($3.0$): Queue threshold.
  * `pa = 10.0`, `pb = 7.0`: Objective function penalty constants.
* **Methods**:
  * `starvationCost()`: Returns `redtimer * 3.5f` to penalize starving red approaches.
  * `canSwitch(int currentQueue, int maxOtherQueue)`:
    * Enforces `greentimer >= minGreenTime`.
    * If `greentimer >= maxGreenTime`, returns `true`.
    * Evaluates bottleneck ratio `(float)currentQueue / maxOtherQueue`.
    * Returns `true` if current queue is smaller than competing queue or starvation penalty exceeds $12.0$.

---

### 3.3 [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h)
* **Purpose**: Template data structure representing an individual vehicle agent.
* **Key Members**:
  * `int id`: Unique vehicle identifier.
  * `t source`, `t dest`, `t current`: Node coordinates (city names as `std::string`).
  * `list<t> path`: Active planned route remaining.
  * `pair<t, t> currentRoad`: Current directed edge `(u, v)`.
  * `int state`:
    * `-1`: Uninitialized.
    * `0`: Waiting in intersection queue.
    * `1`: En route traversing a road.
    * `2`: Arrived at destination.
  * `bool inQueue`: Queue membership flag.
  * `vector<t> selected_path`: Complete chronological history of visited vertices.
  * `float timespent`: Cumulative travel duration.
  * `float timeRemaining`: Countdown timer to traverse the active road segment.

---

### 3.4 [`RoadDetails.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/RoadDetails.h)
* **Purpose**: Models directed edge attributes, capacity constraints, BPR travel time equations, and traffic light integration.
* **Key Members**:
  * `length`: Road distance (km).
  * `max_speed`: Maximum speed limit (km/h).
  * `capacity`: Maximum vehicular capacity.
  * `currentVehicles`: Number of vehicles actively traversing the link.
  * `queueCount`: Vehicles backed up at the exit intersection.
  * `signalState`: `true` (Green) or `false` (Red).
  * `dischargeCapcity`: Maximum vehicles permitted to exit per tick (default 5).
  * `a = 0.5`, `b = 4.0`: BPR coefficients ($\alpha$ and $\beta$).
  * `TrafficSignal light`: Signal instance for this road.
* **Core Physics Methods**:
  * `bestTime()`: Free-flow travel time: $\frac{\text{length}}{\text{max\_speed}}$.
  * `NonIdealtime()` / `calculateWeight()`: BPR formulation:
    $$w(t) = \text{bestTime} \times \left(1 + \alpha \cdot \left(\frac{\text{currentVehicles}}{\text{capacity}}\right)^\beta\right)$$
  * `DischargeAllowed(capacity_ofnext_road, pop_of_next_road)`:
    Backpressure flow control. Calculates how many queued vehicles can enter the downstream road based on available downstream space and discharge rate.
  * `Congestion()`: Ratio $\frac{\text{currentVehicles}}{\text{capacity}}$.
  * `choosing()`: Instantaneous cost: $p_a \cdot \text{queueCount} + p_b \cdot (\text{Congestion})^2$.

---

### 3.5 [`Graph.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Graph.h)
* **Purpose**: Template graph class `Graph<t, size>` modeling the road network.
* **Internal Structures**:
  * `struct weighted`: Contains neighbor vertex index and `RoadDetails weight`.
  * `template <class t> struct Gnode`: Contains vertex label `t` and `std::list<weighted> Neighbors`.
  * `Gnode<t> array[size]`: Fixed-size array representing vertices.
* **Key Algorithms**:
  * `shortest_Path_btw2_vericex_returing_list(t source, t dest)`:
    * Dijkstra's Shortest Path algorithm using dynamically computed edge weights from `it->weight.NonIdealtime()`.
    * Reconstructs the optimal vertex sequence.
  * `minimumSpanningtree()`: Prim's algorithm for MST construction.
  * `bfs()` / `dfs()`: Breadth-first and depth-first traversals.
  * `AverageRush()`: Computes global network saturation:
    $$\text{AverageRush} = \frac{1}{|E|} \sum_{e \in E} \frac{\text{vehicles}_e}{\text{capacity}_e}$$
  * `total_System_Cost()`: Network-wide sum of all edge `choosing()` penalties.
  * `getEdges(t data, list<RoadDetails*> edges)`: Gathers all inbound edges terminating at intersection `data`.

---

### 3.6 [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h)
* **Purpose**: Core simulation orchestration engine (`Manager<t, size>`).
* **Key Responsibilities**:
  * **Vehicle Storage**: `std::list<vehicle<t>> array_of_vehicles`.
  * **Simulation Loop (`physics()`)**: Headless tick execution.
  * **Entrance Flow (`entrance()` & `entraingfromQueetoEdge()`)**:
    Transitions vehicles from origin and intersection queues onto road edges subject to green lights and backpressure discharge limits.
  * **Arrival at Intersections (`arrivalAtIntersection()`)**:
    Processes vehicles finishing an edge, transitions them into the next road's queue or directly onto the road if green and capacity permits.
  * **Dynamic Rerouting (`ShortestPath()`)**:
    Runs Dijkstra for all active vehicles en route. If a less congested path is discovered, updates the vehicle route and increments `totalReroutes`.
  * **Adaptive Signal Control (`updateSignals()`)**:
    Scans every intersection, evaluates all incoming edges, computes total cost:
    $$\text{Cost} = \text{choosing}() + \text{starvationCost}()$$
    Switches the green signal to the worst-case (highest cost) incoming road if the current green phase permits switching.
  * **Traffic Waves (`injectSinusoidalTraffic(currentTime)`)**:
    Injects periodic traffic bursts using a sine wave:
    $$\text{VehiclesToAdd} = \lfloor 10 \cdot (\sin(0.01 \cdot t) + 1) + 5 \rfloor$$
  * **Metrics & Analytics**:
    Maintains arrival counts, free-flow vs actual travel time, rolling average travel times, P95 latency, and outputs to `metrics.csv`, `performance_metrics.txt`, and `car_timings.txt`.

---

### 3.7 [`Simulator.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Simulator.h)
* **Purpose**: High-level wrapper class (`Simulator<t, size>`) that instantiates the network graph and manager.
* **Key Components**:
  * `setupNetwork()`: Constructs the Pakistani highway network with 11 nodes:
    * Karachi, Sukkur, Quetta, DG Khan, Multan, Lahore, Islamabad, Faisalabad, Peshawar, Gujranwala, Sialkot.
    * Configures link lengths, speeds (90–120 km/h), capacities (40–110 vehicles), and BPR $\alpha/\beta$.
  * `addMassiveTraffic()`: Injects randomized travel demand across the 11 cities.
  * `run()`: Executes headless simulation.

---

### 3.8 [`Graphics.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Graphics.cpp) & [`Graphics.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Graphics.h)
* **Purpose**: Interactive GUI application and dashboard using Raylib 5.5.
* **Key Features**:
  * **Real Pakistan Geographic GPS Projection**: Projects all 11 Pakistani cities using calibrated geodetic coordinates ($111\text{ km/deg}$ lat, $96\text{ km/deg}$ lon at $30^\circ\text{N}$) with aspect-ratio preservation.
  * **Dual Layout Morphing**: Pressing `[G]` smoothly interpolates all city nodes and road corridors between the Real Pakistan GPS Map and the Multan Radial Hub Topology using `Vector2Lerp`.
  * **Interactive 2D Camera (Pan & Zoom)**: Raylib `Camera2D` with mouse wheel zoom centered at cursor ($0.35\times$ to $3.5\times$), right-click / middle-click drag pan, and `[R]` camera reset.
  * **Simulation Playback Controls**: Spacebar Play/Pause, speed multipliers ($1\times, 2\times, 5\times, 10\times$ via `[1]`, `[2]`, `[3]`, `[4]`), and single-step tick advance (`[N]`).
  * **Interactive Hover Inspector HUD**: Real-time raycasting using `DistanceToSegment()`. Hovering a city node displays approaches, queue depths, and signal colors; hovering a highway displays length, free-flow vs BPR dynamic speed, vehicle count / capacity (utilization %), queue depth, blockage status, and exit signal.
  * **In-Engine Telemetry Sparkline Waveform**: Rolling FIFO buffer rendering real-time network congestion rush waveforms with guidelines and percentage indicators directly in the sidebar.
  * **Visual Vehicle Taxonomy**: Real-time rendering of distinct vehicle categories (Gold cars, Skyblue heavy trucks, Purple buses, flashing siren Red/White emergency ambulances).
  * **Dynamic Incidents**: Blocked roads rendered in dark grey with maroon warnings; interactive hotkeys (`[B]` block/unblock Karachi-Lahore, `[E]` dispatch ambulance, `[P]` commuter rush).

---

### 3.9 Supporting & Staging Files
* [`Header1.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Header1.h): Umbrella header forwarding includes to the modular headers.
* [`Manager.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manager.h): Clean forwarding header to `Manger.h`.
* [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp): Automated 14-test verification suite covering Phases 1 through 4.
* [`packages.config`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/packages.config): NuGet configuration specifying `raylib` v5.5.0.
* Output logs: [`metrics.csv`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/metrics.csv), [`performance_metrics.txt`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/performance_metrics.txt), [`car_timings.txt`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/car_timings.txt), [`map.txt`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/map.txt).

---

## 4. Summary of Data Structures & Complexity

| Component | Data Structure | Operation | Time Complexity | Implementation Notes |
|---|---|---|---|---|
| Graph Storage | `Gnode<t> array[size]` + `unordered_map<t, int>` | Vertex lookup (`getIndex`) | $O(1)$ | Hash-indexed vertex map |
| Graph Adjacency | `std::list<weighted>` | Edge lookup / update | $O(\text{deg}(u))$ | Edge deduplication via `addOrUpdateEdge` |
| Routing Engine | Min-Heap Priority Queue Dijkstra | Shortest path | $O((V+E)\log V)$ | Early-exit destination optimization |
| Vehicle Fleet | `std::list<vehicle<t>>` | Insertion / Deletion | $O(1)$ | PCU-weighted, multi-speed taxonomy |
| Metrics History | `std::deque<float>` | Rolling window eviction | $O(1)$ (`pop_front()`) | Constant-time eviction |
| Signal Update | Adaptive controller with preemption | Intersection evaluation | $O(V \cdot \text{in-deg})$ | 3-phase (Red, Yellow, Green) + Emergency Override |
| Viewport Raycast | 2D Point-to-Segment Projection | Hover Inspector | $O(V + E)$ | Real-time interactive inspection HUD |
| Geodetic Routing | Min-Heap $A^*$ with Haversine heuristic | Spherical shortest path | $O((V+E)\log V)$ | Admissible scaled Euclidean/spherical heuristic |
| Signal Intelligence | Q-Learning + Max-Pressure Controller | Signal switching | $O(\text{in-deg} \times \text{out-deg})$ | $Q(s, a) \leftarrow Q + \alpha[R + \gamma \max Q - Q]$ |
| Traffic Generation | Demographic Gravity CDF Sample | Commuter injection | $O(\log(N^2))$ | Binary search on precomputed CDF matrix |
| Car-Following | Microscopic IDM ODE Solver | Longitudinal acceleration | $O(1)$ per vehicle | Intelligent Driver Model with smooth deceleration |
| Lane-Changing | MOBIL Safety & Incentive Rule | Lateral lane maneuver | $O(1)$ per candidate | Politeness-weighted follower acceleration evaluation |
| Environmental Engine | Stoichiometric Combustion Physics | Fuel & $CO_2$ tracking | $O(1)$ per tick | Gasoline & diesel idle/cruise consumption regimes |
| Toll Processing | M-Tag RFID vs Cash Lane Delay | Interchange entry delay | $O(1)$ | 1.5s RFID vs 18.0s manual cash collection |
| EV Powertrain | Battery SoC Consumption & Fast-Charge | Electrochemistry | $O(1)$ | 65 kWh capacity, 0.18 kWh/km consumption, 120kW hubs |
| Weather Engine | Multi-state Meteorological Preset | Link friction & capacity | $O(E)$ | Friction, capacity factor, and speed limits across links |
| Diurnal Clock | 24-Hour Continuous Astronomical Clock | Diurnal light interpolation | $O(1)$ | Smooth $D(h)$ ambient darkness & night headlights |
| Network Expansion | Dynamic CPEC Graph Topology Editor | Runtime node & edge construction | $O(1)$ insertion | Instant Gwadar Port / M-8 link commissioning |
| Driver Chase-Cam | 2D Viewport Tracking & Route Polyline | Vehicle tracking & telemetry | $O(K)$ path nodes | Smooth camera centering & planned breadcrumbs |

---

## 5. Advanced Next-Generation ITS Architecture (Phase 5)

Detailed documentation and architectural blueprints for each of the 12 advanced features have been generated inside [`docs/`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs):

1. **[`01_A_STAR_GEODETIC_ROUTING.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/01_A_STAR_GEODETIC_ROUTING.md)**: $A^*$ search with admissible spherical Haversine heuristics.
2. **[`02_MAX_PRESSURE_SIGNAL_CONTROL.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/02_MAX_PRESSURE_SIGNAL_CONTROL.md)**: Reinforcement learning (Q-Table) and network Max-Pressure phase selection.
3. **[`03_GRAVITY_OD_DEMAND_MODEL.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/03_GRAVITY_OD_DEMAND_MODEL.md)**: Census-calibrated gravity model with CDF binary search sampling.
4. **[`04_IDM_CAR_FOLLOWING_PHYSICS.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/04_IDM_CAR_FOLLOWING_PHYSICS.md)**: Microscopic Intelligent Driver Model (IDM) acceleration differential equations.
5. **[`05_MULTI_LANE_MOBIL_MODEL.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/05_MULTI_LANE_MOBIL_MODEL.md)**: Multi-lane motorways with MOBIL overtaking safety and incentive rules.
6. **[`06_CO2_EMISSIONS_FUEL_TRACKER.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/06_CO2_EMISSIONS_FUEL_TRACKER.md)**: Chemical combustion model for idle and cruising fuel burn, $CO_2$ emissions, and congestion waste.
7. **[`07_MTAG_TOLL_PLAZA_SIMULATION.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/07_MTAG_TOLL_PLAZA_SIMULATION.md)**: M-Tag RFID express gates (1.5s delay) vs manual cash booths (18.0s delay) with revenue accounting.
8. **[`08_EV_RANGE_CHARGING_NETWORK.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/08_EV_RANGE_CHARGING_NETWORK.md)**: Electric Vehicle fleet battery SoC tracking, zero tailpipe emissions, and 120kW fast-charging stations.
9. **[`09_WEATHER_SMOG_MONSOON_SYSTEM.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/09_WEATHER_SMOG_MONSOON_SYSTEM.md)**: Meteorological system for Monsoon rains, winter smog, and dense fog affecting pavement grip and BPR link capacity.
10. **[`10_DAY_NIGHT_CYCLE_LIGHTING.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/10_DAY_NIGHT_CYCLE_LIGHTING.md)**: 24-hour astronomical simulation clock, smooth diurnal darkness interpolation, and vehicle headlights.
11. **[`11_INTERACTIVE_CPEC_NETWORK_EDITOR.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/11_INTERACTIVE_CPEC_NETWORK_EDITOR.md)**: Runtime infrastructure construction engine (Gwadar Port & western motorway links with immediate rerouting).
12. **[`12_DRIVER_CHASE_CAM_ROUTE_INSPECTOR.md`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/docs/12_DRIVER_CHASE_CAM_ROUTE_INSPECTOR.md)**: Interactive chase camera tracking en-route vehicles with planned route breadcrumb visualizer and cockpit telemetry card.

