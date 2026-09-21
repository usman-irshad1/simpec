# Feature 4: Microscopic Intelligent Driver Model (IDM) Car-Following & Shockwave Physics

## 1. Executive Summary
The **Intelligent Driver Model (IDM)** introduces continuous microscopic longitudinal vehicle physics to the simulator. Rather than using discrete block movement or static velocity decrements, each vehicle continuously evaluates the distance to the preceding vehicle and dynamic approach velocity, generating realistic acceleration, deceleration, and the natural emergence of **traffic shockwaves (phantom traffic jams)**.

---

## 2. Motivation & Problem Statement
In macroscopic or queue-based simulators, vehicles either move at the road's average speed or sit in a queue. Real traffic dynamics exhibit non-linear phenomena:
* Reaction delay and smooth acceleration when signals turn green.
* Gradual braking rather than abrupt stops when approaching slow trucks or queues.
* Phantom traffic jams (backward-propagating deceleration waves) caused by small disturbances on high-density highways.

The Intelligent Driver Model (Treiber, Hennecke, and Helbing, 2000) is the global gold standard for simulating crash-free, realistic microscopic car-following behavior.

---

## 3. Mathematical Formulation

### 3.1 The Continuous IDM Acceleration Equation
For a vehicle traveling at current velocity $v$ following a lead vehicle at distance $s$ with velocity $v_{\text{lead}}$ and velocity difference $\Delta v = v - v_{\text{lead}}$:
$$\frac{\mathrm{d}v}{\mathrm{d}t} = a_{\text{max}} \left[ 1 - \left(\frac{v}{v_0}\right)^4 - \left(\frac{s^*(v, \Delta v)}{s}\right)^2 \right]$$

### 3.2 Dynamic Desired Gap ($s^*$)
$$s^*(v, \Delta v) = s_0 + v \cdot T + \frac{v \cdot \Delta v}{2\sqrt{a_{\text{max}} \cdot b}}$$
where:
* $v_0$: Desired free-flow speed (e.g. $33.33\text{ m/s} = 120\text{ km/h}$).
* $s_0$: Minimum bumper-to-bumper standstill distance in traffic jams ($2.5\text{ meters}$).
* $T$: Safe time headway ($1.5\text{ seconds}$ for cars, $2.0\text{ seconds}$ for trucks).
* $a_{\text{max}}$: Comfortable maximum acceleration ($1.5\text{ m/s}^2$).
* $b$: Comfortable braking deceleration ($2.0\text{ m/s}^2$).
* $s$: Actual distance to the front bumper of the lead vehicle.
* $\Delta v$: Approach rate ($v - v_{\text{lead}}$).

### 3.3 Physics Interpretation
1. **Free Road Term** $\left[1 - (v/v_0)^4\right]$: Dominates when $s \gg s^*$. The vehicle accelerates smoothly toward $v_0$.
2. **Braking / Interaction Term** $\left[-(s^*/s)^2\right]$: Dominates when approaching a slower vehicle or red signal. If distance $s$ decreases rapidly, the interaction term induces strong braking up to emergency deceleration limits (clamped at $-9.0\text{ m/s}^2$).

---

## 4. Implementation Details in [`vehicle.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/vehicle.h)
* Added microscopic physics state:
  * `float currentSpeed`: Velocity in m/s.
  * `float acceleration`: Instantaneous acceleration ($\text{m/s}^2$).
  * `float positionOnRoad`: Distance along current road segment (meters).
  * `float desiredSpeed`, `idmTimeHeadway`, `idmMinDistance`, `idmMaxAccel`, `idmComfortDecel`.
* Implemented `calculateIDMAcceleration(distanceToLead, leadSpeed)`:
  - Dynamically adapts based on `speedMultiplier` for trucks ($0.8\times$), buses ($0.9\times$), and emergency vehicles ($1.35\times$).
  - Incorporates safety clamps to prevent reverse velocities or unphysical decelerations.

---

## 5. Complexity Analysis

| Operation | Time Complexity | Space Complexity |
|---|---|---|
| IDM Acceleration Calculation | $O(1)$ | $O(1)$ |
| Numerical Euler Integration ($\Delta t$) | $O(1)$ | $O(1)$ |
| Per-Vehicle Road Scan | $O(1)$ (with leader pointer) | $O(1)$ |

---

## 6. Dependencies & Interoperability
* **C++ Standard**: C++11 / C++14 compatible (`<cmath>`).
* Fully interoperable with the existing macroscopic `timeRemaining` and `timespent` metrics, ensuring all queue-discharge and signal timers continue operating without regression.

---

## 7. Verification & Automated Test Results
Verified via Test 18 in [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp):
1. **Free Road Cruising**: At $20\text{ m/s}$ with $300\text{ m}$ lead space, calculated smooth acceleration $a = +1.31\text{ m/s}^2$.
2. **Stationary Obstacle Braking**: At $20\text{ m/s}$ approaching a queue at $12\text{ m}$, calculated emergency deceleration $a = -9.0\text{ m/s}^2$.
