import QtQuick
import QtQuick.Layouts
import QtQuick.Controls

Rectangle {
    width: 46
    height: 46
    radius: 6
    border.color: butt.hovered ? "lightblue" : "#444"
    color: "#ed0d1216"
    property string source : ""
    property alias butt: butt
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
}
