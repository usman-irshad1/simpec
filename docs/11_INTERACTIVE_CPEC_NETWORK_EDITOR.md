# Feature 11: Interactive Highway Construction & CPEC Expansion Editor

## 1. Overview & Summary
The China-Pakistan Economic Corridor (CPEC) represents one of the largest infrastructure modernization initiatives in South Asia, constructing high-speed motorways, dry ports, and western transit routes linking Gwadar Port to northern Pakistan and Khunjerab Pass.

Feature 11 provides an **interactive runtime infrastructure expansion engine** that enables traffic engineers to:
1. Construct new geopolitical city nodes dynamically (e.g., deep-sea port at **Gwadar**).
2. Commission multi-lane motorways and expressways between any two regional centers at runtime.
3. Automatically trigger instantaneous network topology recalculation, re-indexing adjacency lists, and executing $A^*$ / Dijkstra rerouting to immediately absorb real-time traffic onto new corridors.

```
       [Traffic Network Initialized (11 Cities)]
                           |
                           v
        [User Trigger / CPEC Megaproject Action]
                           |
                           v
           +-------------------------------+
           |   addCPECCityNode("Gwadar")   |
           |      Lat: 25.12, Lon: 62.32   |
           +-------------------------------+
                           |
                           v
     +--------------------------------------------+
     |   addCPECHighwayLink("Gwadar", "Quetta")   |
     |   addCPECHighwayLink("Gwadar", "Karachi")  |
     +--------------------------------------------+
                           |
                           v
    [Network Rerouting Triggered: ShortestPath()]
                           |
                           v
  [Vehicles Bound for Gwadar Port Stream Onto M-8 Corridor]
```

---

## 2. Algorithmic Implementation

### 2.1 Dynamic Node & Edge Insertion
1. **Node Registration**:
   `insertVertex(cityName)` appends the new urban center to the graph's internal vertex array and maps the string key to its integer vertex ID in $O(1)$ time via `vertexIndexMap`.
2. **Geodetic GPS Binding**:
   `setVertexCoordinates(cityName, lat, lon)` updates the geodetic registry for spherical Haversine heuristic calculation.
3. **Edge Construction**:
   `makeEdge(uIdx, vIdx, length, speed, capacity)` inserts the new bidirectional road links with custom parameters.
4. **Dynamic Flow Redirection**:
   `ShortestPath()` iterates over all active en-route and queued vehicles, invoking $A^*$ to recompute optimal paths. Vehicles discover the newly minted highway bypasses and divert automatically.

---

## 3. Verification & Route Discovery
In automated test 25:
- Added node `"Gwadar"` ($25.1216^\circ\text{ N}, 62.3254^\circ\text{ E}$).
- Constructed links: Gwadar $\leftrightarrow$ Quetta ($650\text{ km}, 120\text{ km/h}$) and Gwadar $\leftrightarrow$ Karachi ($630\text{ km}, 120\text{ km/h}$).
- Evaluated shortest path query from `Gwadar` to `Islamabad`.
- Resulting optimal corridor discovered:
  $$\text{Gwadar} \longrightarrow \text{Quetta} \longrightarrow \text{Multan} \longrightarrow \text{Islamabad}$$

---

## 4. Dependencies
- [`Graph.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Graph.h): Dynamic vertex/edge insertion and Haversine geodetic lookup.
- [`Manger.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Manger.h): `addCityNode()` and `addHighwayLink()`.
- [`Simulator.h`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Simulator.h): `addCPECCityNode()` and `addCPECHighwayLink()`.
- [`test_simulation.cpp`](file:///C:/Users/usman/Downloads/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/Adaptive-Traffic-Simulator-with-Dynamic-Rerouting-Signal-Control-main/test_simulation.cpp): Verified in Automated Test 25.
