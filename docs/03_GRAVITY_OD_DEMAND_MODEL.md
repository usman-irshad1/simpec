# Feature 3: Demographic Gravity Model Origin-Destination (O-D) Matrix

## 1. Executive Summary
The **Demographic Gravity Model Origin-Destination (O-D) Matrix** replaces uniform/pseudo-random vehicle spawning with transportation engineering spatial interaction theory. By incorporating official population demographics for the 11 major Pakistani metropolitan hubs and computing physical geodetic distances, the simulator produces realistic travel demand patterns where major economic corridors (e.g. Karachi-Lahore, Lahore-Islamabad, Faisalabad-Lahore) naturally experience higher vehicle trip generation.

---

## 2. Motivation & Problem Statement
In earlier phases, vehicle origins and destinations were chosen using modulo or uniform random distributions (`rand() % cityCount`). In reality, inter-city passenger and freight travel is governed by demographic gravity: large population centers generate significantly more trips between each other than small peripheral towns. Random generation failed to reproduce authentic corridor saturation (such as the M-2 and M-5 motorways).

---

## 3. Mathematical Formulation

### 3.1 The Gravity Model of Spatial Interaction
Based on Newtonian gravitational mechanics adapted to spatial economics (Zipf & Reilly):
$$T_{ij} = k \cdot \frac{P_i \cdot P_j}{(d_{ij})^\gamma}$$
where:
* $T_{ij}$: Travel demand / trip generation volume between origin city $i$ and destination city $j$ ($i \ne j$).
* $P_i, P_j$: Populations of origin and destination metropolitan centers (in millions).
* $d_{ij}$: Great-circle geodetic distance in kilometers computed via the Haversine formula.
* $\gamma$: Distance decay friction exponent (calibrated to $\gamma = 1.2$ for inter-provincial highway travel).
* $k$: Normalization factor ensuring $\sum_{i \ne j} P(i \rightarrow j) = 1.0$.

### 3.2 Cumulative Distribution Function (CDF) Sampling
For all $N \times (N - 1)$ city pairs ($11 \times 10 = 110$ directional O-D corridors):
1. Compute gravitational mass:
   $$W_{ij} = \frac{P_i \cdot P_j}{(d_{ij})^{1.2}}$$
2. Total network mass:
   $$W_{\text{total}} = \sum_{i} \sum_{j \ne i} W_{ij}$$
3. Corridor probability:
   $$p_{ij} = \frac{W_{ij}}{W_{\text{total}}}$$
4. Monotonically increasing CDF array $\text{CDF}_k = \sum_{m=1}^k p_m$.
5. Fast $O(\log(N^2))$ sampling: Draw uniform float $r \sim \mathcal{U}[0, 1)$ and perform binary search on the CDF array to select the trip's origin and destination.

---

## 4. Implementation Details

### 4.1 Pakistani Demographics in [`Simulator.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Simulator.h)
* Calibrated metropolitan population census values:
  * **Karachi**: $16.0$ Million
  * **Lahore**: $13.0$ Million
  * **Faisalabad**: $3.5$ Million
  * **Islamabad / Rawalpindi**: $3.2$ Million
  * **Gujranwala**: $2.2$ Million
  * **Peshawar**: $2.0$ Million
  * **Multan**: $2.0$ Million
  * **Quetta**: $1.1$ Million
  * **Sialkot**: $0.7$ Million
  * **Sukkur**: $0.5$ Million
  * **DG Khan**: $0.4$ Million

### 4.2 Algorithm Structure
* Method `addGravityDemand(int totalVehicles)`:
  1. Computes inter-city Haversine distances using `Graph::haversineDistanceKm`.
  2. Constructs the 110-cell O-D matrix with normalized cumulative probabilities.
  3. Samples $N$ vehicles using binary search on the CDF.
  4. Assigns realistic vehicle taxonomy mix:
     - Passenger Cars: $68\%$
     - Public Buses: $16\%$
     - Heavy Freight Trucks: $13\%$
     - Priority Ambulances: $3\%$
  5. Injects units into `cityManager` with optimal initial routes precomputed via $A^*$ search.

---

## 5. Complexity Analysis

| Step | Time Complexity | Space Complexity |
|---|---|---|
| Matrix Initialization | $O(N^2) = 110$ operations | $O(N^2) = 110$ cells |
| Single Vehicle O-D Draw | $O(\log(N^2)) \approx 7$ comparisons | $O(1)$ |
| Batch Spawning ($M$ vehicles) | $O(M \log(N^2))$ | $O(M)$ |

---

## 6. Dependencies & Interoperability
* **C++ Standard**: C++11 / C++14 compatible (`<vector>`, `<cmath>`, `<cstdlib>`).
* **Dependencies**: Requires `Graph::getVertexCoordinates` and `Graph::haversineDistanceKm` from Feature 1.
* Fully interoperable with the vehicle taxonomy system and $A^*$ routing engine.

---

## 7. Verification & Automated Test Results
Verified via Test 17 in [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp):
1. Injected 30 probabilistically weighted units into an active simulation.
2. Verified vehicle count increased exactly by 30 with realistic spatial distribution and valid $A^*$ routing paths.
