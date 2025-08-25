import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    id: rootRec
    width: 46
    height: 46
    radius: 6
    border.color: butt.hovered ? "lightblue" : "#444"
    color: "#ed0d1216"
    property string source : ""
    property alias butt: butt

    property string tooltip: ""
    property bool   tipsEnabled: false

    scale: butt.pressed ? 0.9 : 1
    Behavior on scale {
          NumberAnimation { duration: 50; easing.type: Easing.InOutQuad }
      }
    ToolButton {
        id: butt
        icon.source: source
        background: Item { }
        width:  parent.width
        height: parent.height
        autoExclusive: false
        // icon.color: "#fb2c2e32"
        // flat: true
        // highlighted: true
        // icon.color: butt.hovered ? "lightblue" : "#444"
        icon.color: butt.checked
                       ? "lightblue"
                       : (butt.hovered ? "lightblue" : "#444")

        icon.width: width
        icon.height: height
        // scale: butt.pressed ? 0.9 : 1
    }

    ToolTip {
        id: tip
        // показуємо лише коли треба
        visible: tipsEnabled && butt.hovered && tooltip.length > 0
        text: tooltip
        delay: 180
        timeout: 2000
        // padding: 8
        font.pixelSize: 13
        // parent: Overlay.overlay
        parent: butt

        // Регульований відступ під кнопкою
        property int offsetY: 100000
        property int offsetX: 0

        // ПІД кнопкою, ПО ЦЕНТРУ — чисті біндинги (перерахуються при зміні offsetX/Y)
        x: {
            const p = butt.mapToItem(parent, butt.width/2, butt.height); // центр низу кнопки
            return Math.round(p.x - tip.implicitWidth/2 + offsetX);
        }
        y: {
            const p = butt.mapToItem(parent, 0, butt.height);
            return Math.round(p.y + offsetY);
        }

        // системний вигляд можна залишити; нижче лише текст
        contentItem: Text {
            text: tip.text
            color: "white"
            wrapMode: Text.WordWrap
            font.pixelSize: 13
            width: 220                   // макс. ширина тултіпа
        }

        // «стрілочка» зверху (бо тултіп під кнопкою), по центру тултіпа
        background: Rectangle {
            radius: 6
            color: "#1f1f1f"
            border.color: "lightblue"
            border.width: 1

            Rectangle {
                anchors.horizontalCenter: parent.horizontalCenter
                y: -5
                width: 10; height: 10
                rotation: 45
                color: parent.color
                border.color: parent.border.color
                border.width: parent.border.width
            }
        }
    }

}
