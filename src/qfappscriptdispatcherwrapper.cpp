#include <QQmlEngine>
#include "qfdispatcher.h"
#include "priv/qfappscriptdispatcherwrapper.h"

QString QFAppScriptDispatcherWrapper::type() const
{
    return m_type;
}

void QFAppScriptDispatcherWrapper::setType(const QString &type)
{
    m_type = type;
}

void QFAppScriptDispatcherWrapper::dispatch(const QJSValue &arguments)
{
    if (m_dispatcher.isNull()) {
        qWarning() << "AppScript::Unexcepted condition: AppDispatcher is not present.";
        return;
    }

    m_dispatcher->dispatch(m_type,arguments);
}

QFDispatcher *QFAppScriptDispatcherWrapper::dispatcher() const
{
    return m_dispatcher;
}

void QFAppScriptDispatcherWrapper::setDispatcher(QFDispatcher *dispatcher)
{
    m_dispatcher = dispatcher;
}
