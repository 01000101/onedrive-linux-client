#include <QApplication>
#include <QQmlApplicationEngine>
#include <QNetworkAccessManager>
#include <QByteArray>
#include <QNetworkReply>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <string>
#include <iostream>
#include "auth.h"

#include <QGuiApplication>
#include <QStringList>

#include <qqmlengine.h>
#include <qqmlcontext.h>
#include <qqml.h>
#include <QtQuick/qquickitem.h>
#include <QtQuick/qquickview.h>

// Helper to set loading state
void Auth::setLoading(bool loading) {
    this->isLoading = loading;
    QObject *loadingIndicator = this->rootQMLObj->findChild<QObject*>("loadingIndicator");
    if (loadingIndicator) {
        loadingIndicator->setProperty("visible", loading);
    }
}

// Helper to update breadcrumb path
void Auth::updatePath(QString folderName) {
    if (folderName.isEmpty()) {
        this->currentPath = "/";
    } else if (this->currentPath == "/") {
        this->currentPath = "/" + folderName;
    } else {
        this->currentPath = this->currentPath + "/" + folderName;
    }

    QObject *pathLabel = this->rootQMLObj->findChild<QObject*>("pathLabel");
    if (pathLabel) {
        pathLabel->setProperty("text", this->currentPath);
    }
}

// Helper to update UI elements
void Auth::updateUI() {
    QObject *btn_back = this->rootQMLObj->findChild<QObject*>("btn_back");
    QObject *btn_home = this->rootQMLObj->findChild<QObject*>("btn_home");

    bool canGoBack = !this->navHistory.isEmpty();
    if (btn_back) btn_back->setProperty("enabled", canGoBack);
    if (btn_home) btn_home->setProperty("enabled", canGoBack);
}

void Auth::goBack() {
    if (this->navHistory.isEmpty()) {
        qDebug("Navigation history is empty, cannot go back");
        return;
    }

    FolderState prevState = this->navHistory.pop();
    this->activeView = prevState.contents;
    this->currentFolderId = prevState.folderId;
    this->currentPath = prevState.folderName;

    // Update the model with name and isFolder
    QVariantList itemList;
    for (const QJsonValue &value : std::as_const(this->activeView)) {
        QJsonObject obj = value.toObject();
        QVariantMap item;
        item["name"] = obj["name"].toString();
        item["isFolder"] = obj.contains("folder");
        itemList.append(item);
    }
    this->ctxt->setContextProperty("myModel", QVariant::fromValue(itemList));

    // Update path label
    QObject *pathLabel = this->rootQMLObj->findChild<QObject*>("pathLabel");
    if (pathLabel) {
        pathLabel->setProperty("text", this->currentPath);
    }

    // Update empty folder message
    QObject *emptyLabel = this->rootQMLObj->findChild<QObject*>("emptyFolderLabel");
    if (emptyLabel) {
        emptyLabel->setProperty("visible", itemList.isEmpty());
    }

    updateUI();
    qDebug("Navigated back to: %s", qPrintable(this->currentPath));
}

void Auth::goHome() {
    if (this->navHistory.isEmpty()) {
        return;
    }

    // Clear history and reload root
    this->navHistory.clear();
    this->currentPath = "/";
    this->currentFolderId = "root";

    setLoading(true);

    QNetworkAccessManager *readfolderSocket = new QNetworkAccessManager(this);
    connect(readfolderSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_readfolder(QNetworkReply*)));

    QNetworkRequest request(QUrl(graph_api_base + "/me/drive/root/children"));
    request.setRawHeader("Authorization", "Bearer " + this->access_token);
    readfolderSocket->get(request);
}

void Auth::downloadFile(QString fname) {
    qDebug("Download requested for: %s", qPrintable(fname));

    for (const QJsonValue &value : std::as_const(this->activeView)) {
        QJsonObject obj = value.toObject();

        if (fname == obj["name"].toString()) {
            // Skip folders
            if (obj.contains("folder")) {
                qDebug("Cannot download a folder");
                return;
            }

            // Ask user where to save
            QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + fname;
            QString savePath = QFileDialog::getSaveFileName(nullptr, "Save File", defaultPath);

            if (savePath.isEmpty()) {
                qDebug("Download cancelled by user");
                return;
            }

            this->pendingDownloadPath = savePath;
            this->pendingDownloadName = fname;

            setLoading(true);

            // Request download URL from Graph API
            QNetworkAccessManager *nam = new QNetworkAccessManager(this);
            connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_download(QNetworkReply*)));

            // Graph API: /me/drive/items/{item-id}/content redirects to download URL
            QNetworkRequest request(QUrl(graph_api_base + "/me/drive/items/" + obj["id"].toString() + "/content"));
            request.setRawHeader("Authorization", "Bearer " + this->access_token);
            request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
            nam->get(request);
            return;
        }
    }
    qDebug("File not found for download: %s", qPrintable(fname));
}

