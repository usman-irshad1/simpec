# Feature 5: Multi-Lane Motorways & MOBIL Overtaking Model

## 1. Executive Summary
The **Multi-Lane Motorways & MOBIL Overtaking Model** equips the highway network with multi-lane geometry and driver decision-making for lane changes. Utilizing the **MOBIL (Minimizing Overall Braking Induced by Lane Changes)** framework, vehicles determine when it is safe and advantageous to change lanes, enabling faster passenger cars to overtake slow-moving heavy freight trucks and buses on Pakistani motorways (e.g. M-2, M-5).

---

## 2. Motivation & Problem Statement
On single-pipe roads, a single slow vehicle (such as an overloaded sugarcane truck traveling at $40\text{ km/h}$) blocks all trailing vehicles, artificially collapsing the road's capacity. Real Pakistani motorways (M-2 Lahore-Islamabad, M-3 Lahore-Abdul Hakeem, M-5 Multan-Sukkur) feature 2 to 3 lanes in each direction. Implementing multi-lane geometry and lateral lane-changing logic resolves artificial queuing and models realistic highway lane utilization.

---

## 3. Mathematical Formulation

### 3.1 The MOBIL Model (Kesting, Treiber, and Helbing)
A lane change from the current lane to a target adjacent lane is executed if and only if two criteria are simultaneously satisfied:

#### 1. Safety Criterion
The driver must ensure the new prospective follower in the target lane will not be forced to brake dangerously:
$$\tilde{a}_{\text{new\_follower}} \ge -b_{\text{safe}}$$
where $b_{\text{safe}} = 4.0\text{ m/s}^2$ is the maximum acceptable braking imposed on others.

#### 2. Incentive Criterion
The personal acceleration advantage plus the politeness-weighted impact on neighboring vehicles must exceed a switching threshold $\Delta a_{\text{th}}$:
$$(a_{\text{target}} - a_{\text{current}}) + p \cdot (\tilde{a}_{\text{new\_follower}} - a_{\text{new\_follower}}) > \Delta a_{\text{th}}$$
where:
* $a_{\text{target}}$: Acceleration the driver would enjoy in the candidate lane.
* $a_{\text{current}}$: Acceleration in the current lane.
* $p \in [0, 1]$: Politeness factor (default $0.30$), modeling how much the driver cares about not inconveniencing the new follower.
* $\Delta a_{\text{th}}$: Lane-changing threshold (default $0.20\text{ m/s}^2$) to avoid erratic zig-zagging.

---

## 4. Implementation Details

### 4.1 Highway Geometry in [`RoadDetails.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/RoadDetails.h)
* Added `int numLanes` (defaults to 3 lanes for high-capacity motorways $\ge 80$ capacity, and 2 lanes for standard regional links).
* Added `getNumLanes()` and `setNumLanes(lanes)`.

### 4.2 Lane State & Evaluation in [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h)
* Added `int currentLane`:
  * Lane 0: Slow / Heavy Freight Lane (default for `VEHICLE_TRUCK`)
  * Lane 1: Cruising Lane (default for `VEHICLE_CAR` and `VEHICLE_BUS`)
  * Lane 2: Fast / Overtaking Lane
* Implemented `shouldChangeLaneMOBIL(accelCurLane, accelTargetLane, accelNewFollower, politeness, threshold)`:
  - Enforces safety check: Rejects move if `accelNewFollower < -4.0f`.
  - Enforces incentive threshold: Approves move if driver acceleration gain outweighs follower braking.

---

## 5. Complexity Analysis

| Step | Time Complexity | Space Complexity |
|---|---|---|
| Safety Check | $O(1)$ | $O(1)$ |
| Incentive Check | $O(1)$ | $O(1)$ |
| Lane Assignment | $O(1)$ | $O(1)$ |

---

## 6. Dependencies & Interoperability
* **C++ Standard**: C++11 / C++14 compatible.
* Coupled with the Intelligent Driver Model (Feature 4): Accelerations evaluated by MOBIL are produced directly by the vehicle's `calculateIDMAcceleration()` function.

---

## 7. Verification & Automated Test Results
Verified via Test 19 in [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp):
1. **Multi-Lane Geometry**: Verified highway links with capacity $\ge 80$ automatically configure with 3 lanes.
2. **Safe Lane Change**: Target lane with acceleration improvement ($+1.2$ vs $+0.2$) and modest follower braking ($-1.0\text{ m/s}^2$) approved (`changeSafe == true`).
3. **Unsafe Cut-Off Blocked**: Cut-off imposing violent emergency braking ($-5.0\text{ m/s}^2$) strictly rejected (`changeUnsafe == false`).
