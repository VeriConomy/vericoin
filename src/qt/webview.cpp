#include "webview.h"


WebView::WebView(QWidget *parent) :
    QWebView(parent)
{
}

WebView::~WebView()
{
}

void WebView::myBack()
{
    if (this->history()->canGoBack())
        if (this->history()->currentItemIndex() > 1) // 0 is a blank page
            this->back();
}

void WebView::myHome()
{
    if (this->history()->canGoBack())
        this->history()->goToItem(this->history()->itemAt(1)); // 0 is a blank page
}

void WebView::myForward()
{
    if (this->history()->canGoForward())
        this->forward();
}

void WebView::myReload()
{
    this->reload();
}

void WebView::myOpenUrl(QUrl url)
{
    if (isTrustedUrl(url))
    {
        try
        {
            this->load(url);
        }
        catch (...)
        {
            printf("WebView: Error loading: %s\n", url.toString().toStdString().c_str());
        }
    }
    else
    {
        QDesktopServices::openUrl(url);
    }
}

bool WebView::isTrustedUrl(QUrl url)
{
    QList<QString> trustedHosts;

    trustedHosts << "www.vericoin.info" << "vericoin.info";

    if (trustedHosts.contains(url.host()))
        return true;
    else
        return false;
}

void WebView::sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist)
{
    // Show list of all ssl errors
    //foreach (QSslError err, errlist)
    //    printf((QString("sslErrorHandler Url: %1 , Error: %2\n").arg(qnr->url().toString()).arg(err.errorString())).toAscii());

    qnr->ignoreSslErrors();
}