void Auth::finished_download(QNetworkReply *reply) {
    setLoading(false);

    if (reply->error() != QNetworkReply::NoError) {
        qDebug("Download error: %s", qPrintable(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();

    QFile file(this->pendingDownloadPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
        qDebug("Downloaded %lld bytes to: %s", data.size(), qPrintable(this->pendingDownloadPath));

        // Show success message
        QObject *lv_folders_details = this->rootQMLObj->findChild<QObject*>("lv_folders_details");
        if (lv_folders_details) {
            lv_folders_details->setProperty("title", "Download Complete");
            lv_folders_details->setProperty("text", "Saved to:\n" + this->pendingDownloadPath);
            QMetaObject::invokeMethod(lv_folders_details, "lv_folders_details_open");
        }
    } else {
        qDebug("Failed to write file: %s", qPrintable(this->pendingDownloadPath));
    }

    this->pendingDownloadPath.clear();
    this->pendingDownloadName.clear();
}

void Auth::openInBrowser(QString fname) {
    qDebug("Open in browser requested for: %s", qPrintable(fname));

    for (const QJsonValue &value : std::as_const(this->activeView)) {
        QJsonObject obj = value.toObject();

        if (fname == obj["name"].toString()) {
            QString webUrl = obj["webUrl"].toString();
            if (!webUrl.isEmpty()) {
                QDesktopServices::openUrl(QUrl(webUrl));
                qDebug("Opening: %s", qPrintable(webUrl));
                return;
            }
        }
    }
    qDebug("Item not found: %s", qPrintable(fname));
}

void Auth::finished_getdetails(QNetworkReply *reply) {
    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();
    qDebug("Details Reply: %s", qPrintable(strReply));

    QString sName = jReply.value("name").toString();
    QString sType = jReply.contains("folder") ? "folder" : "file";
    QString sID = jReply.value("id").toString();
    QString sCreatedTime = jReply.value("createdDateTime").toString();
    QString sUpdatedTime = jReply.value("lastModifiedDateTime").toString();
    qlonglong nSize = jReply.value("size").toVariant().toLongLong();

    // Format size nicely
    QString sSize;
    if (nSize < 1024) {
        sSize = QString::number(nSize) + " B";
    } else if (nSize < 1024 * 1024) {
        sSize = QString::number(nSize / 1024.0, 'f', 1) + " KB";
    } else if (nSize < 1024 * 1024 * 1024) {
        sSize = QString::number(nSize / (1024.0 * 1024.0), 'f', 1) + " MB";
    } else {
        sSize = QString::number(nSize / (1024.0 * 1024.0 * 1024.0), 'f', 2) + " GB";
    }

    QObject *lv_folders_details = this->rootQMLObj->findChild<QObject*>("lv_folders_details");

    if (!lv_folders_details) {
        qDebug("lv_folders_details is NULL");
        return;
    }

    lv_folders_details->setProperty("title", "Details: " + sName);
    lv_folders_details->setProperty("text",
        "Name: " + sName + "\n" +
        "Type: " + sType + "\n" +
        "Size: " + sSize + "\n" +
        "Created: " + sCreatedTime.left(10) + "\n" +
        "Modified: " + sUpdatedTime.left(10));

    QMetaObject::invokeMethod(lv_folders_details, "lv_folders_details_open");
}

void Auth::itemDetails(QString fname) {
    qDebug("Details for: %s", qPrintable(fname));

    for (const QJsonValue &value : std::as_const(this->activeView)) {
        QJsonObject obj = value.toObject();

        if (fname == obj["name"].toString()) {
            QNetworkAccessManager *nam = new QNetworkAccessManager(this);
            connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_getdetails(QNetworkReply*)));

            QNetworkRequest request(QUrl(graph_api_base + "/me/drive/items/" + obj["id"].toString()));
            request.setRawHeader("Authorization", "Bearer " + this->access_token);
            nam->get(request);
            return;
        }
    }
    qDebug("ERROR: Item not found: %s", qPrintable(fname));
}

void Auth::itemSelect(QString fname) {
    qDebug("Selected: %s", qPrintable(fname));

    for (const QJsonValue &value : std::as_const(this->activeView)) {
        QJsonObject obj = value.toObject();

        if (fname == obj["name"].toString()) {
            bool isFolder = obj.contains("folder");
            qDebug(" Type: %s", isFolder ? "folder" : "file");

            if (isFolder) {
                // Save current state to history before navigating
                FolderState currentState;
                currentState.folderId = this->currentFolderId;
                currentState.folderName = this->currentPath;
                currentState.contents = this->activeView;
                this->navHistory.push(currentState);

                // Update current folder info
                this->currentFolderId = obj["id"].toString();
                updatePath(fname);

                setLoading(true);

                QNetworkAccessManager *readfolderSocket = new QNetworkAccessManager(this);
                connect(readfolderSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_readfolder(QNetworkReply*)));

                QNetworkRequest request(QUrl(graph_api_base + "/me/drive/items/" + obj["id"].toString() + "/children"));
                request.setRawHeader("Authorization", "Bearer " + this->access_token);
                readfolderSocket->get(request);
                return;
            } else {
                // For files, open in browser
                openInBrowser(fname);
            }
        }
    }
}

