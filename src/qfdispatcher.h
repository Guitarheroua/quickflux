#pragma once

#include <QObject>
#include <QVariantMap>
#include <QJSValue>
#include <QQueue>
#include <QPair>
#include <QQmlEngine>
#include <QPointer>
#include "priv/qflistener.h"
#include "priv/qfhook.h"
/// Message Dispatcher

class QFDispatcher : public QObject
{
    Q_OBJECT
public:
    explicit QFDispatcher(QObject *parent = nullptr);
    ~QFDispatcher() = default;

signals:
    /// Listeners should listen on this signal to get the latest dispatched message from AppDispatcher
    void dispatched(const QString &type, const QJSValue &message);

public slots:
    /// Dispatch a message via Dispatcher
    /**
      @param type The type of the message
      @param message The message content
      @reentrant

      Dispatch a message with type via the AppDispatcher. Listeners should listen on the "dispatched"
      signal to be notified.

      Usually, it will emit "dispatched" signal immediately after calling dispatch(). However, if
      AppDispatcher is still dispatching messages, the new messages will be placed on a queue, and
      wait until it is finished. It guarantees the order of messages are arrived in sequence to
      listeners
     */
    Q_INVOKABLE void dispatch(const QString &type, const QJSValue &message = QJSValue());

    Q_INVOKABLE void waitFor(const QVector<int> &ids);

    Q_INVOKABLE int addListener(const QJSValue &callback);

    Q_INVOKABLE void removeListener(int id);

public:

    void dispatch(const QString& type, const QVariant& message);

    int addListener(QFListener* listener);

    QQmlEngine *engine() const;

    void setEngine(QQmlEngine *engine);

    QFHook *hook() const;

    void setHook(QFHook *hook);

private slots:
    /// Invoke listener and emit the dispatched signal
    void send(const QString &type, const QJSValue &message);

private:
    void invokeListeners(const QVector<int> &ids);

    bool m_dispatching;

    QPointer<QQmlEngine> m_engine;

    // Queue for dispatching messages
    QQueue<QPair<QString,QJSValue > > m_queue;

    // Next id for listener.
    int nextListenerId;

    // Registered listener
    QMap<int, QPointer<QFListener> > m_listeners;

    // Current dispatching listener id
    int dispatchingListenerId;

    // Current dispatching message
    QJSValue dispatchingMessage;

    // Current dispatching message type
    QString dispatchingMessageType;

    // List of listeners pending to be invoked.
    QMap<int,bool> pendingListeners;

    // List of listeners blocked in waitFor()
    QMap<int,bool> waitingListeners;

    QPointer<QFHook> m_hook;
};

