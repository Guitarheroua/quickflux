#include <QtCore>
#include <QtQml>
#include <QVariant>
#include <QJSValue>
#include <QPointer>
#include "priv/quickfluxfunctions.h"
#include "qfdispatcher.h"

struct DispatchingGuard
{
    DispatchingGuard(bool &dispatching)
        : m_dispatching{dispatching}
    {
        m_dispatching = true;
    }

    ~DispatchingGuard()
    {
        m_dispatching = false;
    }

private:
    bool &m_dispatching;
};

/*!
   \qmltype Dispatcher
   \inqmlmodule QuickFlux
   \brief Message Dispatcher

   \code
   import QuickFlux 1.1
   \endcode

    Dispatcher is a component for delivering action message.

    Usually you don't need to declare a Dispatcher component by yourself beside writing test case.
    It is suggested to use AppDispatcher directly.

    It is added since QuickFlux v1.1
*/

/*!
  \qmlsignal Dispatcher::dispatched(string type, object message)

  This signal is emitted when an action message is ready to dispatch by Dispatcher.

  There has several methods to listen this signal:


\b{Method 1 - Using Store component}

It is the suggested method. (Since QuickFlux 1.1)

\code
import QuickFlux 1.1

Dispatcher {
  id: dispatcher
}

Store {
  bindSource: dispatcher

  Filter {
    type: ActionTypes.openItem
    onDispatched: {
      // ..
    }
  }
}
\endcode


\b{Method 2 - Using Connections component}

\code
import QuickFlux 1.1

Dispatcher {
  id: dispatcher
}

Connections {
    target: dispatcher
    onDispatched: {
        switch (type) {
            case "OpenItem";
                // ...
                break;
            case "DeleteItem";
                // ...
                break;
        }
    }
}
\endcode
 */

/*! \fn QFAppDispatcher::dispatched(QString type,QJSValue message)

  This signal is emitted when a message is ready to dispatch by AppDispatcher.

 */


/*!
  \class QFAppDispatcher
  \inmodule QuickFlux

  QFAppDispatcher is the C++ implementation of AppDispatcher in QML scope.
  You may need this class to setup communication between C++ and QML.

\code
#include "qfappdispatcher.h"
\endcode

 */

QFDispatcher::QFDispatcher(QObject *parent)
    : QObject{parent}
      , m_dispatching{false}
      , m_nextListenerId{1}
      , m_dispatchingListenerId{}
{
}

/*!
  \qmlmethod Dispatcher::dispatch(string type, object message)

  Dispatch an action by the Dispatcher. An action consists two parts: The type and message.

  The action may not be dispatched immediately. It will first pass to registered Middleware. They may modify / remove the action.

  If there has more then one pending action, it will be placed on a queue to guarantees the order of messages are arrived in sequence to store (First come first served)

\code
Store {

  Filter {
    type: ActionTypes.askToRemoveItem
    onDispatched: {
        if (options.skipRemoveConfirmation) {
            AppDispatcher.dispatch(ActionTypes.removeItem, message);
            // Because AppDispatcher is still dispatching ActionTypes.askToRemoveItem,
            // ActionTypes.removeItem will be placed in a queue and will dispatch when
            // all the listeners received current message.
        }
    }
  }
}

\endcode

 */

void QFDispatcher::dispatch(const QString &type, const QJSValue &message)
{
    QF_PRECHECK_DISPATCH(m_engine.data(), type, message);

    auto process = [this](const auto &type, const auto &message) {
        if (m_hook.isNull())
            send(type, message);
        else
            m_hook->dispatch(type, message);
    };

    if (m_dispatching)
    {
        m_queue.enqueue(qMakePair(type, message));
        return;
    }

    DispatchingGuard dispatchingGuard(m_dispatching);

    process(type, message);

    while (!m_queue.empty())
    {
        auto pair = m_queue.dequeue();
        process(pair.first, pair.second);
    }
}

/*!
  \qmlmethod Dispatcher::waitFor(int listenerId)
  \b{This method is deprecated}

  Waits for a callback specifed via the listenerId to be executed before continue execution of current callback.
  You should call this method only by a callback registered via addListener.

 */

