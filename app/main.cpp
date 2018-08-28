/*
 * Copyright (C) 2016 The Qt Company Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QCommandLineParser>
#include <QQmlContext>
#include <QQuickWindow>
#include <QUrlQuery>
#include <QtCore/QDebug>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QString>
#include <QtGui/QGuiApplication>
#include <QtQml/QQmlApplicationEngine>
#include <QtQuickControls2/QQuickStyle>
#include <QtWebSockets/QWebSocket>

#include <libhomescreen.hpp>
#include <qlibwindowmanager.h>

#include "visclient.h"

int main(int argc, char* argv[])
{
    VisClient visClient;

    QString myname = QString("VISClient");

    QGuiApplication app(argc, argv);

    app.setApplicationName(myname);

    QQuickStyle::setStyle("AGL");

    QCommandLineParser parser;

    parser.addPositionalArgument("port", app.translate("main", "port for binding"));
    parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    QStringList positionalArguments = parser.positionalArguments();

    QQmlApplicationEngine engine;

    if (positionalArguments.length() == 2)
    {
        int port       = positionalArguments.takeFirst().toInt();
        QString secret = positionalArguments.takeFirst();

        QUrl bindingAddress;

        bindingAddress.setScheme(QStringLiteral("ws"));
        bindingAddress.setHost(QStringLiteral("localhost"));
        bindingAddress.setPort(port);
        bindingAddress.setPath(QStringLiteral("/api"));

        QUrlQuery query;

        query.addQueryItem(QStringLiteral("token"), secret);
        bindingAddress.setQuery(query);

        QQmlContext* context = engine.rootContext();

        context->setContextProperty(QStringLiteral("bindingAddress"), bindingAddress);

        std::string token      = secret.toStdString();
        LibHomeScreen* hs      = new LibHomeScreen();
        QLibWindowmanager* qwm = new QLibWindowmanager();

        // WindowManager
        if (qwm->init(port, secret) != 0)
        {
            exit(EXIT_FAILURE);
        }

        // Request a surface as described in layers.json windowmanager’s file
        if (qwm->requestSurface(myname) != 0)
        {
            exit(EXIT_FAILURE);
        }

        // Create an event callback against an event type. Here a lambda is called when SyncDraw event occurs
        qwm->set_event_handler(QLibWindowmanager::Event_SyncDraw, [qwm, myname](json_object* object) {
            Q_UNUSED(object);
            qwm->endDraw(myname);
        });

        // HomeScreen
        hs->init(port, token.c_str());
        // Set the event handler for Event_TapShortcut which will activate the surface for windowmanager
        hs->set_event_handler(LibHomeScreen::Event_TapShortcut, [qwm, myname](json_object* object) {
            json_object* appnameJ = nullptr;

            if (json_object_object_get_ex(object, "application_name", &appnameJ))
            {
                const char* appname = json_object_get_string(appnameJ);

                if (myname == appname)
                {
                    qDebug("Surface %s got tapShortcut\n", appname);
                    qwm->activateSurface(myname);
                }
            }
        });

        QObject::connect(&visClient, &VisClient::carMessageReceived, [hs, myname](const QString& message) {
            hs->onScreenMessage(
                QJsonDocument(QJsonObject{{"app", myname}, {"message", message}}).toJson(QJsonDocument::Compact));
        });

        // Read configuration from /etc/visclient/config.json

        QFile file("/etc/visclient/config.json");

        file.open(QIODevice::ReadOnly | QIODevice::Text);

        QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();

        file.close();

        QString visUrl = json["visServer"].toString();

        if (visUrl.isEmpty())
        {
            visUrl = "wss://wwwivi:8088";
        }

        context->setContextProperty(QStringLiteral("visUrl"), visUrl);
        context->setContextProperty("visClient", &visClient);

        engine.load(QUrl(QStringLiteral("qrc:/visclient.qml")));

        QObject* root        = engine.rootObjects().first();
        QQuickWindow* window = qobject_cast<QQuickWindow*>(root);

        QObject::connect(window, SIGNAL(frameSwapped()), qwm, SLOT(slotActivateSurface()));

        visClient.connectTo(visUrl);
    }

    /*
    QQmlContext* context = engine.rootContext();

    context->setContextProperty(QStringLiteral("visClient"), &visClient);

    QFile file("/etc/visclient/config.json");

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();

    file.close();

    QString visUrl = json["visServer"].toString();

    if (visUrl.isEmpty())
    {
        visUrl = "wss://wwwivi:8088";
    }

    context->setContextProperty(QStringLiteral("visUrl"), visUrl);

    visClient.connectTo(visUrl);

    engine.load(QUrl(QStringLiteral("qrc:/visclient.qml")));

    QObject* root        = engine.rootObjects().first();
    QQuickWindow* window = qobject_cast<QQuickWindow*>(root);

    root->setProperty("visAddressText", QVariant("Change you text here..."));

    window->setGeometry(0, 0, 600, 800);
    window->show();
    */

    return app.exec();
}
