#ifndef QFAPPSCRIPTDISPATCHERWRAPPER_H
#define QFAPPSCRIPTDISPATCHERWRAPPER_H

#include <QQuickItem>
#include <QPointer>
#include "qfdispatcher.h"

class QFAppScriptDispatcherWrapper : public QObject
{
    Q_OBJECT
public:
    using QObject::QObject;

    QString type() const;
    void setType(const QString &type);

    QFDispatcher *dispatcher() const;
    void setDispatcher(QFDispatcher *dispatcher);

public slots:
    void dispatch(const QJSValue &arguments);

private:
    QString m_type;
    QPointer<QFDispatcher> m_dispatcher;

};

#endif // QFAPPSCRIPTDISPATCHERWRAPPER_H