void QFDispatcher::waitFor(const QVector<int> &ids)
{
    if (!m_dispatching || ids.empty())
        return;

    m_waitingListeners[m_dispatchingListenerId] = true;
    invokeListeners(ids);
    m_waitingListeners.remove(m_dispatchingListenerId);
}

/*!

  \qmlmethod int Dispatcher::addListener(func callback)
  \b{This method is deprecated}

  Registers a callback to be invoked with every dispatched message. Returns a listener ID that can be used with waitFor().

 */

int QFDispatcher::addListener(const QJSValue &callback)
{
    auto listener = new QFListener(this);
    listener->setCallback(callback);

    return addListener(listener);
}

/*! \fn int QFAppDispatcher::addListener(QFListener *listener)

  It is private API. Do not use it.

 */
int QFDispatcher::addListener(QFListener *listener)
{
    m_listeners[m_nextListenerId] = listener;
    listener->setListenerId(m_nextListenerId);
    return m_nextListenerId++;
}

/*!
  \qmlmethod Dispatcher::removeListener(int listenerId)
  \b{This method is deprecated}

  Remove a callback by the listenerId returned by addListener
 */

void QFDispatcher::removeListener(int id)
{
    if (m_listeners.contains(id))
    {
        if (auto listener = m_listeners.value(id).data(); listener->parent() == this)
            listener->deleteLater();

        m_listeners.remove(id);
    }
}


/*! \fn QFAppDispatcher::dispatch(const QString& type, const QVariant& message)

  Dispatch a message with type via the AppDispatcher.
  The message will be placed on a queue and delivery via the "dispatched" signal.
  Listeners may listen on the "dispatched" signal directly,
  or using helper components like AppListener / AppScript to capture signal.
 */

void QFDispatcher::dispatch(const QString &type, const QVariant &message)
{
    if (m_engine.isNull())
    {
        qWarning() << "QFAppDispatcher::dispatch() - Unexpected error: engine is not available.";
        return;
    }

    auto value = m_engine->toScriptValue<QVariant>(message);

    dispatch(type, value);
}


void QFDispatcher::send(const QString &type, const QJSValue &message)
{
    m_dispatchingMessage = message;
    m_dispatchingMessageType = type;
    m_pendingListeners.clear();
    m_waitingListeners.clear();

    QMapIterator<int, QPointer<QFListener>> iter(m_listeners);
    QVector<int> ids;
    while (iter.hasNext())
    {
        iter.next();
        m_pendingListeners[iter.key()] = true;
        ids << iter.key();
    }

    invokeListeners(ids);

    emit dispatched(type,message);
}

void QFDispatcher::invokeListeners(const QVector<int> &ids)
{
    for (const auto &next : ids)
    {
        if (m_waitingListeners.contains(next))
            qWarning() << QStringLiteral("AppDispatcher: Cyclic dependency detected");

        if (m_pendingListeners.contains(next))
        {
            m_pendingListeners.remove(next);
            m_dispatchingListenerId = next;

            if (auto listener = m_listeners.value(next).data(); listener)
                listener->dispatch(this,m_dispatchingMessageType, m_dispatchingMessage);
        }
    }
}

QFHook *QFDispatcher::hook() const
{
    return m_hook;
}

void QFDispatcher::setHook(QFHook *hook)
{
    if (!m_hook.isNull())
        m_hook->disconnect(this);

    m_hook = hook;

    if (!m_hook.isNull())
        connect(m_hook.data(), &QFHook::dispatched, this, &QFDispatcher::send, Qt::UniqueConnection);
}

/*! \fn QQmlEngine *QFAppDispatcher::engine() const

  Obtain the associated engine to this dispatcher.

 */

QQmlEngine *QFDispatcher::engine() const
{
    return m_engine.data();
}

/*! \fn QFAppDispatcher::setEngine(QQmlEngine *engine)

  Set the associated \a engine. It is private API. Do not call it.

 */

void QFDispatcher::setEngine(QQmlEngine *engine)
{
    m_engine = engine;
}

