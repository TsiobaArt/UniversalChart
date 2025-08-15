import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

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
    property alias liveText: liveBtnText.text
    property string currentMode: "drag"

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.12, 0.12, 0.12, 1)
        border.color: "#333"
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 6
        spacing: 6

        // Кнопка: Тема
        Rectangle {
            Layout.preferredWidth: 100
            Layout.fillHeight: true
            radius: 6
            border.color: "#444"
            color: "#222"
            Text { anchors.centerIn: parent; text: "Тема" }
            MouseArea { anchors.fill: parent; onClicked: root.themeToggle() }
        }

        // Кнопка: Автозум
        Rectangle {
            Layout.preferredWidth: 120
            Layout.fillHeight: true
            radius: 6
            border.color: "#444"
            color: "#222"
            Text { anchors.centerIn: parent; text: "Автозум" }
            MouseArea { anchors.fill: parent; onClicked: root.autoZoom() }
        }

        // Кнопка: Live toggle
        Rectangle {
            Layout.preferredWidth: 120
            Layout.fillHeight: true
            radius: 6
            border.color: "#444"
            color: "#222"
            Text { id: liveBtnText; anchors.centerIn: parent; text: "Live" }
            MouseArea { anchors.fill: parent; onClicked: root.liveToggle() }
        }

        // Перемикач режимів (Drag / Rect)
        Rectangle {
            Layout.preferredWidth: 160
            Layout.fillHeight: true
            radius: 6
            border.color: "#444"
            color: "#222"
            Row {
                anchors.centerIn: parent
                spacing: 8
                Text { text: "Рука" }
                Rectangle {
                    width: 16; height: 16; radius: 3
                    border.color: "#555"; color: root.currentMode==="drag" ? "#999" : "#222"
                    MouseArea { anchors.fill: parent; onClicked: { root.currentMode="drag"; root.modeChanged("drag"); } }
                }
                Text { text: "Виділення" }
                Rectangle {
                    width: 16; height: 16; radius: 3
                    border.color: "#555"; color: root.currentMode==="rect" ? "#999" : "#222"
                    MouseArea { anchors.fill: parent; onClicked: { root.currentMode="rect"; root.modeChanged("rect"); } }
                }
            }
        }

        // Очистити виділення
        Rectangle {
            Layout.preferredWidth: 180
            Layout.fillHeight: true
            radius: 6
            border.color: "#444"
            color: "#222"
            Text { anchors.centerIn: parent; text: "Очистити виділення" }
            MouseArea { anchors.fill: parent; onClicked: root.clearRequested() }
        }

        Item { Layout.fillWidth: true }
    }
}
