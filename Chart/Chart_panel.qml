import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "CustomComponent"
import QtQuick.Dialogs

Item {
    id: root
    // QML диктує бажаний розмір:
    implicitWidth: 640
    implicitHeight: 56
    signal themeToggle()
    signal autoZoom()
    signal liveButt(bool mode, int step)
    signal modeChanged(string mode)   // "drag" | "rect"
    signal clearSelection();
    signal liveCount(int count);
    signal exportCsv(string path)
    signal exportImage(string path)
    signal deleteDataChart();
    signal autoZoomX();
    // alias для зміни тексту кнопки Live з C++
    // property alias liveText: liveBtnText.text
    property string currentMode: "drag"
    // Діалог збереження CSV
    FileDialog {
        id: csvSaveDialog
        title: "Зберегти як CSV"
        fileMode: FileDialog.SaveFile
        nameFilters: ["CSV Files (*.csv)", "All Files (*)"]
        defaultSuffix: "csv"
        onAccepted: {
            // FileDialog повертає url. Для C++ краще передати як рядок-URL: "file:///path/..."
            root.exportCsv(selectedFile)   // selectedFile – це URL (string)
        }
    }

    FileDialog {
        id: imageSaveDialog
        title: "Зберегти зображення"
        fileMode: FileDialog.SaveFile
        nameFilters: ["PNG (*.png)", "JPEG (*.jpg)", "PDF (*.pdf)", "BMP (*.bmp)"]
        defaultSuffix: "png"
        onAccepted: root.exportImage(selectedFile)   // URL рядок "file://..."
    }

    Connections { // C++ натискання правою кнопкою мишкою в режимі перетягуванння
        target: chartPanel
        function onRightClickDrag() {
            buttLive.butt.checked = false;
        }
    }

    Rectangle {
        anchors.fill: parent
        color: appWindowColor
        border.color: "#333"
    }
    ButtonGroup {
        id: modeGroup
        buttons: [buttDrag.butt, buttZoomArea.butt]
    }

    Row {
        // anchors.fill: parent
        // anchors.margins: 6
        spacing: 6
        // anchors.centerIn: parent
        anchors.left: parent.left
        anchors.leftMargin: 30
        anchors.verticalCenter: parent.verticalCenter
        CustomButton_Image {
            id: buttDrag
            source: "qrc:/Icon/drag.svg"
            butt.checkable: true
            butt.checked: true
            tooltip: "Режим перетягування (Drag)"
            tipsEnabled: buttToopTips.butt.checked ? true : false
            butt.onClicked: {
                root.currentMode="drag";
                root.modeChanged("drag"); }
        }

        CustomButton_Image {
            id: buttZoomArea
            source: "qrc:/Icon/zoom4.svg"
            butt.checkable: true
            tooltip: "Режим зумування (Rect)"
            tipsEnabled: buttToopTips.butt.checked ? true : false
            butt.onClicked: {
                root.currentMode="rect";
                root.modeChanged("rect"); }
        }
        CustomButton_Image {
            id: buttLive
            source: "qrc:/Icon/live.svg"
            butt.checkable: true
            butt.autoExclusive: false
            tooltip: "Показати останні точки"
            tipsEnabled: buttToopTips.butt.checked ? true : false
            Component.onCompleted: butt.checked = false
            butt.onCheckedChanged: {
                root.liveButt(butt.checked, liveCountSpin.value)
            }
        }
        SpinBox {
            id: liveCountSpin
            from: 5
            to: 100000
            value: 100
            stepSize: 1
            editable: true
            width: buttLive.width +10
            height: buttLive.height
            enabled: !buttLive.butt.checked
            opacity: !buttLive.butt.checked ? 1 : 0.9

            background: Rectangle {
                radius: 4
                color: "#2b2b2b"
                border.color: buttLive.butt.checked ? "lightblue" : "#555"
            }
        }

        CustomButton_Image {
            id: buttExpend
            source: "qrc:/Icon/expand.svg"
            tipsEnabled: buttToopTips.butt.checked ? true : false
            tooltip: "ЛКМ автозум ХУ, ПКМ автозум X"

            butt.onClicked: {
            root.autoZoom();
            }
            MouseArea {
                anchors.fill: buttExpend.butt      // лише поверх самої кнопки
                acceptedButtons: Qt.RightButton
                onClicked: (mouse) => {
                    // Тут і так буде RightButton, але хай буде явна перевірка
                    if (mouse.button === Qt.RightButton)
                        root.autoZoomX();
                }
            }
        }

        CustomButton_Image {
            id: buttClearCheckBox
            source: "qrc:/Icon/clear.svg"
            tipsEnabled: buttToopTips.butt.checked ? true : false
            tooltip: "Очистити чекбокси"
            butt.onClicked:  {
                root.clearSelection();
            }
        }
        CustomButton_Image {
            id: buttExportCsv
            source: "qrc:/Icon/csv.svg"
            tipsEnabled: buttToopTips.butt.checked ? true : false
            tooltip: "Зберегти вибране в csv"
            butt.onClicked: {
            csvSaveDialog.open()
            }

        }
        CustomButton_Image {
            id: buttSaveChart
            source: "qrc:/Icon/image.svg"
            tipsEnabled: buttToopTips.butt.checked ? true : false
            tooltip: "Зберегти графік як картинку"
            butt.onClicked: {
            imageSaveDialog.open()
            }
        }
        CustomButton_Image {
            id: buttTheme
            source: "qrc:/Icon/theme.svg"
            tipsEnabled: buttToopTips.butt.checked ? true : false
            tooltip: "Тема"
        }
        CustomButton_Image {
            id: buttDelete
            source: "qrc:/Icon/delete.svg"
            tipsEnabled: buttToopTips.butt.checked ? true : false
            tooltip: "Очистити дані з графіка"
            butt.onClicked: {
            deleteDataChart();
            }
        }
        CustomButton_Image {
            id: buttToopTips
            source: "qrc:/Icon/info.svg"
            butt.checkable: true
            butt.autoExclusive: false
            tipsEnabled: true
            tooltip: "Включити підказки"
            Component.onCompleted: butt.checked = false
        }
    }
}
