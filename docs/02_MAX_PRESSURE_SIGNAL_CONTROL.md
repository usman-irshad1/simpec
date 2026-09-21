# Feature 2: Max-Pressure & Q-Learning Adaptive Signal Controller

## 1. Executive Summary
The **Max-Pressure & Q-Learning Adaptive Signal Controller** upgrades the intersection traffic signal engine from simple greedy local heuristic switching to an intelligent dual-strategy control model:
1. **Max-Pressure Control (Varaiya 2013)**: Guarantees network-wide stability and prevents downstream spillback ("blocking the box") by incorporating departing downstream queue pressures.
2. **Reinforcement Learning (Tabular Q-Learning)**: Enables intersection controllers to dynamically learn optimal green split decisions based on discretized queue states, delay reduction rewards, and epsilon-greedy exploration.

---

## 2. Motivation & Problem Statement
Traditional adaptive traffic signals focus solely on inbound queues ($Q_{\text{in}}$). When an inbound approach becomes congested, the signal greedily turns green. However, if the departing downstream road ($Q_{\text{out}}$) is already saturated or experiencing gridlock, granting a green signal discharges vehicles directly into an already clogged intersection, causing gridlock and spillback that cascades across adjacent city corridors.

Max-Pressure and Q-Learning resolve this by balancing inbound queues against downstream absorption capacity and learning from traffic throughput feedback.

---

## 3. Mathematical Formulation

### 3.1 Max-Pressure Formulation
For an intersection $x$ with inbound road approach $(u \rightarrow x)$ and departing downstream roads $(x \rightarrow v)$:
$$P(u \rightarrow x) = Q(u \rightarrow x) - \frac{1}{|\text{out}(x)|} \sum_{v \in \text{out}(x)} Q(x \rightarrow v) + \text{starvationCost}(u \rightarrow x)$$
* When downstream roads are free ($Q_{\text{out}} \approx 0$), pressure $P$ is high, driving maximum green time to clear the bottleneck.
* When downstream roads are clogged ($Q_{\text{out}} \gg 0$), pressure $P$ drops or becomes negative, preventing spillback.

### 3.2 Tabular Q-Learning Model
Each signal controller maintains a Q-value table $Q(s, a)$ mapping traffic states to actions:
* **State Space ($S$)**: Discretized queue depth:
  * State 0: Low ($Q < 5$)
  * State 1: Moderate ($5 \le Q < 15$)
  * State 2: High ($15 \le Q < 30$)
  * State 3: Saturated / Critical ($Q \ge 30$)
* **Action Space ($A$)**:
  * Action 0: `MAINTAIN_PHASE` (Keep current approach green)
  * Action 1: `SWITCH_PHASE` (Clear intersection and switch to max-pressure candidate)
* **Reward Function ($R$)**:
  $$R = (Q_{\text{prev}} - Q_{\text{curr}}) - \text{penalty}_{\text{downstream}}$$
  Positive reinforcement is awarded whenever queue depth decreases, with an explicit penalty applied if downstream pressure is severely congested ($> 15$ vehicles).
* **Bellman Temporal Difference Update**:
  $$Q(s, a) \leftarrow Q(s, a) + \alpha \left[ R + \gamma \max_{a'} Q(s', a') - Q(s, a) \right]$$
  where learning rate $\alpha = 0.12$, discount factor $\gamma = 0.90$, and $\epsilon$-greedy exploration rate $\epsilon = 0.08$.

---

## 4. Implementation Details

### 4.1 Signal Enhancements in [`TrafficSignal.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/TrafficSignal.h)
* Added `qTable[4][2]`, `learningRate`, `discountFactor`, `epsilon`.
* Added helper methods:
  * `discretizeQueueState(int q)`: Maps continuous vehicle count to discrete state.
  * `selectQAction(int state, bool explore)`: $\epsilon$-greedy action selection.
  * `updateQValue(int s, int a, float reward, int nextS)`: Online Bellman update.
  * `shouldSwitchQAdaptive(int currentQueue, int maxOtherQueue, float downstreamPressure)`:
    Evaluates both Q-learning policy and max-pressure differential before initiating yellow clearance.

### 4.2 Downstream Network Polling in [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h)
In `updateSignals()`:
* The manager computes the departing downstream queue sum for intersection $i$:
  ```cpp
  float downstreamQSum = 0.0f;
  int outEdgeCount = 0;
  for (auto const& outEdge : nodePtr[i].Neighbors) {
      downstreamQSum += (float)outEdge.weight.queueCount;
      outEdgeCount++;
  }
  float downstreamAvg = (outEdgeCount > 0) ? (downstreamQSum / (float)outEdgeCount) : 0.0f;
  float downstreamPressure = (float)worstCandidate->queueCount - downstreamAvg;
  ```
* Evaluates `shouldSwitchQAdaptive()`: Ensures smooth multi-phase clearance (Green $\rightarrow$ Yellow $\rightarrow$ Red) and avoids spilling traffic into clogged downstream avenues.

---

## 5. Complexity Analysis

| Operation | Time Complexity | Space Complexity |
|---|---|---|
| State Discretization | $O(1)$ | $O(1)$ |
| Q-Table Action Lookup | $O(1)$ | $O(|S| \times |A|) = O(4 \times 2) = 8$ floats |
| Bellman Value Update | $O(1)$ | $O(1)$ |
| Downstream Pressure Scan | $O(\text{deg}(x))$ | $O(1)$ |

---

## 6. Dependencies & Interoperability
* **C++ Standard**: C++11 / C++14 compatible (`<cstdlib>`, `<cmath>`).
* **Emergency Preemption**: Emergency vehicles automatically override Q-learning and max-pressure decisions, guaranteeing uninterrupted green corridors for ambulances.
* **Yellow Clearance**: Fully interoperable with the 3-phase signal clearance model.

---

## 7. Verification & Automated Test Results
Verified via Test 16 in [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp):
1. State discretization across bounds verified: $Q=2 \rightarrow 0$, $Q=10 \rightarrow 1$, $Q=22 \rightarrow 2$, $Q=45 \rightarrow 3$.
2. Bellman Q-table adaptation verified: Positive reward ($+5.0$) correctly elevated $Q(1, 1)$.
3. Downstream pressure switching logic executed with `switchAllowed = true`.
