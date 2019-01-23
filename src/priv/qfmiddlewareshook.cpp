#include <QtCore>
#include "qfmiddlewareshook.h"
#include "./priv/quickfluxfunctions.h"

QFMiddlewaresHook::QFMiddlewaresHook(QObject *parent) : QFHook(parent)
{

}

void QFMiddlewaresHook::dispatch(const QString &type, const QJSValue &message)
{
    if (m_middlewares.isNull()) {
        emit dispatched(type , message);
    } else {
        next(-1, type , message);
    }
}

void QFMiddlewaresHook::setup(QQmlEngine *engine, QObject *middlewares)
{

    m_middlewares = middlewares;
    auto mobj = engine->newQObject(middlewares);
    auto hobj = engine->newQObject(this);

    auto source = QStringLiteral("function (middlewares, hook) {"
                                   "  function create(senderIndex) {"
                                   "    return function (type, message) {"
                                   "      hook.next(senderIndex, type , message);"
                                   "    }"
                                   "  }"
                                   "  var data = middlewares.data;"
                                   "  for (var i = 0 ; i < data.length; i++) {"
                                   "    var m = data[i];"
                                   "    m._nextCallback = create(i);"
                                   "  }"
                                   "}");

    auto function = engine->evaluate(source);

    QJSValueList args;
    args << mobj;
    args << hobj;

    auto ret = function.call(args);

    if (ret.isError()) {
        QuickFlux::printException(ret);
    }

    source = QStringLiteral("function (middlewares, hook) {"
                           "  return function invoke(receiverIndex, type , message) {"
                           "     if (receiverIndex >= middlewares.data.length) {"
                           "       hook.resolve(type, message);"
                           "       return;"
                           "     }"
                           "     var m = middlewares.data[receiverIndex];"
                           "     if (m.filterFunctionEnabled && m.hasOwnProperty(type) && typeof m[type] === \"function\") { "
                           "       m[type](message);"
                           "     } else if (m.hasOwnProperty(\"dispatch\") && typeof m.dispatch === \"function\") {"
                           "       m.dispatch(type, message);"
                           "     } else {"
                           "       invoke(receiverIndex + 1,type, message);"
                           "     }"
                           "  }"
                            "}");

    function = engine->evaluate(source);
    invoke = function.call(args);
    if (invoke.isError()) {
        QuickFlux::printException(invoke);
    }
}

void QFMiddlewaresHook::next(int senderIndex, const QString &type, const QJSValue &message)
{
    QJSValueList args;

    args << QJSValue(senderIndex + 1);
    args << QJSValue(type);
    args << message;
    auto result = invoke.call(args);
    if (result.isError()) {
        QuickFlux::printException(result);
    }
}

void QFMiddlewaresHook::resolve(const QString &type, const QJSValue &message)
{
    emit dispatched(type, message);
}
