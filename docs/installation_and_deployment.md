# Installation and Deployment Guide

This document describes how to build, install, package, and test the [cardwire-plasmoid](file:///home/mixcraftio/Code/Repos/GPU/cardwire-plasmoid) widget.

---

## 1. Native Build and Installation

### Prerequisites
Ensure your system has the required Qt6 and KF6 development packages installed:
* **Qt 6:** `Qt6Qml`, `Qt6Gui`, `Qt6Core`
* **KF6:** `KF6Config`, `KF6I18n`, `KF6Solid`
* **Plasma 6:** `Plasma` (Applet development headers)
* **System Daemon:** `cardwire` (switcher backend and command-line tool)
* **Build tools:** `cmake`, `extra-cmake-modules` (ECM)

On **Arch Linux**:
```bash
sudo pacman -S cmake extra-cmake-modules plasma-workspace qt6-base qt6-declarative
```

### Manual Compilation
Run the following commands in the root of the project:

```bash
# 1. Configure build files
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr

# 2. Build the target
cmake --build build

# 3. Install the files to the system directories
sudo cmake --install build
```

The compiled binary module `dev.opengamingcollective.cardwire.so` is installed to `${KDE_INSTALL_QTPLUGINDIR}/plasma/applets/` (usually `/usr/lib/qt6/plugins/plasma/applets/`), and QML configurations are installed to `/usr/share/plasma/plasmoids/dev.opengamingcollective.cardwire/`.

---

## 2. Arch Linux PKGBUILD Distribution

If you are using Arch Linux, you can package the widget using `makepkg` (which is the recommended approach to allow `pacman` to manage the widget lifecycle, making updates and clean uninstallation simple).

We provide a [PKGBUILD](file:///home/mixcraftio/Code/Repos/GPU/cardwire-plasmoid/PKGBUILD) in the root of the repository.

To package and install locally using the PKGBUILD:
```bash
# In the root of the repository
makepkg -si
```

This will automatically:
1. Resolve build-time and runtime dependencies.
2. Compile the widget using release flags.
3. Install the packaged widget to your system using `pacman`.

To uninstall the package at any time:
```bash
sudo pacman -R cardwire-plasmoid-git
```

---

## 3. Standalone Developer Testing

To test the widget without adding it to your system panel, you can launch it in a standalone window using KDE's `plasmoidviewer` utility:

```bash
plasmoidviewer -a dev.opengamingcollective.cardwire
```

If you make QML styling edits and want to test them immediately, reinstall the package and relaunch `plasmoidviewer`.
