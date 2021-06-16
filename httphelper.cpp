//  Copyright © 2011  Vinícius dos Santos Oliveira

#include "httphelper.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include <qt-json/json.h>

using namespace JsonRPC;

HttpHelper::HttpHelper(QObject *parent) :
    QObject(parent),
    peer(new Peer(this)),
    httpClient(new QNetworkAccessManager(this))
{
    connect(peer, &Peer::readyRequestMessage, this, &HttpHelper::onReadyRequestMessage);
    connect(peer, &Peer::readyResponse, this, &HttpHelper::readyResponse);
    connect(peer, &Peer::requestError, this, &HttpHelper::requestError);

    connect(httpClient, &QNetworkAccessManager::finished, this, &HttpHelper::replyFinished);
}

QUrl HttpHelper::url() const
{
    return m_url;
}

void HttpHelper::setUrl(const QUrl &url)
{
    this->m_url = url;
}

bool HttpHelper::call(const QString &method, const QVariant &params, const QVariant &id)
{
    return peer->call(method, params, id);
}

void HttpHelper::onReadyRequestMessage(const QByteArray &json)
{
    QNetworkRequest request(m_url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      QString("application/json-rpc"));
    request.setRawHeader("Accept", "application/json-rpc");

    httpClient->post(request, json);
}

void HttpHelper::replyFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    QByteArray content = reply->readAll();

    if (reply->error() != QNetworkReply::NoError) {
        bool ok;
        QtJson::parse(QString::fromUtf8(content), ok);

        if (!ok) {
            emit error(reply->error());
            return;
        }
    }

    peer->handleMessage(content);
}
