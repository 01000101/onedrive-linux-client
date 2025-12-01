#ifndef AUTH_H
#define AUTH_H

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QNetworkReply>
#include <QQmlContext>
#include <QJsonArray>
#include <QStack>
#include <QDesktopServices>
#include <QFile>
#include <QStandardPaths>
#include <QFileDialog>

// Microsoft Azure AD / Microsoft Identity Platform credentials
const QString api_client_id = "YOUR_APPLICATION_ID_HERE";
// For desktop/mobile apps, use this special redirect URI
const QString api_redirect_url = "https://login.microsoftonline.com/common/oauth2/nativeclient";

// Microsoft Graph API base URL (replaces apis.live.net/v5.0)
const QString graph_api_base = "https://graph.microsoft.com/v1.0";

// OAuth2 endpoints (Microsoft Identity Platform v2.0)
const QString oauth_authorize_url = "https://login.microsoftonline.com/common/oauth2/v2.0/authorize";
const QString oauth_token_url = "https://login.microsoftonline.com/common/oauth2/v2.0/token";

struct FolderState {
    QString folderId;
    QString folderName;
    QJsonArray contents;
};

class Auth : public QObject
{
    Q_OBJECT

public:
    QObject *rootQMLObj;
    QQmlContext *ctxt;
    QJsonArray activeView;
    QByteArray access_token;

    // Navigation history
    QStack<FolderState> navHistory;
    QString currentPath;
    QString currentFolderId;
    bool isLoading;

    // Download state
    QString pendingDownloadPath;
    QString pendingDownloadName;

public slots:
    void validateInput(QString username);
    void getCode();
    void itemSelect(QString fname);
    void itemDetails(QString fname);
    void goBack();
    void goHome();
    void downloadFile(QString fname);
    void openInBrowser(QString fname);
    void finished_init(QNetworkReply* reply);
    void finished_getuserinfo(QNetworkReply* reply);
    void finished_getquota(QNetworkReply* reply);
    void finished_readfolder(QNetworkReply* reply);
    void finished_getdetails(QNetworkReply* reply);
    void finished_download(QNetworkReply* reply);

private:
    void setLoading(bool loading);
    void updatePath(QString folderName);
    void updateUI();
};


#endif // AUTH_H
