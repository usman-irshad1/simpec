# 🍳 Universal Build & Run Recipe

This guide provides step-by-step instructions to compile and run the **Adaptive Traffic Simulator & Signal Optimization Engine** on any machine (Windows, Linux, macOS) immediately after downloading.

---

## 📋 System Prerequisites

| Platform | C++ Compiler | Graphics / Windowing Library | Python (ML Pipeline) |
| :--- | :--- | :--- | :--- |
| **Windows** | MinGW `g++` (C++17) or MSVC | `raylib.dll` (included in root) | Python 3.8+ (optional for ML) |
| **Linux (Ubuntu/Debian)** | `g++` (`build-essential`) | `libraylib-dev` or Raylib source | Python 3.8+ (optional for ML) |
| **Linux (Fedora/Arch)** | `gcc-c++` / `base-devel` | `raylib-devel` / `raylib` | Python 3.8+ (optional for ML) |
| **macOS** | Apple Clang / Homebrew `gcc` | `raylib` (`brew install raylib`) | Python 3.8+ (optional for ML) |

---

## 🚀 1. Windows Recipe (Fastest & Out-of-the-Box)

The repository includes `raylib.dll` and Raylib headers in `include/` for instant compilation.

### Option A: Direct Command Line (PowerShell / Command Prompt)
Open PowerShell or Command Prompt in the repository root directory:

```powershell
# 1. Compile the visual simulator (Graphics.exe)
g++ -std=c++17 -O2 -Iinclude -Isrc/core -Isrc/simulation -Isrc/visualization -L. src/visualization/Graphics.cpp raylib.dll -o Graphics.exe -lopengl32 -lgdi32 -lwinmm

# 2. Run the simulator
.\Graphics.exe
```

### Option B: VS Code (One-Click with Code Runner)
1. Open the project folder in **VS Code**.
2. Ensure you have the **C/C++** and **Code Runner** extensions installed.
3. Open `src/visualization/Graphics.cpp`.
4. Press `Ctrl + Alt + N` (or click the **Run** play button in the top right).
   * The workspace `.vscode/settings.json` and `.vscode/tasks.json` are pre-configured to automatically compile and launch `Graphics.exe` from the project root.

### Option C: CMake on Windows
```powershell
mkdir build
cd build
cmake ..
cmake --build . --config Release
.\Graphics.exe
```

---

## 🐧 2. Linux Recipe (Ubuntu / Debian / Fedora / Arch)

### Step 1: Install Dependencies
#### Ubuntu / Debian / Pop!_OS:
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake libraylib-dev libgl1-mesa-dev libx11-dev libxcursor-dev libxrandr-dev libxinerama-dev libxi-dev
```

#### Fedora / RHEL:
```bash
sudo dnf install -y gcc-c++ cmake raylib-devel mesa-libGL-devel libX11-devel libXcursor-devel libXrandr-devel libXinerama-devel libXi-devel
```

#### Arch Linux / Manjaro:
```bash
sudo pacman -S --needed base-devel cmake raylib
```

### Step 2: Build & Run
#### Using CMake (Recommended):
```bash
mkdir -p build && cd build
cmake ..
make -j$(nproc)
./Graphics
```

#### Using Direct `g++`:
```bash
g++ -std=c++17 -O2 \
    -Iinclude -Isrc/core -Isrc/simulation -Isrc/visualization \
    src/visualization/Graphics.cpp \
    -o Graphics \
    -lraylib -lGL -lm -lpthread -ldl -lrt -lX11

./Graphics
```

---

## 🍎 3. macOS Recipe (Apple Silicon M1/M2/M3 & Intel)

### Step 1: Install Dependencies via Homebrew
```bash
# Install Homebrew if not already installed: https://brew.sh
brew install cmake raylib
```

### Step 2: Build & Run
#### Using CMake (Recommended):
```bash
mkdir -p build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
./Graphics
```

#### Using Direct `clang++`:
```bash
clang++ -std=c++17 -O2 \
    -Iinclude -Isrc/core -Isrc/simulation -Isrc/visualization \
    -I$(brew --prefix raylib)/include \
    -L$(brew --prefix raylib)/lib \
    src/visualization/Graphics.cpp \
    -o Graphics \
    -lraylib -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

