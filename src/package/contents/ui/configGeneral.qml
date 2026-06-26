import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

Kirigami.FormLayout {
    id: page
    property string title: i18n("General")

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
        Kirigami.FormData.label: i18n("Cardwire Settings")
    }

    CheckBox {
        id: autoApplyGpuStateChk
        Kirigami.FormData.label: i18n("GPU Block:")
        text: i18n("Auto-apply GPU Block in Manual Mode")
        checked: plasmoid.autoApplyGpuState
        onToggled: plasmoid.autoApplyGpuState = checked
    }

    CheckBox {
        id: experimentalNvidiaBlockChk
        text: i18n("Experimental NVIDIA Block")
        checked: plasmoid.experimentalNvidiaBlock
        onToggled: plasmoid.experimentalNvidiaBlock = checked
    }

    CheckBox {
        id: batteryAutoSwitchChk
        Kirigami.FormData.label: i18n("Power Events:")
        text: i18n("Battery Auto-switching")
        checked: plasmoid.batteryAutoSwitch
        onToggled: plasmoid.batteryAutoSwitch = checked
    }

    ComboBox {
        id: batteryAutoSwitchModeCombo
        visible: batteryAutoSwitchChk.checked
        Kirigami.FormData.label: i18n("On AC Mode:")
        model: [i18n("Integrated"), i18n("Hybrid"), i18n("Manual"), i18n("Smart")]
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
