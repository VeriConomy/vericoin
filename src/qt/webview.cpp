#include "webview.h"
#include "util.h"

#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/join.hpp>
#include <stdarg.h>

using namespace std;

bool fTrustedUrlsSet = false;

WebView::WebView(QWidget *parent) :
    QWebView(parent)
{
    trustedUrls << "www.vericoin.info" << "vericoin.info";  // These will get appended to by the values in VERSION.json
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

    if (!fTrustedUrlsSet)
    {
        if (!GetArg("-vTrustedUrls", "").empty())
        {
            std::string urls = GetArg("-vTrustedUrls", "");
            typedef vector<string> parts_type;
            parts_type parts;
            boost::split(parts, urls, boost::is_any_of(",; "), boost::token_compress_on);
            for (vector<string>::iterator it = parts.begin(); it != parts.end(); ++it)
            {
                QString url = QString::fromStdString(*it);
                // Sanity check
                if (url.contains(QChar('.')))
                {
                    trustedUrls << url;
                }
            }
            fTrustedUrlsSet = true;
        }
    }

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
    if (trustedUrls.contains(url.host()))
        return true;
    else
        return false;
}

void WebView::sslErrorHandler(QNetworkReply* qnr, const QList<QSslError> & errlist)
{
    qnr->ignoreSslErrors();
}
