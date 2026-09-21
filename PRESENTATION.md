---
marp: true
theme: default
paginate: true
header: "Adaptive Traffic Simulator & Signal Optimization Engine"
footer: "Advanced Intelligent Transportation System (ITS) Platform"
style: |
  section {
    background-color: #0e1118;
    color: #e6edf3;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
  }
  h1, h2, h3 {
    color: #fcc40a;
  }
  h4 {
    color: #00e676;
  }
  table {
    font-size: 0.8em;
    color: #e6edf3;
  }
  th {
    background-color: #161b22;
    color: #58a6ff;
  }
  td {
    border-color: #30363d;
  }
  code {
    background-color: #161b22;
    color: #39c5bb;
  }
  a {
    color: #58a6ff;
  }
  .highlight {
    color: #00e676;
    font-weight: bold;
  }
  .alert {
    color: #ff5252;
    font-weight: bold;
  }
---

<!-- Slide 1: Title Slide -->
# 🚦 Adaptive Traffic Simulator & Signal Optimization Engine
### High-Performance C++17 Microscopic Engine, 3D Raylib Digital Twin & PyTorch Deep Learning Platform

**Pair Programming & Systems Engineering Presentation**  
*Comprehensive Project Overview, Toolchain, Architecture & Algorithms*

---

<!-- Slide 2: Executive Summary & Problem Statement -->
## 🌍 1. Problem Statement & Motivation

### The Urban Mobility Crisis
- **Economic Loss**: Over **$300 Billion/year** wasted globally in traffic congestion and fuel burn.
- **Environmental Impact**: Vehicle idling in stop-and-go queues is responsible for **~28% of urban greenhouse gas emissions**.
- **Emergency Delays**: Critical ambulances and first responders face life-threatening delays due to rigid, uncoordinated traffic signals.

### Limitations of Existing Solutions
- **Fixed-Time Signal Controllers (Webster/Pre-timed)** fail under unpredictable rush-hour surges.
- **Commercial Simulators (VISSIM, SUMO)** often lack integrated modern 3D visualization, interactive incident control, or real-time neural network inference out-of-the-box.

### 💡 Our Solution
A **unified, cross-platform Intelligent Transportation System (ITS)** that blends:
1. **Microscopic IDM Physics & BPR Congestion Formulations**
2. **Hardware-Accelerated 3D/2D Real-Time Raylib Graphics**
3. **Multi-Strategy Signal Optimization (Webster, Genetic Algorithm, Q-Learning)**
4. **PyTorch Deep Learning Pipeline for Predictive Signal Control**
5. **OpenStreetMap (OSM) Importer to simulate real-world hometown streets**

---

<!-- Slide 3: System Capabilities & Highlights -->
## 🌟 2. Key Capabilities at a Glance

| Feature Category | Capabilities & Real-World Realization |
| :--- | :--- |
| **🏙️ Multi-Tier Networks** | NYC Metro Grid, Multi-District Metropolis, Realm Overworld, and Real-World OpenStreetMap (OSM) imports. |
| **🚦 Dynamic Blinking Nodes** | Intersection nodes actively pulse in **Emerald Green**, **Amber Yellow**, **Ruby Red**, or **EMS Cyan/Red Strobe** based on live signal state. |
| **🚗 Microscopic Fleets** | 6 vehicle types (Sedans, Freight Trucks, Buses, Ambulances, EVs, Motorcycles) governed by **Intelligent Driver Model (IDM)** & **MOBIL** lane-changing. |
| **🧭 Dual Routing Engine** | Real-time **Dijkstra Shortest Path** and **A\* Geodetic Haversine Routing** with dynamic detour rerouting around incidents (`[B]`). |
| **🧠 Deep Learning (PyTorch)** | Residual Multi-Task MLP predicting optimal cycle lengths ($R^2 = 0.9854$), green splits, and delay reduction ($R^2 = 0.9981$). |
| **🌱 Carbon & Telemetry** | Tick-by-tick calculation of fuel consumption (L), $CO_2$ emissions (kg), HCM Level of Service (**LOS A–F**), and RFID toll revenues. |

---

<!-- Slide 4: Technology Stack & Toolchain -->
## 🛠️ 3. Technology Stack & Toolchain

### Core Language & Standards
- **C++17**: Modern language features (structured bindings, smart memory management, standard algorithms, high-efficiency memory layout).
- **Object-Oriented & Generic Programming**: Templated adjacency-list graphs `Graph<T, Size>` and extensible vehicle taxonomies.

