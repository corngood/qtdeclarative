import QtQuick 2.0
import "optimisedLocalFunctions.js" as Script

QtObject {
    property var v1
    property var v2
    property var v3

    Component.onCompleted: {
        v1 = Script.variable
        v2 = Script.variable2
        v3 = Script.func ()
    }
}
