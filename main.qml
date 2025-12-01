import QtQuick
import QtQuick.Controls
import QtQuick.Dialogs
import QtQuick.Layouts

ApplicationWindow {
    id: page_auth
    visible: true
    width: 800
    height: 600
    color: "#2b2b2b"
    title: qsTr("OneDrive Linux Client")

    // Custom About Dialog with fixed size
    Dialog {
        id: aboutDialog
        title: "About OneDrive Linux Client"
        width: 400
        height: 200
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok

        contentItem: Rectangle {
            color: "#363636"
            Label {
                anchors.fill: parent
                anchors.margins: 20
                text: "OneDrive Linux Client v0.2\n\n" +
                      "By Joscor Technical Research\n\n" +
                      "Browse your Microsoft OneDrive files on Linux.\n" +
                      "Updated for Microsoft Graph API and Qt6."
                color: "#FFFFFF"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        background: Rectangle {
            color: "#363636"
            border.color: "#505050"
            radius: 8
        }
    }

    menuBar: MenuBar {
        Menu {
            title: qsTr("File")
            Action {
                text: qsTr("Exit")
                onTriggered: Qt.quit()
            }
        }
        Menu {
            title: qsTr("Help")
            Action {
                text: qsTr("About")
                onTriggered: aboutDialog.open()
            }
        }
    }

    // Header area
    Rectangle {
        id: header
        width: parent.width
        height: 100
        color: "#2b2b2b"
        z: 10

        Label {
            id: lbl_title
            color: "#FFFFFF"
            text: qsTr("OneDrive")
            anchors.top: parent.top
            anchors.topMargin: 15
            anchors.horizontalCenter: parent.horizontalCenter
            font.bold: true
            font.pointSize: 28
        }

        // Storage quota bar
        ProgressBar {
            id: pb_quota
            objectName: "pb_quota"
            width: parent.width * 0.6
            height: 20
            anchors.top: lbl_title.bottom
            anchors.topMargin: 10
            anchors.horizontalCenter: parent.horizontalCenter
            from: 0
            to: 100
            value: 0
            visible: false

            background: Rectangle {
                implicitWidth: parent.width
                implicitHeight: 20
                radius: 10
                color: "#404040"
                border.color: "#0078d4"
                border.width: 1
            }

            contentItem: Item {
                implicitWidth: parent.width
                implicitHeight: 20

                Rectangle {
                    width: pb_quota.visualPosition * parent.width
                    height: parent.height
                    radius: 10
                    color: "#0078d4"
                }
            }

            Text {
                anchors.centerIn: parent
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                color: "#FFFFFF"
                font.pointSize: 9
                id: lbl_signin_quota
                objectName: "lbl_signin_quota"
            }
        }
    }

    // Login controls (initially visible)
    TextField {
        id: txt_authcode
        objectName: "txt_authcode"
        width: 300
        height: 35
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -20
        placeholderText: qsTr("Paste authorization code here")
        font.pointSize: 11
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: txt_authcode.bottom
        anchors.topMargin: 15
        spacing: 15

        Button {
            id: btn_getcode
            objectName: "btn_getcode"
            text: qsTr("Get Code")
            width: 120
            signal getcodeSignal()
            onClicked: btn_getcode.getcodeSignal()
        }

        Button {
            id: btn_signin
            objectName: "btn_signin"
            text: qsTr("Sign In")
            width: 120
            signal qmlSignal(string username)
            onClicked: btn_signin.qmlSignal(txt_authcode.text)
        }
    }

    // Navigation bar (hidden until logged in)
    Rectangle {
        id: navBar
        objectName: "navBar"
        width: parent.width
        height: 45
        anchors.top: header.bottom
        color: "#363636"
        visible: false
        z: 5

        Row {
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.verticalCenter: parent.verticalCenter
            spacing: 8

            Button {
                id: btn_back
                objectName: "btn_back"
                text: "<"
                width: 40
                height: 32
                enabled: false
                font.bold: true
                font.pointSize: 14

                background: Rectangle {
                    color: btn_back.enabled ? (btn_back.pressed ? "#0078d4" : "#505050") : "#404040"
                    radius: 4
                }

                contentItem: Text {
                    text: btn_back.text
                    color: btn_back.enabled ? "#FFFFFF" : "#808080"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    font: btn_back.font
                }
            }

            Button {
                id: btn_home
                objectName: "btn_home"
                text: "Home"
                width: 60
                height: 32
                enabled: false

                background: Rectangle {
                    color: btn_home.enabled ? (btn_home.pressed ? "#0078d4" : "#505050") : "#404040"
                    radius: 4
                }

                contentItem: Text {
                    text: btn_home.text
                    color: btn_home.enabled ? "#FFFFFF" : "#808080"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
            }

            Rectangle {
                width: 1
                height: 25
                color: "#505050"
            }

            Text {
                id: pathLabel
                objectName: "pathLabel"
                text: "/"
                color: "#CCCCCC"
                font.pointSize: 11
                anchors.verticalCenter: parent.verticalCenter
                width: navBar.width - 180
                elide: Text.ElideMiddle
            }
        }
    }

    // Loading indicator
    Rectangle {
        id: loadingIndicator
        objectName: "loadingIndicator"
        anchors.fill: parent
        color: "#80000000"
        visible: false
        z: 100

        BusyIndicator {
            anchors.centerIn: parent
            running: parent.visible
            width: 60
            height: 60
        }

        Text {
            anchors.centerIn: parent
            anchors.verticalCenterOffset: 50
            text: "Loading..."
            color: "#FFFFFF"
            font.pointSize: 12
        }

        MouseArea {
            anchors.fill: parent
            // Block clicks while loading
        }
    }

    // Empty folder message
    Text {
        id: emptyFolderLabel
        objectName: "emptyFolderLabel"
        anchors.centerIn: parent
        text: "This folder is empty"
        color: "#808080"
        font.pointSize: 14
        visible: false
        z: 2
    }

    // File/Folder grid
    GridView {
        id: lv_folders
        objectName: "lv_folders"
        width: parent.width - 20
        anchors.top: navBar.bottom
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.horizontalCenter: parent.horizontalCenter
        clip: true
        keyNavigationWraps: true
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.VerticalFlick
        visible: false
        cellWidth: 130
        cellHeight: 140

        signal itemselectSignal(string fname)
        signal itemdetailsSignal(string fname)
        signal downloadSignal(string fname)
        signal openBrowserSignal(string fname)

        delegate: Rectangle {
            id: itemDelegate
            width: 120
            height: 130
            color: mouseArea.containsMouse ? "#404040" : "#363636"
            border.width: 1
            border.color: "#2b2b2b"
            radius: 8

            // Access model data - now includes isFolder property
            property string itemName: modelData.name || modelData
            property bool itemIsFolder: modelData.isFolder || false

            Column {
                anchors.centerIn: parent
                spacing: 8

                // Icon - using simple shapes that render on all systems
                Rectangle {
                    width: 50
                    height: 50
                    anchors.horizontalCenter: parent.horizontalCenter
                    color: "transparent"

                    // Folder icon
                    Rectangle {
                        visible: itemIsFolder
                        anchors.centerIn: parent
                        width: 44
                        height: 34
                        color: "#f0c040"
                        radius: 3

                        // Folder tab
                        Rectangle {
                            width: 18
                            height: 8
                            color: "#f0c040"
                            radius: 2
                            anchors.bottom: parent.top
                            anchors.left: parent.left
                            anchors.leftMargin: 4
                            anchors.bottomMargin: -2
                        }
                    }

                    // File icon
                    Rectangle {
                        visible: !itemIsFolder
                        anchors.centerIn: parent
                        width: 36
                        height: 44
                        color: "#FFFFFF"
                        radius: 2
                        border.color: "#AAAAAA"
                        border.width: 1

                        // Dog-ear corner
                        Rectangle {
                            width: 10
                            height: 10
                            color: "#DDDDDD"
                            anchors.top: parent.top
                            anchors.right: parent.right
                        }

                        // File type label
                        Text {
                            anchors.centerIn: parent
                            anchors.verticalCenterOffset: 5
                            text: {
                                var name = itemName.toLowerCase();
                                var dotIndex = name.lastIndexOf(".");
                                if (dotIndex === -1) return "";
                                var ext = name.substring(dotIndex + 1).toUpperCase();
                                if (ext.length > 4) ext = ext.substring(0, 4);
                                return ext;
                            }
                            color: {
                                var name = itemName.toLowerCase();
                                var dotIndex = name.lastIndexOf(".");
                                if (dotIndex === -1) return "#666666";
                                var ext = name.substring(dotIndex);
                                if ([".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp", ".svg"].indexOf(ext) >= 0) return "#4CAF50"; // Green - images
                                if ([".mp4", ".avi", ".mov", ".mkv", ".wmv", ".webm"].indexOf(ext) >= 0) return "#9C27B0"; // Purple - video
                                if ([".mp3", ".wav", ".flac", ".aac", ".ogg", ".m4a"].indexOf(ext) >= 0) return "#FF9800"; // Orange - audio
                                if ([".pdf"].indexOf(ext) >= 0) return "#F44336"; // Red - PDF
                                if ([".doc", ".docx", ".odt", ".rtf", ".txt"].indexOf(ext) >= 0) return "#2196F3"; // Blue - docs
                                if ([".xls", ".xlsx", ".csv", ".ods"].indexOf(ext) >= 0) return "#4CAF50"; // Green - spreadsheets
                                if ([".zip", ".rar", ".7z", ".tar", ".gz"].indexOf(ext) >= 0) return "#795548"; // Brown - archives
                                return "#666666"; // Gray - other
                            }
                            font.pointSize: 8
                            font.bold: true
                        }
                    }
                }

                // Filename
                Text {
                    width: 110
                    text: itemName
                    color: "#FFFFFF"
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WrapAtWordBoundaryOrAnywhere
                    maximumLineCount: 2
                    elide: Text.ElideRight
                    font.pointSize: 10
                }
            }

            MouseArea {
                id: mouseArea
                anchors.fill: parent
                hoverEnabled: true
                acceptedButtons: Qt.LeftButton | Qt.RightButton
                onClicked: function(mouse) {
                    if (mouse.button === Qt.RightButton) {
                        contextMenu.popup()
                    } else {
                        lv_folders.itemselectSignal(itemName)
                    }
                }
                onDoubleClicked: lv_folders.itemselectSignal(itemName)
            }

            Menu {
                id: contextMenu

                Action {
                    text: "Open"
                    onTriggered: lv_folders.itemselectSignal(itemName)
                }
                Action {
                    text: "Open in Browser"
                    onTriggered: lv_folders.openBrowserSignal(itemName)
                }
                MenuSeparator {}
                Action {
                    text: "Download"
                    enabled: !itemIsFolder
                    onTriggered: lv_folders.downloadSignal(itemName)
                }
                MenuSeparator {}
                Action {
                    text: "Details"
                    onTriggered: lv_folders.itemdetailsSignal(itemName)
                }
            }
        }
        model: myModel

        ScrollBar.vertical: ScrollBar {
            active: true
            policy: ScrollBar.AsNeeded
        }
    }

    // Details/notification dialog
    Dialog {
        id: lv_folders_details
        objectName: "lv_folders_details"
        width: 350
        height: 220
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok

        property string text: ""

        contentItem: Rectangle {
            color: "#363636"
            Label {
                anchors.fill: parent
                anchors.margins: 15
                text: lv_folders_details.text
                color: "#FFFFFF"
                wrapMode: Text.WordWrap
                font.pointSize: 10
            }
        }

        background: Rectangle {
            color: "#363636"
            border.color: "#505050"
            radius: 8
        }

        function lv_folders_details_open() {
            lv_folders_details.open()
        }
    }
}
