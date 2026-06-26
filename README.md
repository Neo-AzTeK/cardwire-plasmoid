# Cardwire KDE Plasma Widget

A modern, high-performance KDE Plasma 6 widget for monitoring and switching GPU modes via `cardwired`.

This widget provides seamless control over hybrid and multi-GPU setups on Linux, communicating asynchronously with the system daemon to ensure a zero-lag desktop experience.

---

- **GPU Mode Switcher:** Switch on-the-fly between **Integrated**, **Hybrid**, **Smart**, and **Manual** rendering modes directly from a clean tray popup.
- **Dynamic Tray Icon:** Instantly reflects your current active GPU mode and discrete GPU power state (Active vs. Suspended).
- **System Tray Visibility Configuration:** Configure the widget icon to automatically show or hide in the system tray based on AC power status and GPU activity, matching `supergfxctl-plasmoid` visibility options.
- **Daemon Configuration:** Configure options like `auto_apply_gpu_state`, `experimental_nvidia_block`, and `battery_auto_switch` from the standard KDE Plasma widget settings panel.

---

## Documentation

For deep technical details and advanced deployment guides, refer to:
- **[Architecture & Design](file:///home/mixcraftio/Code/Repos/GPU/cardwire-plasmoid/docs/architecture.md):** Deep-dive into the reentrant `QProcess` queue, sysfs monitoring, and QML layout architecture.
- **[Detailed Installation & Deployment](file:///home/mixcraftio/Code/Repos/GPU/cardwire-plasmoid/docs/installation_and_deployment.md):** Build options, testing, manual instructions, and troubleshooting.

---

## Installation (Arch Linux)

On Arch Linux, the recommended way to install and manage the widget is using the provided [PKGBUILD](file:///home/mixcraftio/Code/Repos/GPU/cardwire-plasmoid/PKGBUILD). This builds the package from source and registers it with `pacman` for clean updates and uninstallation.

### 1. Install Build Dependencies
```bash
sudo pacman -S cmake extra-cmake-modules libplasma qt6-base qt6-declarative kconfig ki18n solid cardwire
```

### 2. Build and Install via PKGBUILD
In the root directory of the repository, run:
```bash
makepkg -si
```

### 3. Uninstalling
To cleanly uninstall the widget and remove all files, run:
```bash
sudo pacman -R cardwire-plasmoid
```

---

## Manual Installation (Other Distros)

If you are not using Arch Linux, you can compile and install the widget using CMake directly:

```bash
# 1. Configure the build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr

# 2. Build the widget
cmake --build build

# 3. Install to the system
sudo cmake --install build
```

---

## Local Development & Testing

To run the widget in a standalone window for debugging without adding it to your system panel, use `plasmoidviewer`:

```bash
plasmoidviewer -a dev.opengamingcollective.cardwire
```
