#ifndef AUTH_H
#define AUTH_H

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QNetworkReply>
#include <QQmlContext>
#include <QJsonArray>

const QString api_client_id = "00000000603E0BFE";
const QString api_client_secret = "qXipuPomaauItsIsmwtKZ2YacGZtCyXD";
const QString api_redirect_url = "https://joscor.com/onedriverest";

class Auth : public QObject
{
    Q_OBJECT

public:
    QObject *rootQMLObj;
    QQmlContext *ctxt;
    QJsonArray activeView;
    QByteArray access_token;

public slots:
    void validateInput(QString username);
    void getCode();
    void itemSelect(QString fname);
    void itemDetails(QString fname);
    void finished_init(QNetworkReply* reply);
    void finished_getuserinfo(QNetworkReply* reply);
    void finished_getquota(QNetworkReply* reply);
    void finished_readfolder(QNetworkReply* reply);
    void finished_getdetails(QNetworkReply* reply);
};


#endif // AUTH_H
