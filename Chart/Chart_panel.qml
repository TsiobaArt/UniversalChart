import QtQuick
import QtQuick.Layouts
import QtQuick.Controls
import "CustomComponent"
Item {
    id: root
    // QML диктує бажаний розмір:
    implicitWidth: 640
    implicitHeight: 56
    signal themeToggle()
    signal autoZoom()
    signal liveToggle()
    signal modeChanged(string mode)   // "drag" | "rect"
    signal clearRequested()
    // alias для зміни тексту кнопки Live з C++
    // property alias liveText: liveBtnText.text
    property string currentMode: "drag"
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
        anchors.centerIn: parent
        CustomButton_Image {
            id: buttDrag
            source: "qrc:/Icon/drag.svg"
            butt.checkable: true
            butt.checked: true

        }

        CustomButton_Image {
            id: buttZoomArea
            source: "qrc:/Icon/zoom4.svg"
            butt.checkable: true
        }
        CustomButton_Image {
            id: buttExpend
            source: "qrc:/Icon/expand.svg"
        }
        CustomButton_Image {
            id: buttIconHome
            source: "qrc:/Icon/home.svg"
        }
        CustomButton_Image {
            id: buttLive
            source: "qrc:/Icon/live.svg"
        }
        CustomButton_Image {
            id: buttClearCheckBox
            source: "qrc:/Icon/clear.svg"
        }
        CustomButton_Image {
            id: buttExportCsv
            source: "qrc:/Icon/csv.svg"
        }
        CustomButton_Image {
            id: buttSaveChart
            source: "qrc:/Icon/image.svg"
        }
        CustomButton_Image {
            id: buttTheme
            source: "qrc:/Icon/theme.svg"
        }
    }
}



// import QtQuick
// import QtQuick.Layouts
// import QtQuick.Controls

// Item {
//     id: root
//     // QML диктує бажаний розмір:
//     implicitWidth: 640
//     implicitHeight: 56
//     signal themeToggle()
//     signal autoZoom()
//     signal liveToggle()
//     signal modeChanged(string mode)   // "drag" | "rect"
//     signal clearRequested()

//     // alias для зміни тексту кнопки Live з C++
//     property alias liveText: liveBtnText.text
//     property string currentMode: "drag"

//     Rectangle {
//         anchors.fill: parent
//         color: appWindowColor
//         border.color: "#333"
//     }

//     RowLayout {
//         anchors.fill: parent
//         anchors.margins: 6
//         spacing: 6

//         // Кнопка: Тема
//         Rectangle {
//             Layout.preferredWidth: 100
//             Layout.fillHeight: true
//             radius: 6
//             border.color: "#444"
//             color: "#222"
//             Text { anchors.centerIn: parent; text: "Тема" }
//             MouseArea { anchors.fill: parent; onClicked: root.themeToggle() }
//         }

//         // Кнопка: Автозум
//         Rectangle {
//             Layout.preferredWidth: 120
//             Layout.fillHeight: true
//             radius: 6
//             border.color: "#444"
//             color: "#222"
//             Text { anchors.centerIn: parent; text: "Автозум" }
//             MouseArea { anchors.fill: parent; onClicked: root.autoZoom() }
//         }

//         // Кнопка: Live toggle
//         Rectangle {
//             Layout.preferredWidth: 120
//             Layout.fillHeight: true
//             radius: 6
//             border.color: "#444"
//             color: "#222"
//             Text { id: liveBtnText; anchors.centerIn: parent; text: "Live" }
//             MouseArea { anchors.fill: parent; onClicked: root.liveToggle() }
//         }

//         // Перемикач режимів (Drag / Rect)
//         Rectangle {
//             Layout.preferredWidth: 160
//             Layout.fillHeight: true
//             radius: 6
//             border.color: "#444"
//             color: "#222"
//             Row {
//                 anchors.centerIn: parent
//                 spacing: 8
//                 Text { text: "Рука" }
//                 Rectangle {
//                     width: 16; height: 16; radius: 3
//                     border.color: "#555"; color: root.currentMode==="drag" ? "#999" : "#222"
//                     MouseArea { anchors.fill: parent; onClicked: { root.currentMode="drag"; root.modeChanged("drag"); } }
//                 }
//                 Text { text: "Виділення" }
//                 Rectangle {
//                     width: 16; height: 16; radius: 3
//                     border.color: "#555"; color: root.currentMode==="rect" ? "#999" : "#222"
//                     MouseArea { anchors.fill: parent; onClicked: { root.currentMode="rect"; root.modeChanged("rect"); } }
//                 }
//             }
//         }

//         // Очистити виділення
//         Rectangle {
//             Layout.preferredWidth: 180
//             Layout.fillHeight: true
//             radius: 6
//             border.color: "#444"
//             color: "#222"
//             Text { anchors.centerIn: parent; text: "Очистити виділення" }
//             MouseArea { anchors.fill: parent; onClicked: root.clearRequested() }
//         }


//         Button {
//             text: ""
//             icon.source: "qrc:/Icon/theme.svg"
//             icon.width: 18
//             icon.height: 18
//             icon.color: hovered ? "#4CC2FF" : "#E6E6E6"   // зміна кольору
//         }

//         Item { Layout.fillWidth: true }
//     }
// }
