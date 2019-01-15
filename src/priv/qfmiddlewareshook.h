#ifndef QFMIDDLEWARESHOOK_H
#define QFMIDDLEWARESHOOK_H

#include <QObject>
#include <QQmlEngine>
#include <QPointer>
#include "./priv/qfhook.h"

class QFMiddlewaresHook : public QFHook
{
    Q_OBJECT
public:
    explicit QFMiddlewaresHook(QObject *parent = nullptr);

signals:

public:
    void dispatch(const QString &type, const QJSValue &message) override;
    void setup(QQmlEngine* engine, QObject* middlewares);

public slots:
    void next(int senderId, const QString &type, const QJSValue &message);
    void resolve(const QString &type, const QJSValue &message);


private:
    QJSValue invoke;
    QPointer<QObject> m_middlewares;
};

#endif // QFMIDDLEWARESHOOK_H
