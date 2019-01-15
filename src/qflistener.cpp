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

    if (m_waitFor.size() > 0) {
        dispatcher->waitFor(m_waitFor);
    }

    if (m_callback.isCallable()) {
        QJSValueList args;
        args << type << message;
        auto ret = m_callback.call(args);

        if (ret.isError()) {
            auto message = QString("%1:%2: %3: %4")
                           .arg(ret.property(QLatin1String{"fileName"}).toString())
                           .arg(ret.property(QLatin1String{"lineNumber"}).toString())
                           .arg(ret.property(QLatin1String{"name"}).toString())
                           .arg(ret.property(QLatin1String{"message"}).toString());
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

QList<int> QFListener::waitFor() const
{
    return m_waitFor;
}

void QFListener::setWaitFor(const QList<int> &waitFor)
{
    m_waitFor = waitFor;
}

