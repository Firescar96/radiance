import QtQuick 2.7
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import radiance 1.0
import "."

VideoNodeTile {
    id: tile;

    normalHeight: 400;
    normalWidth: 300;

    ColumnLayout {
        anchors.fill: parent;
        anchors.margins: 15;

        RowLayout {
            Label {
                Layout.fillWidth: true;
                text: "its an output";
                color: "#ddd";
                elide: Text.ElideMiddle;
            }
        
            Label {
                text: attachedParameter >= 0 ? "#" + attachedParameter : "";
                color: "#ddd";
            }
        }

        Item {
            Layout.preferredHeight: width;
            Layout.fillWidth: true;
            layer.enabled: true;

            CheckerboardBackground {
                anchors.fill: parent;
            }
            VideoNodePreview {
                id: vnr;
                anchors.fill: parent;
                previewAdapter: Globals.previewAdapter;
                videoNodeId: tile.videoNode ? tile.videoNode.id : 0;
            }
        }

        ComboBox {
            id: screenSelector;
            model: tile.videoNode.availableScreens;
        }

        RowLayout {
            Layout.fillWidth: true
            CheckBox {
                id: visibleCheck
                checked: tile.videoNode ? tile.videoNode.visible : false;
                Binding {
                    target: tile.videoNode
                    property: "visible"
                    value: visibleCheck.checked
                }
                style: CheckBoxStyle {
                    label: Text {
                        color: "#ddd"
                        text: "Visible"
                    }
                }
            }
        }
    }
}
