#include <QtCore>
#include <QQmlComponent>
#include "qfappdispatcher.h"

/*!
   \qmltype AppDispatcher
   \inqmlmodule QuickFlux
   \brief Message Dispatcher
   \inherits Dispatcher

    AppDispatcher is a singleton object for delivering action message.
 */


QFAppDispatcher::QFAppDispatcher(QObject *parent)
    : QFDispatcher{parent}
{
}

/*! \fn QFAppDispatcher *QFAppDispatcher::instance(QQmlEngine *engine)

  Obtain the singleton instance of AppDispatcher for specific \a engine

 */

QFAppDispatcher *QFAppDispatcher::instance(QQmlEngine *engine)
{
    auto dispatcher = qobject_cast<QFAppDispatcher*>(singletonObject(engine, QStringLiteral("QuickFlux"), 1, 0, QStringLiteral("AppDispatcher")));
    return dispatcher;
}


/*! \fn QObject *QFAppDispatcher::singletonObject(QQmlEngine *engine, QString package, int versionMajor, int versionMinor, QString typeName)

 \a engine QQmlEngine instance

 \a package The package name of the singleton object

 \a versionMajor The major version no. of the singleton object

 \a versionMinor The minor version no. of the singleton object

 \a typeName The name of the singleton object

  Obtain a singleton object from a package for specific QQmlEngine instance.
  It is useful when you need to get a singleton Actions object from C++.

 */

QObject *QFAppDispatcher::singletonObject(QQmlEngine *engine, const QString &package, int versionMajor, int versionMinor, const QString &typeName)
{
    auto pattern = QStringLiteral("import QtQuick 2.0\nimport %1 %2.%3;QtObject { property var object : %4 }");
    auto qml = pattern.arg(package).arg(versionMajor).arg(versionMinor).arg(typeName);
    auto comp = QQmlComponent{engine};
    comp.setData(qml.toUtf8(), QUrl());
    auto holder = comp.create();

    if (!holder)
    {
        qWarning() << QStringLiteral("QuickFlux: Failed to gain singleton object: %1").arg(typeName);
        qWarning() << QStringLiteral("Error: ") << comp.errorString();
        return nullptr;
    }

    auto object = holder->property("object").value<QObject*>();
    holder->deleteLater();

    if (!object)
    {
        qWarning() << QStringLiteral("QuickFlux: Failed to gain singleton object: %1").arg(typeName);
        qWarning() << QStringLiteral("Error: Unknown");
    }

    return object;
}
