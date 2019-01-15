import QtQuick 2.3
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.2
import QtQuick.Layouts 1.3

Dialog {
    visible: true
    title: "HueLightbulb"
    standardButtons: StandardButton.Ok | StandardButton.Cancel

    onAccepted: {
        var vn = registry.deserialize(context, JSON.stringify({
            type: "HueOutputNode",
            host: hostField.text,
            username: usernameField.text,
            secret: secretField.text,
            group: groupField.text,
            width: widthField.text,
        }));
        if (vn) {
            graph.insertVideoNode(vn);
        } else {
            console.log("Could not instantiate HueOutputNode");
        }
    }

    ColumnLayout {
        anchors.fill: parent

        Label {
            text: "Hue Bridge IP"
        }
        TextField {
            id: hostField
            Layout.fillWidth: true

            Component.onCompleted: {
                hostField.forceActiveFocus();
            }
        }

        Label {
            text: "Username"
        }
        TextField {
            id: usernameField
            Layout.fillWidth: true
        }
        Label {
            text: "Client Secret"
        }
        TextField {
            id: secretField
            Layout.fillWidth: true
        }
        Label {
            text: "Group"
        }
        TextField {
            id: groupField
            Layout.fillWidth: true
        }
        Label {
            text: "Width"
        }
        TextField {
            id: widthField
            Layout.fillWidth: true
        }
    }
}
