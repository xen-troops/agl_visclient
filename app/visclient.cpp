#include "visclient.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>

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

void VisClient::setCarMessage(const QString& message)
{
    qDebug() << "Set car message:" << message;

    QJsonDocument json(QJsonObject{{"action", "set"},
                                   {"path", "Attribute.Car.Message"},
                                   {"value", message},
                                   {"requestId", getRequestId()}});

    mWebSocket.sendTextMessage(json.toJson(QJsonDocument::Compact));
}

/*******************************************************************************
 * Private
 ******************************************************************************/

void VisClient::onConnected()
{
    Q_EMIT VisClient::connected();

    subscribe();
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
    qDebug() << "Message received:" << message;

    QJsonParseError parseError;

    QJsonObject json = QJsonDocument::fromJson(message.toUtf8(), &parseError).object();

    if (parseError.error != QJsonParseError::NoError)
    {
        qCritical() << "Error parsing message at" << parseError.offset << ":" << parseError.errorString();
        return;
    }

    if (json["action"].toString() == "subscribe")
    {
        mSubscribeId = json["subscriptionId"].toString();
    }
    else if (json["action"].toString() == "set")
    {
        // TODO: handle set errors
    }
    else if (json["action"].toString() == "subscription" && json["subscriptionId"].toString() == mSubscribeId)
    {
        qDebug() << "Car message:" << json["value"].toString();
        emit VisClient::carMessageReceived(json["value"].toString());
    }
    else
    {
        qCritical() << "Malformed response received";
    }
}

QString VisClient::getRequestId()
{
    static int requestId;

    return QString::number(requestId++);
}

void VisClient::subscribe()
{
    QJsonDocument json(
        QJsonObject{{"action", "subscribe"}, {"path", "Attribute.Car.Message"}, {"requestId", getRequestId()}});

    mWebSocket.sendTextMessage(json.toJson(QJsonDocument::Compact));
}
