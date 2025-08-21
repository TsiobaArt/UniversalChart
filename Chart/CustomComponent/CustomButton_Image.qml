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
    ToolButton {
        id: butt
        icon.source: source
        background: Item { }
        width:  parent.width
        height: parent.height
        // icon.color: "#fb2c2e32"
        // flat: true
        // highlighted: true
        icon.color: butt.hovered ? "lightblue" : "#444"
        icon.width: width
        icon.height: height
        // зміна кольору
    }

    Rectangle {
        width: butt.width
        radius: 6
        height: 2
        color: butt.checked ?  "lightblue" : "transparent"
        anchors.bottom: parent.top
        anchors.bottomMargin: 2
    }
}