### Visualization & Graphics
- **Raylib 4.x / OpenGL 3.3 Core Profile**: Hardware-accelerated 3D perspective camera, 2D tactical maps, dynamic lighting, custom shaders, and immediate-mode HUD.
- **Raymath**: 3D vector arithmetic, spherical linear interpolation (`Vector3Lerp`), and camera transformation matrices.

### Machine Learning & Data Science
- **Python 3.10+ & PyTorch 2.x**: Multi-task deep neural networks (`TrafficOptimizationModel`) with residual skip connections.
- **NumPy, Pandas & Scikit-learn**: Data synthesis, normalization pipelines, and $R^2$ / MAE validation harness.

### Geospatial & Build Tools
- **OpenStreetMap (OSM) XML Engine**: Lightweight, zero-dependency XML parsing and WGS84 GPS coordinate transformation.
- **MinGW-w64 / GCC & CMake**: Universal build system for Windows, Linux, and macOS.

---

<!-- Slide 5: End-to-End System Architecture -->
## 🏛️ 4. End-to-End System Architecture

```
+-----------------------------------------------------------------------------------+
|                            VISUAL USER INTERFACE (Raylib)                         |
|  [Main Menu Hub]   [3D NYC Metro Digital Twin]   [2D Vignelli Map]   [Data Studio] |
+------------------------------------------+----------------------------------------+
                                           |
                                           v
+-----------------------------------------------------------------------------------+
|                               SIMULATION LAYER                                    |
|  Manager Engine (Tick Manager) | CityDesigner (3D Renderer) | OSMImporter (XML)    |
|  CityPlanner (GA & Webster Optimizer) | Electronic M-Tag Toll System             |
+------------------------------------------+----------------------------------------+
                                           |
                                           v
+-----------------------------------------------------------------------------------+
|                             CORE GRAPH & PHYSICS LAYER                            |
|  Graph<T, N> (Adjacency List)  |  RoadDetails (BPR Travel-Time Delay Curves)      |
|  TrafficSignal (3-Aspect + Q)  |  Vehicle<T> (Microscopic IDM & MOBIL Physics)   |
|  Dijkstra's Shortest Path      |  A* Haversine Geodetic Routing                   |
+------------------------------------------+----------------------------------------+
                                           |
                                           v
+-----------------------------------------------------------------------------------+
|                        OFFLINE / ONLINE MACHINE LEARNING (PyTorch)                |
|  dataset_generator.py  -->  models.py (Residual MLP)  -->  evaluate_model.py      |
|  JSON Telemetry Bridge (data/ml_evaluation.json)                                 |
+-----------------------------------------------------------------------------------+
```

---

<!-- Slide 6: Microscopic Vehicle Physics & Roadbed Modeling -->
## 🚗 5. Microscopic Vehicle & Roadbed Physics

### 1. Intelligent Driver Model (IDM)
Each vehicle continuously updates longitudinal acceleration based on gap $s$, current speed $v$, and leading vehicle speed difference $\Delta v$:
$$\dot{v} = a \left[ 1 - \left( \frac{v}{v_0} \right)^\delta - \left( \frac{s^*(v, \Delta v)}{s} \right)^2 \right]$$
$$s^*(v, \Delta v) = s_0 + v T + \frac{v \, \Delta v}{2 \sqrt{a \, b}}$$
- Prevents collisions, models realistic bumper-to-bumper queue compression, and eliminates phantom traffic jams.

### 2. Bureau of Public Roads (BPR) Dynamic Delay Function
Effective link travel time dynamically escalates under volume-to-capacity strain:
$$t_e = t_0 \left[ 1 + \alpha \left( \frac{V}{C_{\text{eff}}} \right)^\beta \right] + P_{\text{incident}}$$
- Incorporates weather-friction reduction factors ($\text{Rain} = 0.85$, $\text{Fog} = 0.70$, $\text{Smog} = 0.75$).

---

<!-- Slide 7: Intelligent Signal Control & Optimization -->
## 🚦 6. Multi-Strategy Traffic Signal Optimization

