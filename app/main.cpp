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

#ifdef HAVE_LIBHOMESCREEN
#include <libhomescreen.hpp>
#endif

#include "visclient.h"

int main(int argc, char* argv[])
{
    VisClient visClient;

#ifdef HAVE_LIBHOMESCREEN
    LibHomeScreen libHomeScreen;

    if (!libHomeScreen.renderAppToAreaAllowed(0, 1))
    {
        qWarning() << "renderAppToAreaAllowed is denied";
        return -1;
    }
#endif

    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("VISClient"));

    QQuickStyle::setStyle("AGL");

    QCommandLineParser parser;
    parser.addPositionalArgument("port", app.translate("main", "port for binding"));
    parser.addPositionalArgument("secret", app.translate("main", "secret for binding"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);
    QStringList positionalArguments = parser.positionalArguments();

    QQmlApplicationEngine engine;
    QQmlContext* context = engine.rootContext();

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
        context->setContextProperty(QStringLiteral("bindingAddress"), bindingAddress);
    }

    // Read configuration from /etc/visclient/config.json

    QFile file("/etc/visclient/config.json");

    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QJsonObject json = QJsonDocument::fromJson(file.readAll()).object();

    file.close();

    QString visUrl = json["visServer"].toString();

    if (visUrl.isEmpty())
    {
        visUrl = "wss://localhost:8088";
    }

    context->setContextProperty(QStringLiteral("visUrl"), visUrl);
    context->setContextProperty("visClient", &visClient);

    engine.load(QUrl(QStringLiteral("qrc:/visclient.qml")));

    return app.exec();
}
