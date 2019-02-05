#include <QtCore>
#include "qfdispatcher.h"
#include "priv/qflistener.h"

QFListener::QFListener(QObject *parent)
    : QObject(parent)
    , m_listenerId{0}
{
}

QJSValue QFListener::callback() const
{
    return m_callback;
}

void QFListener::setCallback(const QJSValue &callback)
{
    m_callback = callback;
}

void QFListener::dispatch(QFDispatcher *dispatcher, const QString &type, const QJSValue &message)
{

    if (!m_waitFor.empty())
        dispatcher->waitFor(m_waitFor);

    if (m_callback.isCallable())
    {
        auto args = QJSValueList{} << type << message;
        if (auto ret = m_callback.call(args); ret.isError())
        {
            auto message = QStringLiteral("%1:%2: %3: %4")
                               .arg(ret.property(QStringLiteral("fileName")).toString()
                                  , ret.property(QStringLiteral("lineNumber")).toString()
                                  , ret.property(QStringLiteral("name")).toString()
                                  , ret.property(QStringLiteral("message")).toString());
            qWarning() << message;
        }
    }

    emit dispatched(type,message);
}

int QFListener::listenerId() const
{
    return m_listenerId;
}

void QFListener::setListenerId(int listenerId)
{
    m_listenerId = listenerId;
}

QVector<int> QFListener::waitFor() const
{
    return m_waitFor;
}

void QFListener::setWaitFor(const QVector<int> &waitFor)
{
    m_waitFor = waitFor;
}