./Graphics
```

---

## 🧪 4. Headless Test Suite (No GUI Required)

To verify the core Dijkstra, A* Haversine, and BPR congestion physics without launching the 3D graphics window:

### On Windows:
```powershell
g++ -std=c++17 -O2 -Iinclude -Isrc/core -Isrc/simulation -Isrc/visualization src/tests/test_simulation.cpp -o test_simulation.exe
.\test_simulation.exe
```

### On Linux / macOS:
```bash
g++ -std=c++17 -O2 -Iinclude -Isrc/core -Isrc/simulation -Isrc/visualization src/tests/test_simulation.cpp -o test_simulation
./test_simulation
```

---

## 🧠 5. Machine Learning Pipeline (Python & PyTorch)

The simulator includes a deep neural network pipeline for signal timing prediction and traffic speed forecasting.

### Step 1: Create Virtual Environment & Install Dependencies
```bash
# 1. Create a Python virtual environment
python -m venv venv

# 2. Activate virtual environment
# Windows (PowerShell):
.\venv\Scripts\Activate.ps1
# Windows (CMD):
.\venv\Scripts\activate.bat
# Linux / macOS:
source venv/bin/activate

# 3. Install requirements
pip install -r requirements.txt
```

### Step 2: Evaluate the Pre-Trained Model
```bash
python ml_pipeline/evaluate_model.py
```
This will:
* Load the pre-trained weights (`ml_pipeline/traffic_signal_model.pth`).
* Compute $R^2$ Score, Mean Absolute Error (MAE), and Prediction Accuracy across test batches.
* Output detailed metrics to `data/ml_evaluation.json`.
* Enable the in-game GUI **[TEST ML ACCURACY [E]]** button to display live telemetry.

### Step 3: Retrain the Neural Network (Optional)
```bash
# Generate synthetic dataset (if needed)
python ml_pipeline/dataset_generator.py

# Train the model
python ml_pipeline/train.py
```

---

## 🎮 6. Interactive In-Game Controls Cheat Sheet

| Key / Action | Function |
| :--- | :--- |
| **`[M]` / `[ESC]`** | Return to Main Menu Hub |
| **`[O]` / `[D]`** | Open Data-Driven Optimization Studio & ML Model Runner |
| **`[F1]`** | Open NYC Metro City Designer Settings |
| **`[SPACE]`** | Pause / Resume Simulation |
| **`[N]`** | Single-Tick Advance (Step Frame) |
| **`[Z]` / `[X]` / `[C]`** | Set Simulation Speed to 1x, 2x, or 5x |
| **`[1]` / `[2]` / `[3]`** | Load City Planning Presets (Manhattan, Broadway, Crosstown) |
| **`[T]`** | Toggle Driver Chase-Cam Tracking |
| **`[R]`** | Reset Camera / Toggle 3D Auto-Orbit |
| **`[V]` / `[I]`** | Dispatch Single Commuter Vehicle / Taxi |
| **`[F]`** | Dispatch 5-Vehicle Mixed Fleet |
| **`[E]`** | Dispatch Emergency Ambulance (Triggers Green Wave Preemption) |
| **`[P]`** | Inject Peak Rush-Hour Commuter Surge |
| **`[B]`** | Toggle Road Incident Blockage & Test Dynamic Rerouting |
| **`[W]`** | Cycle Weather (Clear, Monsoon Rain, Smog, Dense Fog) |
| **`[K]`** | Export Real-Time CSV Telemetry to `data/` |
| **Right-Click Drag** | Orbit & Tilt 3D Perspective Camera |
| **Middle-Click Drag** | Pan 3D Camera Target |
| **Mouse Wheel** | Zoom In / Out |
