#ifndef VISCLIENT_H
#define VISCLIENT_H

#include <QtCore/QObject>
#include <QtWebSockets/QWebSocket>

class VisClient : public QObject
{
    Q_OBJECT

public:
    explicit VisClient(QObject* parent = nullptr);
    ~VisClient();

    Q_INVOKABLE void connectTo(const QString& address);
    Q_INVOKABLE void disconnect();
    Q_INVOKABLE void sendMessage(const QString& message);

Q_SIGNALS:
    void connected();
    void disconnected();
    void error(const QString& message);
    void messageReceived(const QString& message);

private Q_SLOTS:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError>& errors);
    void onTextMessageReceived(const QString& message);

private:
    QWebSocket mWebSocket;
};

#endif // VISCLIENT_H