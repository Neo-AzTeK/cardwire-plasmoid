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

    Plasmoid.status: PlasmaCore.Types.ActiveStatus

    toolTipMainText: i18n("Cardwire GPU Manager")
    toolTipSubText: {
        if (plasmoid.isDaemonFailing) {
            return i18n("Daemon is not running");
        }
        var modeName = "";
        switch (plasmoid.mode) {
            case 0: modeName = i18n("Integrated"); break;
            case 1: modeName = i18n("Hybrid"); break;
            case 2: modeName = i18n("Manual"); break;
            case 3: modeName = i18n("Smart"); break;
            default: modeName = i18n("Manual"); break;
        }
        return i18n("Graphics Mode: %1", modeName);
    }

    compactRepresentation: MouseArea {
        property bool wasExpanded
        onPressed: (mouse) => wasExpanded = mainWindow.expanded
        onClicked: (mouse) => mainWindow.expanded = !wasExpanded
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
        Layout.minimumHeight: Kirigami.Units.gridUnit * 15

        header: PlasmaExtras.PlasmoidHeading {
            visible: !plasmoid.isDaemonFailing
            RowLayout {
                spacing: Kirigami.Units.largeSpacing
                Layout.fillWidth: true

                Repeater {
                    model: plasmoid.gpus
                    delegate: RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        Kirigami.Icon {
                            source: {
                                var powerState = modelData.powerState.trim().toLowerCase();
                                var isActive = (powerState !== "d3cold" && powerState !== "unknown" && powerState !== "suspended");
                                return isActive ? "cardwire-plasmoid-gpu-integrated-active" : "cardwire-plasmoid-gpu-integrated";
                            }
                            Layout.preferredWidth: Kirigami.Units.iconSizes.small
                            Layout.preferredHeight: Kirigami.Units.iconSizes.small
                            opacity: {
                                var powerState = modelData.powerState.trim().toLowerCase();
                                return (powerState === "d3cold" || powerState === "unknown" || powerState === "suspended") ? 0.35 : 1.0;
                            }
                        }

                        PlasmaComponents.Label {
                            text: {
                                var prefix = modelData.isDefault ? i18n("iGPU") : i18n("dGPU");
                                return prefix + ": " + modelData.powerState;
                            }
                            font.bold: true
                            opacity: {
                                var powerState = modelData.powerState.trim().toLowerCase();
                                return (powerState === "d3cold" || powerState === "unknown" || powerState === "suspended") ? 0.6 : 1.0;
                            }
                        }
                    }
                }

                Item { Layout.fillWidth: true }
            }
        }

        // Daemon failing banner
        PlasmaExtras.PlaceholderMessage {
            anchors.centerIn: parent
            width: parent.width - Kirigami.Units.gridUnit * 2
            visible: plasmoid.isDaemonFailing
            iconName: "network-disconnect"
            text: i18n("Cannot connect to cardwired daemon")
            explanation: i18n("Please ensure that cardwired systemd service is active.")
        }

        // Clean graphics switcher list
        ScrollView {
            anchors.fill: parent
            visible: !plasmoid.isDaemonFailing

            ColumnLayout {
                width: availableWidth
                spacing: Kirigami.Units.smallSpacing

                // 1. Modes List
                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing

                    Repeater {
                        model: ListModel {
                            id: modesModel
                            ListElement { modeId: 0; name: "Integrated"; icon: "cardwire-plasmoid-gpu-integrated"; desc: "Blocks discrete GPU to maximize battery life." }
                            ListElement { modeId: 1; name: "Hybrid"; icon: "cardwire-plasmoid-gpu-hybrid"; desc: "Unblocks discrete GPU for on-demand offloading." }
                            ListElement { modeId: 3; name: "Smart"; icon: "cardwire-plasmoid-gpu-smart"; desc: "Dynamically manages offloading using eBPF hooks." }
                            ListElement { modeId: 2; name: "Manual"; icon: "cardwire-plasmoid-gpu-manual"; desc: "Enables manual blocking of individual GPUs." }
                        }

                        delegate: MouseArea {
                            id: modeDelegate
                            Layout.fillWidth: true
                            implicitHeight: layoutRow.implicitHeight + Kirigami.Units.smallSpacing * 2
                            hoverEnabled: true
                            required property int modeId
                            required property string name
                            required property string icon
                            required property string desc

                            // Highlight item on hover or if active
                            Rectangle {
                                anchors.fill: parent
                                color: plasmoid.mode === modeId ? Kirigami.Theme.highlightColor : (parent.containsMouse ? Kirigami.Theme.hoverColor : "transparent")
                                opacity: plasmoid.mode === modeId ? 0.25 : 0.1
                                radius: Kirigami.Units.smallSpacing
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
                                        text: i18n(name)
                                        font.bold: true
                                    }
                                    PlasmaComponents.Label {
                                        text: i18n(desc)
                                        font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                                        opacity: 0.7
                                        wrapMode: Text.Wrap
                                        Layout.fillWidth: true
                                    }
                                }

                                PlasmaComponents.Button {
                                    text: plasmoid.mode === modeId ? i18n("Active") : i18n("Apply")
                                    flat: true
                                    down: plasmoid.mode === modeId
                                    enabled: plasmoid.mode !== modeId
                                    onClicked: (mouse) => plasmoid.setMode(modeId)
                                }
                            }
                        }
                    }
                }

                // 2. Expandable GPU blocking list when Manual Mode is active
                ColumnLayout {
                    visible: plasmoid.mode === 2 // Manual mode
                    Layout.fillWidth: true
                    spacing: Kirigami.Units.smallSpacing
                    Layout.topMargin: Kirigami.Units.gridUnit

                    KSvg.SvgItem {
                        Layout.fillWidth: true
                        height: 2
                        elementId: "horizontal-line"
                        svg: KSvg.Svg { imagePath: "widgets/line" }
                    }

                    PlasmaComponents.Label {
                        text: i18n("Manual GPU Block Control:")
                        font.bold: true
                        Layout.topMargin: Kirigami.Units.smallSpacing
                    }

                    Repeater {
                        model: plasmoid.gpus
                        delegate: RowLayout {
                            Layout.fillWidth: true
                            spacing: Kirigami.Units.gridUnit

                            ColumnLayout {
                                spacing: 2
                                Layout.fillWidth: true
                                PlasmaComponents.Label {
                                    text: modelData.name + (modelData.isDefault ? " (" + i18n("iGPU") + ")" : " (" + i18n("dGPU") + ")")
                                    font.bold: true
                                    elide: Text.ElideRight
                                    Layout.fillWidth: true
                                }
                                PlasmaComponents.Label {
                                    text: i18n("Pci Bus: %1", modelData.pci)
                                    font.pixelSize: Kirigami.Theme.smallFont.pixelSize
                                    opacity: 0.7
                                    Layout.fillWidth: true
                                }
                            }

                            PlasmaComponents.CheckBox {
                                text: i18n("Block")
                                checked: modelData.isBlocked
                                enabled: !modelData.isDefault
                                hoverEnabled: enabled
                                onToggled: modelData.isBlocked = checked
                            }
                        }
                    }
                }
            }
        }
    }
}