void Auth::finished_readfolder(QNetworkReply *reply) {
    setLoading(false);

    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();
    qDebug("Folder Reply received");

    QJsonArray dataObj = jReply["value"].toArray();

    // Build a list of maps with name and isFolder for QML
    QVariantList itemList;
    for (const QJsonValue &value : std::as_const(dataObj)) {
        QJsonObject obj = value.toObject();
        bool isFolder = obj.contains("folder");
        qDebug("  %s (%s)", qPrintable(obj["name"].toString()), isFolder ? "folder" : "file");

        QVariantMap item;
        item["name"] = obj["name"].toString();
        item["isFolder"] = isFolder;
        itemList.append(item);
    }

    QObject *lv_folders = this->rootQMLObj->findChild<QObject*>("lv_folders");
    QObject *txt_authcode = this->rootQMLObj->findChild<QObject*>("txt_authcode");
    QObject *btn_signin = this->rootQMLObj->findChild<QObject*>("btn_signin");
    QObject *btn_getcode = this->rootQMLObj->findChild<QObject*>("btn_getcode");
    QObject *navBar = this->rootQMLObj->findChild<QObject*>("navBar");
    QObject *pathLabel = this->rootQMLObj->findChild<QObject*>("pathLabel");
    QObject *emptyLabel = this->rootQMLObj->findChild<QObject*>("emptyFolderLabel");

    txt_authcode->setProperty("visible", false);
    btn_signin->setProperty("visible", false);
    btn_getcode->setProperty("visible", false);
    lv_folders->setProperty("visible", true);
    if (navBar) navBar->setProperty("visible", true);
    if (pathLabel) pathLabel->setProperty("text", this->currentPath);
    if (emptyLabel) emptyLabel->setProperty("visible", itemList.isEmpty());

    this->ctxt->setContextProperty("myModel", QVariant::fromValue(itemList));
    this->activeView = dataObj;

    updateUI();
}

void Auth::finished_getquota(QNetworkReply *reply) {
    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();

    QJsonObject quotaObj = jReply.value("quota").toObject();
    qlonglong nTotal = quotaObj.value("total").toVariant().toLongLong();
    qlonglong nUsed = quotaObj.value("used").toVariant().toLongLong();
    qlonglong nRemaining = quotaObj.value("remaining").toVariant().toLongLong();

    int iTotal = (nTotal / (1024*1024));
    int iUsed = (nUsed / (1024*1024));

    QString sTotal = QString::number(nTotal / (1024.0*1024.0*1024.0), 'f', 1) + " GB";
    QString sUsed = QString::number(nUsed / (1024.0*1024.0*1024.0), 'f', 2) + " GB";
    QString sRemaining = QString::number(nRemaining / (1024.0*1024.0*1024.0), 'f', 1) + " GB";

    qDebug("Storage: %s used of %s", qPrintable(sUsed), qPrintable(sTotal));

    QObject *pb_quota = this->rootQMLObj->findChild<QObject*>("pb_quota");
    pb_quota->setProperty("to", iTotal);
    pb_quota->setProperty("from", 0);
    pb_quota->setProperty("value", iUsed);
    pb_quota->setProperty("visible", true);

    QObject *lbl_signin_quota = this->rootQMLObj->findChild<QObject*>("lbl_signin_quota");
    if (lbl_signin_quota) {
        lbl_signin_quota->setProperty("text", sUsed + " / " + sTotal + " used");
        lbl_signin_quota->setProperty("visible", true);
    }
}

void Auth::finished_getuserinfo(QNetworkReply *reply) {
    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();
    QString displayName = jReply["displayName"].toString();
    qDebug("Logged in as: %s", qPrintable(displayName));
}

