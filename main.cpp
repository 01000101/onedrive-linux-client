#include <QApplication>
#include <QQmlApplicationEngine>
#include <QNetworkAccessManager>
#include <QByteArray>
#include <QNetworkReply> // HTTP GET/POST
#include <QDesktopServices> // Open page in browser
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


void Auth::finished_getdetails(QNetworkReply *reply) {
    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();
    qDebug("Reply: %s", qPrintable(strReply));

    // Get the stats in bytes
    QString sName = jReply.value("name").toString();
    QString sType = jReply.value("type").toString();
    QString sID = jReply.value("id").toString();
    QString sCreatedTime = jReply.value("created_time").toString();
    QString sUpdatedTime = jReply.value("updated_time").toString();
    qlonglong nSize = jReply.value("size").toVariant().toLongLong();
    QString sSize = QString::number((int)nSize);

    QObject *lv_folders_details = this->rootQMLObj->findChild<QObject*>("lv_folders_details");

    if( !lv_folders_details )
        qDebug("lv_folders_details is NULL");

    lv_folders_details->setProperty("title", "Details for " + sName.toUtf8());
    lv_folders_details->setProperty("text",
        "Name: " + sName.toUtf8() + "<br />" +
        "Type: " + sType.toUtf8() + "<br />" +
        "ID: " + sID.toUtf8() + "<br />" +
        "Created: " + sCreatedTime.toUtf8() + "<br />" +
        "Updated: " + sUpdatedTime.toUtf8() + "<br />" +
        "Size: " + sSize.toUtf8() + " bytes<br />");

    QMetaObject::invokeMethod(lv_folders_details, "lv_folders_details_open");
}

void Auth::itemDetails(QString fname) {
    qDebug("fname: %s", qPrintable(fname));

    foreach (const QJsonValue & value, this->activeView) {
        QJsonObject obj = value.toObject();

        if( fname == obj["name"].toString() ) {
            qDebug("File/Folder found: %s", qPrintable(obj["id"].toString()));
            qDebug(" Type: %s", qPrintable(obj["type"].toString()));

            QNetworkAccessManager *readfolderSocket = new QNetworkAccessManager(this);
            connect(readfolderSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_getdetails(QNetworkReply*)));
            readfolderSocket->get(QNetworkRequest(QUrl("https://apis.live.net/v5.0/" + obj["id"].toString().toUtf8() + "?access_token=" + this->access_token)));
            return;
        }
    }

    qDebug("ERROR: Invalid fname given to Auth::itemDetails");
    qDebug("  fname = %s", qPrintable(fname));
}

void Auth::itemSelect(QString fname) {
    qDebug("fname: %s", qPrintable(fname));

    foreach (const QJsonValue & value, this->activeView) {
        QJsonObject obj = value.toObject();

        if( fname == obj["name"].toString() ) {
            qDebug("File/Folder found: %s", qPrintable(obj["id"].toString()));
            qDebug(" Type: %s", qPrintable(obj["type"].toString()));

            if( obj["type"] == "folder" ||
                obj["type"] == "album" ) {
                QNetworkAccessManager *readfolderSocket = new QNetworkAccessManager(this);
                connect(readfolderSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_readfolder(QNetworkReply*)));
                readfolderSocket->get(QNetworkRequest(QUrl("https://apis.live.net/v5.0/" + obj["id"].toString().toUtf8() + "/files?access_token=" + this->access_token)));
                return;
            }
        }
    }

    qDebug("ERROR: Invalid fname given to Auth::itemSelect");
    qDebug("  fname: %s", qPrintable(fname));
}

void Auth::finished_readfolder(QNetworkReply *reply) {
    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();
    qDebug("Reply: %s", qPrintable(strReply));

    QJsonArray dataObj = jReply["data"].toArray();
    int i = 0;

    QStringList dataList;
    foreach (const QJsonValue & value, dataObj) {
        QJsonObject obj = value.toObject();

        qDebug("Item #%s: %s", qPrintable(QString::number(i++)), qPrintable(obj["name"].toString()));

        dataList.append(obj["name"].toString());
    }

    QObject *lv_folders = this->rootQMLObj->findChild<QObject*>("lv_folders");
    QObject *txt_authcode = this->rootQMLObj->findChild<QObject*>("txt_authcode");
    QObject *btn_signin = this->rootQMLObj->findChild<QObject*>("btn_signin");
    QObject *btn_getcode = this->rootQMLObj->findChild<QObject*>("btn_getcode");
    txt_authcode->setProperty("visible", false);
    btn_signin->setProperty("visible", false);
    btn_getcode->setProperty("visible", false);
    lv_folders->setProperty("visible", true);
    this->ctxt->setContextProperty("myModel", QVariant::fromValue(dataList));

    this->activeView = dataObj;
}

