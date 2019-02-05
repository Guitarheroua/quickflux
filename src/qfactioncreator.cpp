#include <QtCore>
#include <QMetaMethod>
#include <QtQml>
#include "qfactioncreator.h"
#include "priv/qfsignalproxy.h"
#include "qfdispatcher.h"

/*!
  \qmltype ActionCreator
  \brief Create message from signal then dispatch via AppDispatcher

ActionCreator is a component that listens on its own signals, convert to message then dispatch via AppDispatcher. The message type will be same as the signal name. There has no limitation on the number of arguments and their data type.

For example, you may declare an ActionCreator based component as:

\code
import QtQuick 2.0
import QuickFlux 1.0
pragma singleton

ActionCreator {
   signal open(string url);
}
\endcode

It is equivalent to:

\code
import QtQuick 2.0
import QuickFlux 1.0
pragma singleton

Item {
   function open(url) {
     AppDispatcher.dispatch(“open”, {url: url});
   }
}
\endcode

 */

QFActionCreator::QFActionCreator(QObject *parent)
    : QObject(parent)
{
}

QString QFActionCreator::genKeyTable()
{
    auto imports = QStringList{} << QStringLiteral("pragma Singleton")
                                 << QStringLiteral("import QtQuick 2.0")
                                 << QStringLiteral("import QuickFlux 1.0\n");
    auto header = QStringList{} << QStringLiteral("KeyTable {\n");
    auto footer = QStringList{} <<  QStringLiteral("}");

    const auto memberOffset = QObject::staticMetaObject.methodCount();
    const auto meta = metaObject();
    auto count = meta->methodCount();
    QStringList properties;

    for (auto i = memberOffset ; i < count ;i++)
    {
        auto method = meta->method(i);
        if (method.name() == QByteArrayLiteral("dispatcherChanged"))
            continue;
        if (method.methodType() == QMetaMethod::Signal)
            properties << QStringLiteral("    property string %1;\n").arg(QString(method.name()));
    }

    auto content = QStringList{} << imports << header << properties << footer;

    return content.join('\n');
}

void QFActionCreator::dispatch(const QString &type, const QJSValue &message)
{
    if (!m_dispatcher.isNull())
        m_dispatcher->dispatch(type, message);
}

void QFActionCreator::classBegin()
{
}

void QFActionCreator::componentComplete()
{
    auto engine = qmlEngine(this);

    if (m_dispatcher.isNull())
        setDispatcher(QFAppDispatcher::instance(engine));

    auto dispatcher = m_dispatcher.data();

    const auto memberOffset = QObject::staticMetaObject.methodCount();

    const auto meta = metaObject();

    auto count = meta->methodCount();

    for (int i = memberOffset ; i < count ;i++)
    {
        QMetaMethod method = meta->method(i);
        if (method.name() == QByteArray{"dispatcherChanged"})
            continue;

        if (method.methodType() == QMetaMethod::Signal)
        {
            auto proxy = new QFSignalProxy(this);
            proxy->bind(this, i, engine, dispatcher);
            m_proxyList << proxy;
        }
    }
}

/*! \qmlproperty object ActionCreator::dispatcher

This property holds the target Dispatcher instance. It will dispatch all the actions to that object.

\code

    ActionCreator {
        dispatcher: Dispatcher {
        }
    }

\endcode

The default value is AppDispatcher
 */


QFDispatcher *QFActionCreator::dispatcher() const
{
    return m_dispatcher;
}

void QFActionCreator::setDispatcher(QFDispatcher *value)
{
    m_dispatcher = value;
    for (auto &proxy : m_proxyList)
        proxy->setDispatcher(m_dispatcher);

    emit dispatcherChanged();
}