void Auth::finished_init(QNetworkReply* reply) {
    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();

    if (jReply.contains("error")) {
        qDebug("OAuth Error: %s", qPrintable(jReply["error_description"].toString()));
        setLoading(false);
        return;
    }

    QByteArray access_token = jReply["access_token"].toString().toUtf8();
    qDebug("Authentication successful");
    this->access_token = access_token;

    // Initialize navigation state
    this->currentPath = "/";
    this->currentFolderId = "root";
    this->navHistory.clear();

    QNetworkAccessManager *getuserinfoSocket = new QNetworkAccessManager(this);
    QNetworkAccessManager *getquotaSocket = new QNetworkAccessManager(this);
    QNetworkAccessManager *readfolderSocket = new QNetworkAccessManager(this);

    QNetworkRequest userRequest(QUrl(graph_api_base + "/me"));
    userRequest.setRawHeader("Authorization", "Bearer " + access_token);
    connect(getuserinfoSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_getuserinfo(QNetworkReply*)));
    getuserinfoSocket->get(userRequest);

    QNetworkRequest quotaRequest(QUrl(graph_api_base + "/me/drive"));
    quotaRequest.setRawHeader("Authorization", "Bearer " + access_token);
    connect(getquotaSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_getquota(QNetworkReply*)));
    getquotaSocket->get(quotaRequest);

    QNetworkRequest folderRequest(QUrl(graph_api_base + "/me/drive/root/children"));
    folderRequest.setRawHeader("Authorization", "Bearer " + access_token);
    connect(readfolderSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_readfolder(QNetworkReply*)));
    readfolderSocket->get(folderRequest);
}

void Auth::validateInput(QString username) {
    qDebug("Authenticating...");
    setLoading(true);

    QNetworkAccessManager *nam = new QNetworkAccessManager(this);
    connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_init(QNetworkReply*)));

    QByteArray postData;
    postData.append("client_id=" + api_client_id.toUtf8());
    postData.append("&redirect_uri=" + QUrl::toPercentEncoding(api_redirect_url));
    postData.append("&code=" + username.toUtf8());
    postData.append("&grant_type=authorization_code");
    postData.append("&scope=" + QUrl::toPercentEncoding("Files.Read Files.ReadWrite User.Read offline_access"));

    QNetworkRequest request{QUrl(oauth_token_url)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    nam->post(request, postData);
}

void Auth::getCode() {
    QString authUrl = oauth_authorize_url +
        "?client_id=" + api_client_id +
        "&response_type=code" +
        "&redirect_uri=" + QUrl::toPercentEncoding(api_redirect_url) +
        "&response_mode=query" +
        "&scope=" + QUrl::toPercentEncoding("Files.Read Files.ReadWrite User.Read offline_access") +
        "&state=12345";
    QDesktopServices::openUrl(QUrl(authUrl));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    qDebug("QML Engine Loaded");

    QObject *rootObj = engine.rootObjects().first();
    if (!rootObj) {
        qDebug("QML Root Object not found");
        return -1;
    }

    QObject *btn_signin = rootObj->findChild<QObject*>("btn_signin");
    QObject *btn_getcode = rootObj->findChild<QObject*>("btn_getcode");
    QObject *lv_folders = rootObj->findChild<QObject*>("lv_folders");
    QObject *btn_back = rootObj->findChild<QObject*>("btn_back");
    QObject *btn_home = rootObj->findChild<QObject*>("btn_home");

    Auth auth;
    auth.rootQMLObj = rootObj;
    auth.isLoading = false;
    auth.currentPath = "/";
    auth.currentFolderId = "root";

    QObject::connect(btn_signin, SIGNAL(qmlSignal(QString)), &auth, SLOT(validateInput(QString)));
    QObject::connect(btn_getcode, SIGNAL(getcodeSignal()), &auth, SLOT(getCode()));
    QObject::connect(lv_folders, SIGNAL(itemselectSignal(QString)), &auth, SLOT(itemSelect(QString)));
    QObject::connect(lv_folders, SIGNAL(itemdetailsSignal(QString)), &auth, SLOT(itemDetails(QString)));
    QObject::connect(lv_folders, SIGNAL(downloadSignal(QString)), &auth, SLOT(downloadFile(QString)));
    QObject::connect(lv_folders, SIGNAL(openBrowserSignal(QString)), &auth, SLOT(openInBrowser(QString)));

    if (btn_back) QObject::connect(btn_back, SIGNAL(clicked()), &auth, SLOT(goBack()));
    if (btn_home) QObject::connect(btn_home, SIGNAL(clicked()), &auth, SLOT(goHome()));

    auth.ctxt = engine.rootContext();

    return app.exec();
}
