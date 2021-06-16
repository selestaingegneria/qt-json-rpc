//  Copyright © 2011  Vinícius dos Santos Oliveira

#include "tcphelper.h"
#include <QTcpSocket>
#include <QDataStream>

using namespace JsonRPC;

TcpHelper::TcpHelper(QObject *parent) :
    QObject(parent),
    peer(nullptr),
    socket(nullptr),
    nextMessageSize(0)
{
}

bool TcpHelper::setSocket(QTcpSocket *socket)
{
    if (this->socket)
        onDisconnected();

    if (socket && socket->state() == QAbstractSocket::ConnectedState) {
        socket->setParent(this);

        connect(socket, &QTcpSocket::readyRead, this, &TcpHelper::onReadyRead);
        connect(socket, &QTcpSocket::disconnected, this, &TcpHelper::onDisconnected);


        peer = new Peer(this);

        connect(peer, &Peer::readyRequestMessage, this, &TcpHelper::onReadyMessage);
        connect(peer, &Peer::readyResponseMessage, this, &TcpHelper::onReadyMessage);

        connect(peer, &Peer::readyResponse, this, &TcpHelper::readyResponse);
        connect(peer, &Peer::requestError, this, &TcpHelper::requestError);

        connect(peer,
                SIGNAL(readyRequest(QSharedPointer<JsonRPC::ResponseHandler>)),
                this,
                SIGNAL(readyRequest(QSharedPointer<JsonRPC::ResponseHandler>)));

        this->socket = socket;
        return true;
    } else {
        return false;
    }
}

bool TcpHelper::call(const QString &method, const QVariant &params, const QVariant &id)
{
    if (peer)
        return peer->call(method, params, id);
    else
        return false;
}

void TcpHelper::onReadyMessage(const QByteArray &json)
{
    {
        QDataStream stream(socket);
        stream.setVersion(QDataStream::Qt_4_6);
        quint16 size = json.size();
        stream << size;
    }
    socket->write(json);
}

void TcpHelper::onReadyRead()
{
    buffer.append(socket->readAll());

    if (nextMessageSize)
        goto STATE_WAITING_FOR_CONTENT;

    STATE_UNKNOW_SIZE:
    // nextMessageSize is 2 bytes long (quint16)
    if (buffer.size() >= 2) {
        QDataStream stream(&buffer, QIODevice::ReadWrite);
        stream.setVersion(QDataStream::Qt_4_6);
        stream >> nextMessageSize;
        buffer.remove(0, 2);
    } else {
        return;
    }

    STATE_WAITING_FOR_CONTENT:
    if (buffer.size() >= nextMessageSize) {
        peer->handleMessage(buffer.left(nextMessageSize));
        buffer.remove(0, nextMessageSize);

        nextMessageSize = 0;
        goto STATE_UNKNOW_SIZE;
    }
}

void TcpHelper::onDisconnected()
{
    // clear peer data
    peer->deleteLater();
    peer = nullptr;

    // clear buffer data
    buffer.clear();
    nextMessageSize = 0;

    // clear socket data
    socket->disconnect();
    socket->deleteLater();
    socket = nullptr;

    emit disconnected();
}
