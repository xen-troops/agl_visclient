import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.0
import QtWebSockets 1.0

import AGL.Demo.Controls 1.0

ApplicationWindow {
    id: root

    Component.onCompleted: {
        visClient.connectTo(visAddress.text)
    }

    property bool enableButtons : false

    Connections {
        target: visClient

        onConnected: {
            visStatus.text = "Connected"
            enableButtons = true

            visClient.sendMessage(JSON.stringify({
                action: "subscribe",
                path: "Attribute.Car.Message",
                requestId: "subscribeCarMessageId"}))

            visClient.sendMessage(JSON.stringify({
                action: "get",
                path: "Attribute.Vehicle.UserIdentification.Users",
                requestId: "getUsersId"}))

            console.log("Connected")
        }

        onError: {
            visStatus.text = message
            console.log("Error: ", message)
        }

        onDisconnected: {
            enableButtons = false
            console.log("Disconnected")
        }

        onMessageReceived: {
            var jsonMessage = JSON.parse(message)

            if (jsonMessage.hasOwnProperty("error")) {
                visStatus.text = jsonMessage.error.Message
            } else {
                visStatus.text = "Connected"

                if (jsonMessage.action == "subscription") {
                    carMessageList.insert(0, {"message" : jsonMessage.value})
                    console.log("Car message:", jsonMessage.value)
                } else if (jsonMessage.requestId == "getUsersId") {
                    users.text = jsonMessage.value.join(": ")
                }
            }
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
                        text: "Set"
                        enabled: enableButtons

                        onClicked: {
                            visClient.sendMessage(JSON.stringify({
                                action: "set",
                                path: "Attribute.Car.Message",
                                value: carMessage.text,
                                requestId: "setCarMessageId"}))
                        }
                    }
                }

                Label { text: 'Users:' }
                Row {
                    spacing: 20

                    TextField {
                        id: users
                    }
                    Button {
                        text: "Set"
                        enabled: enableButtons

                        onClicked: {
                            visClient.sendMessage(JSON.stringify({
                                action: "set",
                                path: "Attribute.Vehicle.UserIdentification.Users",
                                value: users.text.replace(/\s/g, "").split(":"),
                                requestId: "setUsersId"}))
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
