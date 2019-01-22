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
        m_condition.hasProperty(QStringLiteral("disconnect"))) {

        auto disconnect = m_condition.property(QStringLiteral("disconnect"));
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
        message.hasProperty(QStringLiteral("length"))) {
        int count = message.property(QStringLiteral("length")).toInt();
        for (int i = 0 ; i < count;i++) {
            args << message.property(i);
        }
    } else {
        args << message;
    }
    QJSValue ret = m_script.call(args);

    if (ret.isError()) {
        QString message = QStringLiteral("%1:%2: %3: %4")
                              .arg(ret.property(QStringLiteral("fileName")).toString()
                                       , ret.property(QStringLiteral("lineNumber")).toString()
                                       , ret.property(QStringLiteral("name")).toString()
                                       , ret.property(QStringLiteral("message")).toString());
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
    } else if (condition.isObject() && condition.hasProperty(QStringLiteral("connect"))) {
        Q_ASSERT(!m_engine.isNull());

        auto type = QStringLiteral("QuickFlux.AppScript.%1").arg(QUuid::createUuid().toString());
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

        auto connect = condition.property(QStringLiteral("connect"));
        connect.callWithInstance(condition, args);

        m_callback = callback;
        m_isSignalCondition = true;
    } else {
        qWarning() << QStringLiteral("AppScript: Invalid condition type");
    }
}

