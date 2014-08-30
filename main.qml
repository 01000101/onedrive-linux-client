import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Dialogs 1.1
import QtQuick.Controls.Styles 1.2

ApplicationWindow {
    id: page_auth
    visible: true
    width: 720
    height: 480
    color: "#343434"
    title: qsTr("OneDrive Linux Client")

    MessageDialog {
        id: messageDialog
        title: "About OneDrive Linux Client 0.1a"
        text: "The OneDrive Linux Client by Joscor Technical Research allows users running Linux" +
              " and Mac access to their Microsoft OneDrive files through a native app."
        onAccepted: {
            Qt.quit()
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            MenuItem {
                text: qsTr("Exit")
                onTriggered: Qt.quit();
            }
        }
        Menu {
            title: qsTr("Help")
            MenuItem {
                text: qsTr("About")
                onTriggered: messageDialog.open()
            }
        }
    }

    Label {
        id: lbl_title
        color: "#FFFFFF"
        text: qsTr("OneDrive Linux Client")
        z: 2
        anchors.top: parent.top
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 0
        font.bold: true
        font.pointSize: 36
    }

    TextField {
        id: txt_authcode
        objectName: "txt_authcode"
        x: -132
        y: -106
        width: 253
        height: 25
        anchors.verticalCenterOffset: -32
        anchors.horizontalCenterOffset: 1
        placeholderText: qsTr("Authorization Code")
        anchors.centerIn: parent
    }

    Button {
        id: btn_signin
        objectName: "btn_signin"
        x: -48
        y: -29
        text: qsTr("Sign In")
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenterOffset: 49
        anchors.centerIn: parent
        visible: true

        signal qmlSignal(string username)

        MouseArea {
            anchors.fill: parent
            onClicked: {
                btn_signin.qmlSignal(txt_authcode.text)
            }
        }
    }

    Button {
        id: btn_getcode
        objectName: "btn_getcode"
        x: -85
        y: -15
        text: qsTr("Get Code")

        anchors.verticalCenterOffset: 0
        anchors.horizontalCenterOffset: -42
        anchors.centerIn: parent
        visible: true

        signal getcodeSignal()

        MouseArea {
            anchors.fill: parent
            onClicked: {
                btn_getcode.getcodeSignal()
            }
        }
    }


    GridView {
        id: lv_folders
        objectName: "lv_folders"
        width: parent.width
        //height: parent.height
        keyNavigationWraps: true
        boundsBehavior: Flickable.StopAtBounds
        interactive: true
        flickableDirection: Flickable.VerticalFlick
        layoutDirection: Qt.LeftToRight
        z: 1
        visible: false
        anchors.top: pb_quota.bottom
        anchors.topMargin: 20
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: (lv_folders.width % lv_folders.cellWidth) / 2
        cellWidth: 120
        cellHeight: 120

        signal itemselectSignal(string fname)
        signal itemdetailsSignal(string fname)

        delegate: Rectangle {
            width: 120
            height: 120
            color: "#454545"
            border.width: 1
            border.color: "#343434"
            radius: 5

            Text {
                width: 115
                height: 115
                text: modelData
                anchors.centerIn: parent;
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                color: "#FFFFFF"
            }

            MouseArea {
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: {
                    if( mouse.button == Qt.RightButton ) {
                        lv_folders_contextmenu.popup()
                    } else if( mouse.button == Qt.LeftButton ) {
                        lv_folders.itemselectSignal(modelData)
                    }
                }
            }

            Menu {
                id: lv_folders_contextmenu

                MenuItem {
                    text: qsTr("Details")
                    onTriggered: lv_folders.itemdetailsSignal(modelData)
                }
            }
        }
        model: myModel
    }

    MessageDialog {
        id: lv_folders_details
        objectName: "lv_folders_details"
        onAccepted: {
            lv_folders_details.close()
        }
        function lv_folders_details_open() {
            lv_folders_details.open();
        }
    }

    ProgressBar {
        id: pb_quota
        objectName: "pb_quota"
        width: parent.width * 0.7
        height: 23
        z: 2
        indeterminate: false
        anchors.top: lbl_title.bottom
        anchors.topMargin: 20
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.horizontalCenterOffset: 0
        minimumValue: 0
        visible: false

        style: ProgressBarStyle {
            background: Rectangle {
                radius: 10
                color: "#343434"
                border.color: "steelblue"
                border.width: 1


            }

            progress: Rectangle {
                color: "steelblue"
                radius: 10
            }
        }

        Text {
            anchors.centerIn: parent;
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            color: "#FFFFFF"
            id: lbl_signin_quota
            objectName: "lbl_signin_quota"
        }
    }

    Rectangle {
        id: rectangle1
        width: parent.width

        color: "#343434"
        visible: true
        anchors.verticalCenterOffset: -170
        anchors.horizontalCenterOffset: 0
        anchors.top: parent.top
        anchors.bottom: pb_quota.bottom
        anchors.bottomMargin: -2

        z: 1
        border.width: 0
    }

}
