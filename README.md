# 🚗 ADAS Emergency Braking System (AEB) — Native C++

A fully functional simulation of an **Automatic Emergency Braking (AEB)** system built from scratch in Native C++ using POSIX UDP/TCP sockets. This project demonstrates the core sensor-to-controller-to-actuator pipeline found in real ADAS architectures.

---

## 🏗️ System Architecture

```
┌─────────────────────┐         UDP (10Hz)        ┌────────────────────────┐        TCP         ┌────────────────────┐
│  LiDAR Sensor Node  │ ──────────────────────►  │  ADAS Controller Node  │ ─────────────────► │  Actuator Node     │
│      (Python)       │    distance + velocity     │       (C++)            │  BRAKE_100_PERCENT │     (C++)          │
└─────────────────────┘                           └────────────────────────┘                    └────────────────────┘
```

**Three independent nodes communicating in real-time over local network sockets:**

| Node | Language | Protocol | Role |
|---|---|---|---|
| LiDAR Sensor | Python | UDP Port 5005 | Simulates front-mounted distance sensor at 10Hz |
| ADAS Controller | C++ | UDP Listener + TCP Client | Calculates TTC physics and triggers brake command |
| Brake Actuator | C++ | TCP Server Port 5006 | Simulates physical ABS braking system |

---

## ⚙️ How It Works

1. The **LiDAR Sensor Node** continuously broadcasts vehicle distance and relative velocity over UDP.
2. The **ADAS Controller** calculates **Time-To-Collision (TTC)** on every frame:
   ```
   TTC = distance / |relative_velocity|
   ```
3. When **TTC drops below 2.0 seconds**, the controller immediately dispatches `BRAKE_100_PERCENT` over TCP.
4. The **Actuator Node** receives the command and engages the ABS braking simulation.

---

## 🚨 Simulation Output

**ADAS Controller Log:**
```
🧠 ADAS Controller Node Booting...
📡 Listening for Lidar Data on UDP Port 5005
🔗 Connected to Actuator Node on TCP Port 5006
🚨 CRITICAL DANGER: TTC = 1.96583s. COLLISION IMMINENT!
⚡ INITIATING AUTOMATIC EMERGENCY BRAKING (AEB)...
```

**Actuator Node Log:**
```
==============================================
     🔥 EMERGENCY BRAKE DEPLOYED 🔥        
==============================================
Command received: BRAKE_100_PERCENT
Actuator applying 100% clamping force.
ABS engaging... Vehicle is coming to a stop.
==============================================
```

---

## 🛠️ Tech Stack

- **Language:** C++ (C++11), Python 3
- **Networking:** POSIX UDP/TCP Sockets (no external libraries)
- **Compiler:** `clang++` (macOS native)
- **Concepts:** Real-time sensor fusion, physics-based TTC calculation, inter-process communication

---

## 🚀 Running the Simulation

**Terminal 1 — Start the Actuator (Brakes):**
```bash
clang++ -std=c++11 actuator_node/actuator_node.cpp -o actuator_node/actuator_bin
./actuator_node/actuator_bin
```

**Terminal 2 — Start the ADAS Controller:**
```bash
clang++ -std=c++11 adas_controller/adas_controller.cpp -o adas_controller/adas_bin
./adas_controller/adas_bin
```

**Terminal 3 — Start the LiDAR Sensor:**
```bash
python3 sensor_node/lidar_sim.py
```

Watch all three terminals — when the distance closes below the safety threshold, the emergency brake fires automatically.

---

## 📁 Project Structure

```
adas-emergency-braking-cpp/
├── sensor_node/
│   └── lidar_sim.py          # Python LiDAR simulator (UDP Publisher)
├── adas_controller/
│   └── adas_controller.cpp   # C++ TTC calculator (UDP Subscriber + TCP Client)
└── actuator_node/
    └── actuator_node.cpp     # C++ brake actuator (TCP Server)
```

---

## 👨‍💻 Author

**Harikrishna Raj** — BSc Computer Science, BSBI Berlin  
[LinkedIn](https://linkedin.com/in/harikrishnark) · [GitHub](https://github.com/harikrishnark)
