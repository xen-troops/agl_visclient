#include "visclient.h"

QT_USE_NAMESPACE

const unsigned long visClientTimeout = 1000;

/*******************************************************************************
 * VisClient
 ******************************************************************************/

VisClient::VisClient(QObject* parent) : QObject(parent)
{
    qDebug() << "Create VIS client";

    connect(&mWebSocket, QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::error), this, &VisClient::onError);
    connect(&mWebSocket, &QWebSocket::sslErrors, this, &VisClient::onSslErrors);
    connect(&mWebSocket, &QWebSocket::connected, this, &VisClient::onConnected);
    connect(&mWebSocket, &QWebSocket::disconnected, this, &VisClient::onDisconnected);
    connect(&mWebSocket, &QWebSocket::textMessageReceived, this, &VisClient::onTextMessageReceived);
}

VisClient::~VisClient()
{
    qDebug() << "Delete VIS client";

    mWebSocket.close();
}

/*******************************************************************************
 * Public
 ******************************************************************************/

void VisClient::connectTo(const QString& address)
{
    qDebug() << "Connect to:" << address;

    mWebSocket.open(QUrl(address));
}

void VisClient::disconnect()
{
    qDebug() << "Disconnect";

    mWebSocket.close();
}

void VisClient::sendMessage(const QString& message)
{
    qDebug() << "Send message:" << message;

    mWebSocket.sendTextMessage(message);
}

/*******************************************************************************
 * Private
 ******************************************************************************/

void VisClient::onConnected()
{
    Q_EMIT VisClient::connected();
}

void VisClient::onDisconnected()
{
    Q_EMIT VisClient::disconnected();
}

void VisClient::onSslErrors(const QList<QSslError>& errors)
{
    Q_FOREACH (QSslError error, errors)
    {
        if (error.error() == QSslError::SelfSignedCertificate
            || error.error() == QSslError::SelfSignedCertificateInChain)
        {
            mWebSocket.ignoreSslErrors();
            return;
        }
    }
}

void VisClient::onError(QAbstractSocket::SocketError error)
{
    Q_UNUSED(error)

    qDebug() << "Error:" << mWebSocket.errorString();

    Q_EMIT VisClient::error(mWebSocket.errorString());
}

void VisClient::onTextMessageReceived(const QString& message)
{
    qDebug() << "Receive message:" << message;

    Q_EMIT VisClient::messageReceived(message);
}