void Auth::finished_getquota(QNetworkReply *reply) {
    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();

    // Get the stats in bytes
    qlonglong nQuota = jReply.value("quota").toVariant().toLongLong();
    qlonglong nAvailable = jReply.value("available").toVariant().toLongLong();
    qlonglong nUsed = (nQuota - nAvailable);

    int iQuota = (nQuota / (1024*1024));
    int iUsed = (nUsed / (1024*1024));

    // Convert the stats to MB and then to readable string
    QString sQuota = QString::number(nQuota / (1024*1024));
    QString sAvailable = QString::number(nAvailable / (1024*1024));
    QString sUsed = QString::number(nUsed / (1024*1024));

    qDebug("Reply: %s", qPrintable(strReply));
    qDebug("quota: %s", qPrintable(sQuota));
    qDebug("available: %s", qPrintable(sAvailable));
    qDebug("used: %s", qPrintable(sUsed));

    QObject *pb_quota = this->rootQMLObj->findChild<QObject*>("pb_quota");
    pb_quota->setProperty("maximumValue", iQuota);
    pb_quota->setProperty("minimumValue", 0);
    pb_quota->setProperty("value", iUsed);
    pb_quota->setProperty("visible", true);

    QObject *lbl_signin_quota = this->rootQMLObj->findChild<QObject*>("lbl_signin_quota");
    if( lbl_signin_quota ) {
        qDebug("lbl_signin_quota is OK");
        lbl_signin_quota->setProperty("text", sUsed + " / " + sQuota + " MB used, " + sAvailable + " MB free");
        lbl_signin_quota->setProperty("visible", true);
    }
}

void Auth::finished_getuserinfo(QNetworkReply *reply) {
    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();
    QByteArray username = jReply["name"].toString().toUtf8();
    qDebug("Reply: %s", qPrintable(strReply));
}

void Auth::finished_init(QNetworkReply* reply) {
    QString strReply = (QString)reply->readAll();
    QJsonDocument jDoc = QJsonDocument::fromJson(strReply.toUtf8());
    QJsonObject jReply = jDoc.object();
    QByteArray access_token = jReply["access_token"].toString().toUtf8();
    //qDebug("Reply: " + strReply.toUtf8());
    qDebug("Access Token: %s", qPrintable(access_token));
    this->access_token = access_token;

    QNetworkAccessManager *getuserinfoSocket = new QNetworkAccessManager(this);
    QNetworkAccessManager *getquotaSocket = new QNetworkAccessManager(this);
    QNetworkAccessManager *readfolderSocket = new QNetworkAccessManager(this);

    connect(getuserinfoSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_getuserinfo(QNetworkReply*)));
    getuserinfoSocket->get(QNetworkRequest(QUrl("https://apis.live.net/v5.0/me?access_token=" + access_token)));

    connect(getquotaSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_getquota(QNetworkReply*)));
    getquotaSocket->get(QNetworkRequest(QUrl("https://apis.live.net/v5.0/me/skydrive/quota?access_token=" + access_token)));

    connect(readfolderSocket, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_readfolder(QNetworkReply*)));
    readfolderSocket->get(QNetworkRequest(QUrl("https://apis.live.net/v5.0/me/skydrive/files?access_token=" + access_token)));
}

void Auth::validateInput(QString username) {
    qDebug("Auth::validateInput(%s)", qPrintable(username));

    QNetworkAccessManager *nam =
        new QNetworkAccessManager(this);

    connect(nam, SIGNAL(finished(QNetworkReply*)), this, SLOT(finished_init(QNetworkReply*)));

    QByteArray postData;
    postData.append("client_id=" + api_client_id);
    postData.append("&redirect_uri=" + api_redirect_url);
    postData.append("&client_secret=" + api_client_secret);
    postData.append("&code=" + username.toUtf8());
    postData.append("&grant_type=authorization_code");
    qDebug("%s", qPrintable(postData));
    nam->post(QNetworkRequest(QUrl("https://login.live.com/oauth20_token.srf")),
              postData);
}

void Auth::getCode() {
    QDesktopServices::openUrl(
        QUrl("https://login.live.com/oauth20_authorize.srf?client_id=" + api_client_id + "&scope=wl.signin%20wl.skydrive%20wl.basic%20wl.offline_access&response_type=code&display=touch&redirect_uri=" + api_redirect_url));
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:///main.qml")));

    qDebug("QML Engine Loaded");

    QObject *rootObj = engine.rootObjects().first();
    if( !rootObj )
        qDebug("QML Root Object not found");


    QObject *btn_signin = rootObj->findChild<QObject*>("btn_signin");
    QObject *btn_getcode = rootObj->findChild<QObject*>("btn_getcode");
    QObject *lv_folders = rootObj->findChild<QObject*>("lv_folders");

    // Bind a slot
    Auth auth;
    auth.rootQMLObj = rootObj;

    QObject::connect(btn_signin,
                     SIGNAL(qmlSignal(QString)),
                     &auth,
                     SLOT(validateInput(QString)));

    QObject::connect(btn_getcode,
                     SIGNAL(getcodeSignal()),
                     &auth,
                     SLOT(getCode()));

    QObject::connect(lv_folders,
                     SIGNAL(itemselectSignal(QString)),
                     &auth,
                     SLOT(itemSelect(QString)));

    QObject::connect(lv_folders,
                     SIGNAL(itemdetailsSignal(QString)),
                     &auth,
                     SLOT(itemDetails(QString)));

    auth.ctxt = engine.rootContext();

    return app.exec();
}
