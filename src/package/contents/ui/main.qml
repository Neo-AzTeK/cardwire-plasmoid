/*
    SPDX-FileCopyrightText: 2026 Open Gaming Collective <info@opengamingcollective.github.io>
    SPDX-License-Identifier: GPL-3.0
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

PlasmoidItem {
    id: mainWindow
    Plasmoid.icon: plasmoid.iconName
    switchWidth: Kirigami.Units.gridUnit * 12
    switchHeight: Kirigami.Units.gridUnit * 12

    Binding {
        target: plasmoid
        property: "status"

        // Determine tray icon active status dynamically
        function isPlasmoidActive() {
            if (plasmoid.isDaemonFailing) {
                return PlasmaCore.Types.ActiveStatus;
            }
            // Check if any discrete GPU is active (D0)
            var dGpuActive = false;
            for (var i = 0; i < plasmoid.gpus.length; ++i) {
                var gpu = plasmoid.gpus[i];
                if (!gpu.isDefault) {
                    var powerState = gpu.powerState.trim().toLowerCase();
                    if (powerState !== "d3cold" && powerState !== "unknown") {
                        dGpuActive = true;
                    }
                }
            }
            return dGpuActive ? PlasmaCore.Types.ActiveStatus : PlasmaCore.Types.PassiveStatus;
        }

        value: isPlasmoidActive()
    }

    toolTipMainText: i18n("Cardwire GPU Manager")
    toolTipSubText: {
        if (plasmoid.isDaemonFailing) {
            return i18n("Daemon is not running");
        }
        var modeName = "";
        switch (plasmoid.mode) {
            case 0: modeName = "Integrated"; break;
            case 1: modeName = "Hybrid"; break;
            case 2: modeName = "Manual"; break;
            case 3: modeName = "Smart"; break;
            default: modeName = "Manual"; break;
        }
        return i18n("Graphics Mode: %1", modeName);
    }

    compactRepresentation: MouseArea {
        property bool wasExpanded
        onPressed: wasExpanded = mainWindow.expanded
        onClicked: mainWindow.expanded = !wasExpanded
        hoverEnabled: true

        Kirigami.Icon {
            anchors.fill: parent
            source: plasmoid.icon
            active: parent.containsMouse
        }
    }

    fullRepresentation: PlasmaExtras.Representation {
        id: dialog
        anchors.fill: parent
        Layout.minimumWidth: Kirigami.Units.gridUnit * 18
        Layout.minimumHeight: Kirigami.Units.gridUnit * 22

        header: PlasmaExtras.PlasmoidHeading {
            visible: !plasmoid.isDaemonFailing
            ColumnLayout {
                spacing: Kirigami.Units.smallSpacing
                Layout.fillWidth: true

                RowLayout {
                    Layout.fillWidth: true
                    PlasmaComponents.Label {
                        text: i18n("Cardwire Status")
                        font.bold: true
                        font.pixelSize: Kirigami.Theme.defaultFont.pixelSize * 1.2
                    }
                    Item { Layout.fillWidth: true }
                    PlasmaComponents.ToolButton {
                        icon.name: "view-refresh"
                        text: i18n("Refresh Devices")
                        onClicked: plasmoid.refreshDevices()
                    }
                }

                // Collapsible settings
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 0

                    PlasmaComponents.Button {
                        id: configToggle
                        text: expanded ? i18n("Hide Config Options") : i18n("Show Config Options")
                        icon.name: expanded ? "arrow-up" : "settings-configure"
                        flat: true
                        Layout.fillWidth: true
                        property bool expanded: false
                        onClicked: expanded = !expanded
                    }

                    ColumnLayout {
                        visible: configToggle.expanded
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.gridUnit
                        Layout.bottomMargin: Kirigami.Units.smallSpacing
                        spacing: Kirigami.Units.smallSpacing

                        PlasmaComponents.CheckBox {
                            text: i18n("Auto-apply GPU Block in Manual Mode")
                            checked: plasmoid.autoApplyGpuState
                            onToggled: plasmoid.autoApplyGpuState = checked
                        }

                        PlasmaComponents.CheckBox {
                            text: i18n("Experimental NVIDIA Block")
                            checked: plasmoid.experimentalNvidiaBlock
                            onToggled: plasmoid.experimentalNvidiaBlock = checked
                        }

                        PlasmaComponents.CheckBox {
                            id: batterySwitchChk
                            text: i18n("Battery Auto-switching")
                            checked: plasmoid.batteryAutoSwitch
                            onToggled: plasmoid.batteryAutoSwitch = checked
                        }

                        RowLayout {
                            visible: batterySwitchChk.checked
                            Layout.leftMargin: Kirigami.Units.gridUnit
                            PlasmaComponents.Label {
                                text: i18n("On AC Mode:")
                            }
                            ComboBox {
                                model: ["Integrated", "Hybrid", "Manual", "Smart"]
                                currentIndex: {
                                    switch (plasmoid.batteryAutoSwitchMode) {
                                        case 0: return 0;
                                        case 1: return 1;
                                        case 2: return 2;
                                        case 3: return 3;
                                        default: return 1;
                                    }
                                }
                                onActivated: {
                                    var modesMap = [0, 1, 2, 3];
                                    plasmoid.batteryAutoSwitchMode = modesMap[currentIndex];
                                }
                            }
                        }
                    }
                }
            }
        }

        ColumnLayout {
            anchors.fill: parent
            spacing: Kirigami.Units.smallSpacing

            // Daemon failing banner
            PlasmaExtras.PlaceholderMessage {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: plasmoid.isDaemonFailing
                iconName: "network-disconnect"
                text: i18n("Cannot connect to cardwired daemon")
                explanation: i18n("Please ensure that cardwired systemd service is active.")
            }

            // Normal UI
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: !plasmoid.isDaemonFailing

                ColumnLayout {
                    width: parent.width - Kirigami.Units.gridUnit
                    spacing: Kirigami.Units.gridUnit

                    // GPU status list
                    PlasmaComponents.Label {
                        text: i18n("Detected GPUs:")
                        font.bold: true
                    }

                    Repeater {
                        model: plasmoid.gpus
                        delegate: RowLayout {
                            id: gpuDelegate
                            Layout.fillWidth: true
                            spacing: Kirigami.Units.gridUnit
                            required property var modelData

                            Kirigami.Icon {
                                source: modelData.isDefault ? "computer-laptop" : "video-card"
                                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                            }

                            ColumnLayout {
                                spacing: 2
                                PlasmaComponents.Label {
                                    text: modelData.name
                                    font.bold: true
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                                PlasmaComponents.Label {
                                    text: i18n("PCI: %1 | %2", modelData.pci, modelData.isDefault ? i18n("Default") : i18n("Discrete"))
                                    font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                                    opacity: 0.7
                                }
                                PlasmaComponents.Label {
                                    text: i18n("Power State: %1", modelData.powerState)
                                    font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                                    opacity: 0.7
                                }
                                PlasmaComponents.Label {
                                    text: modelData.appCount > 0 ? i18np("%1 active process", "%1 active processes", modelData.appCount) : i18n("No active processes")
                                    font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                                    font.italic: true
                                    opacity: 0.8
                                    color: modelData.appCount > 0 ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor

                                    // Hover processes list tooltip
                                    ToolTip {
                                        visible: maHover.containsMouse && modelData.appCount > 0
                                        text: modelData.appDetails
                                    }

                                    MouseArea {
                                        id: maHover
                                        anchors.fill: parent
                                        hoverEnabled: true
                                    }
                                }
                            }

                            Item { Layout.fillWidth: true }

                            PlasmaComponents.CheckBox {
                                visible: !modelData.isDefault && plasmoid.mode === 2 // Only toggle discrete GPU in manual mode
                                text: i18n("Block")
                                checked: modelData.isBlocked
                                onToggled: modelData.isBlocked = checked
                            }
                        }
                    }

                    // Separation line
                    KSvg.SvgItem {
                        Layout.fillWidth: true
                        height: 2
                        elementId: "horizontal-line"
                        svg: KSvg.Svg { imagePath: "widgets/line" }
                    }

                    // Mode selections
                    PlasmaComponents.Label {
                        text: i18n("Choose Mode:")
                        font.bold: true
                    }

                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        Repeater {
                            model: ListModel {
                                id: modesModel
                                ListElement { modeId: 0; name: "Integrated"; icon: "cardwire-plasmoid-gpu-integrated"; desc: "Blocks discrete GPU completely to maximize battery life." }
                                ListElement { modeId: 1; name: "Hybrid"; icon: "cardwire-plasmoid-gpu-hybrid"; desc: "Unblocks discrete GPU for on-demand performance offloading." }
                                ListElement { modeId: 3; name: "Smart"; icon: "cardwire-plasmoid-gpu-smart"; desc: "Dynamically whitelists heavy apps/games via eBPF hooks." }
                                ListElement { modeId: 2; name: "Manual"; icon: "cardwire-plasmoid-gpu-manual"; desc: "Enables manual blocking of individual GPUs." }
                            }

                            delegate: MouseArea {
                                Layout.fillWidth: true
                                height: layoutRow.implicitHeight + Kirigami.Units.gridUnit
                                hoverEnabled: true
                                id: modeDelegate
                                required property int modeId
                                required property string name
                                required property string icon
                                required property string desc

                                // Highlight current selected mode
                                Rectangle {
                                    anchors.fill: parent
                                    color: plasmoid.mode === modeId ? Kirigami.Theme.highlightColor : (parent.containsMouse ? Kirigami.Theme.hoverColor : "transparent")
                                    opacity: plasmoid.mode === modeId ? 0.3 : 0.1
                                    radius: 4
                                }

                                RowLayout {
                                    id: layoutRow
                                    anchors.fill: parent
                                    anchors.margins: Kirigami.Units.smallSpacing
                                    spacing: Kirigami.Units.gridUnit

                                    Kirigami.Icon {
                                        source: icon
                                        Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                                        Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                                    }

                                    ColumnLayout {
                                        spacing: 2
                                        Layout.fillWidth: true
                                        PlasmaComponents.Label {
                                            text: name
                                            font.bold: true
                                        }
                                        PlasmaComponents.Label {
                                            text: desc
                                            font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                                            opacity: 0.7
                                            wrapMode: Text.Wrap
                                            Layout.fillWidth: true
                                        }
                                    }

                                    PlasmaComponents.Button {
                                        text: i18n("Apply")
                                        flat: true
                                        enabled: plasmoid.mode !== modeId
                                        onClicked: plasmoid.setMode(modeId)
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