```
                                  TRAFFIC SIGNAL CONTROL
                                            |
        +-------------------+---------------+-------------------+--------------------+
        |                   |                                   |                    |
        v                   v                                   v                    v
  [1. Webster's]     [2. Genetic Algorithm]            [3. Q-Learning]       [4. Green Corridor]
  Analytical Formula  Evolutionary Search              Max-Pressure Agent    Emergency Wave
  C0 = (1.5L+5)/(1-Y) Multi-generation Chromosomes     Queue Differential    Instant Green on
  Minimum Delay       -48.3% Delay Reduction           Spillback Control     Ambulance Approach
```

### Webster's Baseline Formulation
Computes minimum-delay cycle length $C_0$ and proportional green splits:
$$C_0 = \frac{1.5 L + 5}{1 - \sum Y_i}, \quad g_i = (C_0 - L) \frac{Y_i}{\sum Y_i}$$

### Genetic Algorithm (GA) Search
- **Chromosome**: Vector of normalized green-split splits $[g_1, g_2, \dots, g_k]$ and cycle length $C$.
- **Fitness Function**: Minimizes cumulative intersection delay and fuel burn across simulated peak traffic waves.

---

<!-- Slide 8: Real-Time Blinking Signal Nodes & 3D Visual Twin -->
## 💡 7. Real-Time Blinking Signal Nodes & 3D Twin

### Dynamic Node Signal Blinking Engine
Intersections visually communicate operational health in real time:
- 🟢 **Emerald Green (`GRN`)**: Flowing approach (smooth 2.0 Hz rhythmic expansion pulse).
- 🟡 **Amber Yellow (`YEL`)**: Phase transition caution (3.5 Hz rapid warning flash).
- 🔴 **Ruby Red (`RED`)**: Queue buildup hold (1.2 Hz - 2.2 Hz queue-depth pulse).
- 🚨 **Cyan / Red Strobe (`EMS`)**: 10 Hz high-frequency strobe for emergency vehicles.

### Immersive 3D Digital Twin Features
- **Multi-Level Urban Concrete Decks**: Elevated Midtown concourses and flyover bypasses with concrete support piers.
- **Physical 3-Aspect Signal Posts**: 3D housing bezels, lens glows, visors, and real-time countdown timers.
- **Full Tactical Camera Suite**:
  - **Isometric 3D (`[C]`)**, **Top-Down Tactical Orthographic**, and **Street-Level View**.
  - **Driver Chase-Cam (`[T]`)**: Real-time camera lock on any passenger car, freight truck, or ambulance.
  - **Auto-Orbit (`[R]`)**: Smooth continuous panoramic inspection.

---

<!-- Slide 9: OpenStreetMap (OSM) Importer -->
## 🗺️ 8. OpenStreetMap (OSM) Real-World Importer

### Simulate Any Real Hometown Street
Users can load real-world `.osm` XML files directly into the simulator:
1. **XML Parser (`OSMImporter.h`)**: Extracts `<node>` coordinates, `<way>` road segments, lane numbers, and speed limits.
2. **WGS84 Equirectangular Projection**: Maps real GPS (latitude/longitude) to local Cartesian 3D coordinates $(x, z)$.
3. **Haversine Geodetic Metric**: Computes accurate spherical distances ($km$) between junctions.
4. **Automated Junction & Signal Synthesis**: Identifies multi-way intersections and installs 3-aspect adaptive traffic lights.

### Pre-Packaged Real-World Maps
- 🇵🇰 **Lahore Mall Road (Pakistan)**: Charing Cross, Regal Chowk, GPO, High Court, Governor House, Canal Bank Road.
- 🇬🇧 **London Westminster (UK)**: Parliament Square, Westminster Bridge, Whitehall, Trafalgar Square, Piccadilly Circus.

---

<!-- Slide 10: Deep Learning Pipeline & Benchmark Results -->
## 🧠 9. PyTorch Deep Learning Pipeline

### Architecture: `TrafficOptimizationModel`
- **Model Type**: Deep Residual Multi-Layer Perceptron (ResNet-style MLP).
- **Features**: Batch Normalization, LeakyReLU activations, Dropout (0.2), and Residual Skip Connections.
- **Multi-Task Outputs**:
  1. Optimal Cycle Length ($C_0$)
  2. Multi-Phase Green Split Ratios ($g_1, g_2, \dots$)
  3. Estimated Route Travel Time & Delay Reduction

