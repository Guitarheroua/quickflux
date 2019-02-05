#pragma once
#include <QObject>
#include <QJSValue>

class QFHook : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;
    virtual void dispatch(const QString &type, const QJSValue &message) = 0;

signals:
    void dispatched(const QString &type, const QJSValue &message);
};
