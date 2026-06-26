# Cardwire KDE Plasma Widget

A KDE Plasma 6 widget for managing GPU modes via `cardwired`.

This widget allows you to:
- Switch between GPU modes (**Integrated**, **Hybrid**, **Smart**, **Manual**).
- Dynamic tray icon that updates based on the current mode and discrete GPU power state (Active vs. Suspended).
- View runtime power state (e.g. `D0`, `D3cold`) and block status of each GPU.
- Display the count of active processes running on each GPU, with detailed process names shown on mouse hover.
- Collapsible configuration panel to toggle `auto_apply_gpu_state`, `experimental_nvidia_block`, and `battery_auto_switch` settings.

## Prerequisites

To build and run this plasmoid, your system must have the following dependencies installed:

- **Qt 6** (Gui, Qml, DBus, Core)
- **KDE Frameworks 6** (Config, I18n, Solid, Plasma)
- **Extra CMake Modules (ECM)**
- **CMake** (3.16 or higher)
- **GCC / Clang** supporting C++17

On Arch Linux, you can install them using:
```bash
sudo pacman -S cmake extra-cmake-modules plasma-framework qt6-base qt6-declarative
```

## Build and Installation

1. Create a build directory and run CMake:
   ```bash
   cmake -B build -S . -DCMAKE_INSTALL_PREFIX=/usr
   ```

2. Build the project:
   ```bash
   cmake --build build
   ```

3. Install the applet to the KDE environment:
   ```bash
   sudo cmake --install build
   ```

4. Refresh Plasma or add the widget to your panel/desktop through the standard Plasma widgets explorer.

## Manual Testing

You can run the widget in a standalone test window using the KDE Plasma utility:
```bash
plasmoidviewer -a dev.opengamingcollective.cardwire
```
