#ifndef WEBVIEW_H
#define WEBVIEW_H

#include <QWebView>
#include <QWebHistory>
#include <QDesktopServices>
#include <QNetworkReply>
#include <QList>
#include <QUrl>

namespace Ui {
class WebView;
}

class WebView : public QWebView
{
    Q_OBJECT

public:
    explicit WebView(QWidget *parent = 0);
    ~WebView();

public slots:
    void myBack();
    void myHome();
    void myForward();
    void myReload();
    void myOpenUrl(QUrl url);
    bool isTrustedUrl(QUrl url);
    void sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist);

private:
    Ui::WebView *ui;

    QList<QString> trustedUrls;
};

#endif // WEBVIEW_H
