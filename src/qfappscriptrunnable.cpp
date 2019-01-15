#include <QtCore>
#include <QUuid>
#include <QFAppDispatcher>
#include "priv/qfappscriptrunnable.h"
#include "priv/qfappscriptdispatcherwrapper.h"

QFAppScriptRunnable::QFAppScriptRunnable(QObject *parent)
    : QObject(parent)
      , m_next{}
      , m_isSignalCondition{false}
      , m_isOnceOnly{true}
{
}

QFAppScriptRunnable::~QFAppScriptRunnable()
{
    release();
}

QJSValue QFAppScriptRunnable::script() const
{
    return m_script;
}

void QFAppScriptRunnable::setScript(const QJSValue &script)
{
    m_script = script;
}

QString QFAppScriptRunnable::type() const
{
    return m_type;
}

void QFAppScriptRunnable::setType(const QString &type)
{
    m_type = type;
}

bool QFAppScriptRunnable::isOnceOnly() const
{
    return m_isOnceOnly;
}

void QFAppScriptRunnable::setIsOnceOnly(bool isOnceOnly)
{
    m_isOnceOnly = isOnceOnly;
}

void QFAppScriptRunnable::setEngine(QQmlEngine* engine)
{
    m_engine = engine;
}

void QFAppScriptRunnable::release()
{
    if (!m_condition.isNull() &&
            m_condition.isObject() &&
        m_condition.hasProperty(QLatin1String{"disconnect"})) {

        auto disconnect = m_condition.property(QLatin1String{"disconnect"});
        QJSValueList args;
        args << m_callback;

        disconnect.callWithInstance(m_condition,args);
    }

    m_condition = QJSValue();
    m_callback = QJSValue();
}

void QFAppScriptRunnable::run(const QJSValue &message)
{
    QJSValueList args;
    if (m_isSignalCondition &&
        message.hasProperty(QLatin1String{"length"})) {
        int count = message.property(QLatin1String{"length"}).toInt();
        for (int i = 0 ; i < count;i++) {
            args << message.property(i);
        }
    } else {
        args << message;
    }
    QJSValue ret = m_script.call(args);

    if (ret.isError()) {
        QString message = QString("%1:%2: %3: %4")
                          .arg(ret.property(QLatin1String{"fileName"}).toString()
                              , ret.property(QLatin1String{"lineNumber"}).toString()
                              , ret.property(QLatin1String{"name"}).toString()
                              , ret.property(QLatin1String{"message"}).toString());
        qWarning() << message;
    }

}

QFAppScriptRunnable *QFAppScriptRunnable::then(const QJSValue &condition, const QJSValue &script)
{
    auto runnable = new QFAppScriptRunnable(this);
    runnable->setEngine(m_engine.data());
    runnable->setCondition(condition);
    runnable->setScript(script);
    setNext(runnable);
    return runnable;
}

QFAppScriptRunnable *QFAppScriptRunnable::next() const
{
    return m_next;
}

void QFAppScriptRunnable::setNext(QFAppScriptRunnable *next)
{
    m_next = next;
}

void QFAppScriptRunnable::setCondition(const QJSValue &condition)
{
    m_condition = condition;

    if (condition.isString()) {
        setType(condition.toString());
        m_isSignalCondition = false;
    } else if (condition.isObject() && condition.hasProperty("connect")) {
        Q_ASSERT(!m_engine.isNull());

        auto type = QString("QuickFlux.AppScript.%1").arg(QUuid::createUuid().toString());
        setType(type);

        auto generator = "function(dispatcher) { return function() {dispatcher.dispatch(arguments)}}";
        auto dispatcher = QFAppDispatcher::instance(m_engine);
        auto wrapper = new QFAppScriptDispatcherWrapper();
        wrapper->setType(type);
        wrapper->setDispatcher(dispatcher);

        auto generatorFunc = m_engine->evaluate(generator);

        QJSValueList args;
        args << m_engine->newQObject(wrapper);
        auto callback = generatorFunc.call(args);

        args.clear();
        args << callback;

        auto connect = condition.property(QLatin1String{"connect"});
        connect.callWithInstance(condition,args);

        m_callback = callback;
        m_isSignalCondition = true;
    } else {
        qWarning() << QLatin1String{"AppScript: Invalid condition type"};
    }
}

