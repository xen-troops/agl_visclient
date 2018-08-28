import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import QtWebSockets 1.0

import AGL.Demo.Controls 1.0

ApplicationWindow {
    id: root

    Connections {
        target: visClient

        onConnected: {
            visStatus.text = "Connected"
            setButton.enabled = true
            console.log("Connected")
        }

        onError: {
            visStatus.text = message
            console.log("Error: ", message)
        }

        onDisconnected: {
            setButton.enabled = false
            console.log("Disconnected")
        }

        onCarMessageReceived: {
            carMessageList.insert(0, {"message" : message})
            console.log("Car message:", message)
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: root.width / 20

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true

            GridLayout {
                anchors.fill: parent
                columns: 2
                rowSpacing: 20

                Label { text: 'VIS address:' }
                Row {
                    spacing: 20

                    TextField {
                        id: visAddress
                        text: visUrl
                    }

                    Button {
                        id: visButton
                        enabled: true
                        text: "Reconnect"

                        onClicked: {
                            visClient.disconnect()
                            visStatus.text = "Connecting..."
                            visClient.connectTo(visAddress.text)
                        }
                    }
                }

                Label { text: 'VIS status:' }
                Label { id: visStatus }

                Label { text: 'Car message:' }
                Row {
                    spacing: 20

                    TextField {
                        id: carMessage
                    }
                    Button {
                        id: setButton
                        text: "Set"
                        enabled: false

                        onClicked: {
                            visClient.setCarMessage(carMessage.text)
                        }
                    }
                }
            }
        }

        ListView {
            Layout.fillHeight: true
            Layout.fillWidth: true

            model: ListModel{
                id: carMessageList
            }

            delegate: Text {
                color: "yellow"
                font.pixelSize: 40
                text: Qt.formatDateTime(new Date(), "hh:mm:ss") + " : " + message
            }
        }

        Keyboard {
            id: keyboard
            Layout.fillWidth: true
            Layout.fillHeight: true
            z: 99
            target: root.activeFocusControl
        }
    }
}
