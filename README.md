# critics_viz_rviz

> **Real-time RViz2 panel for visualising Nav2 MPPI critic costs**

An RViz2 panel plugin that subscribes to `/controller_server/critics_stats` and gives you a live, at-a-glance view of every MPPI critic's contribution — cost bars, percentage share, trigger status, dominant critic, and a scrolling cost-history chart — all without leaving RViz2.

[Screencast from 05-26-2026 04:58:51 PM.webm](https://github.com/user-attachments/assets/5ebf6855-138f-4a07-baeb-0292530c6e88)

---

## Features

| Widget | What it shows |
|--------|---------------|
| **Metric cards** | Active critic count · triggered count · dominant critic + % share · total cost |
| **Per-critic bar rows** | Cost bar scaled to the current max, raw cost value, and % of total; a green dot lights up when the critic fires |
| **Cost history chart** | Scrolling line chart of the last 80 control cycles, one coloured series per critic |
| **Alert banner** | Turns amber when `ObstaclesCritic` cost exceeds 20; turns blue when total cost is near zero |
| **Status line** | Shows "waiting" / "live" and the topic name |

---

## Requirements

### ROS 2 distribution

| Requirement | Version |
|-------------|---------|
| ROS 2 | **Humble Hawksbill** (tested) |
| Nav2 | navigation2 repo  |

### System packages

```bash
# Qt5 + QtCharts (build deps)
sudo apt install \
  libqt5charts5-dev \
  qtbase5-dev


```

### ADDITIONAL NAV2 PACKAGE FOR MPPI CONTROLLER

This panel requires a `nav2_msgs/msg/CriticsStats` message that is **not** part of the standard Humble release.  
You must clone the navigation2 repo and build it first . follow below step 

```bash
mkdir -p ~/nav2_ws/src
cd ~/nav2_ws/src
git clone https://github.com/NITHISHKUMAR-A/navigation2.git
cd ~/nav2_ws
colcon build --symlink-install
source install/local_setup.bash 
```
```bash
echo "source ~/nav2_ws/install/local_setup.bash" >> ~/.bashrc
source ~/.bashrc
```



> **Note:** The `CriticsStats` message must define at least `critics_names` (string[]), `costs_sum` (float64[]), and `critics_triggered` (bool[]).

---

## Installation

```bash
mkdir -p ~/ros_ws/src
cd ~/ros_ws/src
git clone https://github.com/NITHISHKUMAR-A/critics_viz_rviz.git
cd ~/ros_ws

# Source ROS 2 base and the nav2_ws overlay that provides CriticsStats

source /opt/ros/humble/setup.bash
source ~/nav2_ws/install/setup.bash
colcon build --packages-select critics_viz_rviz
source install/setup.bash
```

---

## Usage
NOTE : Must add the below param in your nav2_params.yaml
``` bash
FollowPath:
  publish_critics_stats: true
```
### 1 — Launch your Nav2 stack

Make sure the MPPI controller is running and publishing on `/controller_server/critics_stats`.

### 2 — Launch RViz2 (sourcing both workspaces)

```bash
source /opt/ros/humble/setup.bash
source ~/nav2_ws/install/setup.bash
source ~/ros_ws/install/setup.bash
rviz2
```

### 3 — Add the panel

1. In RViz2 go to **Panels → Add New Panel**
2. Select **`critics_viz_rviz / CriticsPanel`**
3. Click **OK**

The panel will show **"● waiting for /controller_server/critics_stats"** until the first message arrives, then switch to live mode automatically.

---

## Package structure

```
critics_viz_rviz/
├── include/critics_viz_rviz/
│   └── critics_panel.hpp        # Panel + helper widget declarations
├── src/
│   └── critics_panel.cpp        # Full implementation
├── plugin_description.xml       # pluginlib registration
├── CMakeLists.txt
└── package.xml
```

---

## How it works

- **No extra executor** — the panel reuses RViz2's own node (obtained via `getDisplayContext()->getRosNodeAbstraction()`). Adding the same node to a second executor would crash RViz2 with `"Node has already been added to an executor"`.
- A `QTimer` at 100 ms polls a mutex-protected `latest_msg_` and drives all UI updates on the Qt main thread.
- Critic rows and chart series are rebuilt automatically whenever the set of critic names changes (e.g., after a parameter reload).

---

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| `Could not load library … undefined symbol: _ZTVN…CriticsPanelE` | `rviz_common` not explicitly linked | Rebuild — fixed in current `CMakeLists.txt` (`rviz_common::rviz_common` in `target_link_libraries`) |
| `Node '/rviz2' has already been added to an executor` → RViz2 crash | Old code spun RViz2's node in a second executor | Rebuild — fixed in current `onInitialize()` |
| Panel shows "waiting…" but topic is publishing | Workspace not sourced correctly | Run `ros2 topic echo /controller_server/critics_stats` to verify; re-source both workspaces |
| `nav2_msgs/msg/critics_stats.hpp: No such file or directory` | `nav2_ws` not sourced before building | `source ~/nav2_ws/install/setup.bash` then rebuild |

---



## License

Apache-2.0 — see [LICENSE](LICENSE) for details.