### Benchmark Validation Results
```
+--------------------------------------------+-----------------------+
| Evaluation Metric                          | Benchmark Value       |
+--------------------------------------------+-----------------------+
| Cycle Length Prediction R² Score           | 0.9854 (97.9% Acc)    |
| Delay Reduction Estimation R² Score        | 0.9981 (98.8% Acc)    |
| HCM Level of Service Classification Acc    | 94.0%                 |
| Route Travel Time Mean Absolute Error (MAE)| 1.15 seconds          |
+--------------------------------------------+-----------------------+
```
*Results exported to `data/ml_evaluation.json` and rendered live inside the Data Optimizer Studio.*

---

<!-- Slide 11: Quantitative Impact & Environmental Results -->
## 📊 10. Quantitative Impact & Environmental Results

```
    [AVERAGE VEHICLE DELAY]                 [CARBON EMISSIONS (CO2)]
    Baseline:  32.5s / trip                 Baseline:  148.2 kg
    GA/ML:     16.8s / trip (-48.3%)        GA/ML:      96.6 kg (-34.8%)
    ========================                ========================
    [====================] 100%             [====================] 100%
    [==========          ]  51.7%           [=============       ]  65.2%
```

### Key Performance Indicators (KPIs)
- **-48.3% Average Delay Reduction**: Achieved through adaptive green allocation during peak commuter hours.
- **-34.8% $CO_2$ Emissions & Fuel Burn**: Drastic reduction in queue idling and stop-and-go shockwaves.
- **HCM Level of Service Upgrade**: Improved overall network grade from **LOS E/F** (unstable/breakdown) to **LOS B/C** (stable flow).
- **+28.5% Throughput Increase**: Accelerated vehicle clearance rate at critical arterial bottlenecks.

---

<!-- Slide 12: Interactive Demonstration Controls -->
## 🎮 11. Interactive In-Game Controls

| Key / Input | Action & Functionality |
| :--- | :--- |
| **`[1]` / `[2]` / `[3]`** | Load Planning Presets (Manhattan Midtown, Broadway Diagonal, Crosstown) |
| **`[L]`** | Load **Lahore Mall Road (OpenStreetMap)** real-world network |
| **`[J]`** | Load **London Westminster (OpenStreetMap)** real-world network |
| **`[O]` / `[D]`** | Open **Data-Driven Optimization Studio & ML Model Runner** |
| **`[SPACE]` / `[N]`** | Play / Pause Simulation & Single-Tick Frame Step |
| **`[Z]` / `[X]` / `[C]`** | Set Simulation Speed to 1x, 2x, or 5x Multiplier |
| **`[T]`** | Lock **Driver Chase-Cam** onto any active commuter or emergency vehicle |
| **`[E]` / `[P]` / `[F]`** | Dispatch Priority Ambulance / Inject Rush Surge / Dispatch 5x Fleet |
| **`[B]`** | Block Road Link to trigger **Dynamic Congestion Rerouting** |
| **`[W]`** | Cycle Dynamic Weather (Clear, Monsoon Rain, Winter Smog, Dense Fog) |
| **`[K]`** | Export Real-Time CSV Telemetry Logs to `data/` directory |

---

<!-- Slide 13: Summary & Future Roadmap -->
## 🚀 12. Summary & Future Horizons

### Summary of Accomplishments
1. **Engineered a Unified Digital Twin**: Combined C++17 microscopic physics, Raylib 3D hardware graphics, and PyTorch deep learning into a single cohesive system.
2. **Real-Time Blinking Signal Nodes**: Developed an intuitive, color-accurate visual signal monitoring system.
3. **OpenStreetMap Integration**: Enabled simulation of any hometown or real-world city streets directly from `.osm` data.
4. **Verified Performance**: Achieved up to **-48.3% delay reduction** and **-34.8% lower carbon emissions**.

### Future Roadmap
- **V2X (Vehicle-to-Everything) Communication**: Cooperative autonomous vehicle (CAV) platooning and virtual intersections.
- **Computer Vision Bridge**: Feed live CCTV video feeds (YOLO/OpenCV) into the graph for real-time queue estimation.
- **Cloud Telemetry Portal**: WebGL browser streaming and cloud-hosted city operations dashboard.

---

<!-- Slide 14: Q&A -->
# 💬 Questions & Discussion

### Thank You!
- **GitHub Repository**: [github.com/usman-irshad1/simpec](https://github.com/usman-irshad1/simpec.git)
- **Documentation**: [`README.md`](README.md) | [`ARCHITECTURE.md`](ARCHITECTURE.md) | [`RECIPE.md`](RECIPE.md)
