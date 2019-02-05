#include <QtQml>
#include <QQmlEngine>
#include <QFAppDispatcher>
#include "priv/quickfluxfunctions.h"
#include "qfactioncreator.h"
#include "qfstore.h"

/*!
   \qmltype Store
   \inqmlmodule QuickFlux 1.1
   \brief Store Component

    Store is a helper item for implement the data “Store” component in a Quick Flux application. It could listen from ActionCreator / Dispatcher    component and redispatch the received action to another store components (e.g children store).

    It is a replacement of AppListener component

    Example:

    \code

    import QuickFlux 1.1

    Store {
      bindSource: AppDispatcher

      Filter {
        type: ActionTypes.addItem
        onDispatched: {
          /// Process
        }
      }

    }

    \endcode

    \b{The order of action delivery:}

    \code

    Store {
      id: rootStore

      bindSource: AppDispatcher

      property alias page1 : page1

      Store {
        id: page1
      }

      Store {
        id: page2
      }

      Filter {
        id: filter1
      }

    }
    \endcode

In the example above, the rootStore is bind to AppDispatcher, whatever the dispatcher dispatch an action, it will first re-dispatch the action to its children sequentially. Then emit the dispatched signal on itself. Therefore, the order of receivers is: page1, page2 then filter1.

If the redispatchTargets property is set, Store component will also dispatch the received action to the listed objects.

*/

/*!
  \qmlsignal Store::dispatched(string type, object message)

  This signal is emitted when a message is received by this store.

  There has two suggested methods to listen this signal:

  Method 1 - Use Filter component

  \code

  Store {
      Filter {
          type: ActionTypes.addItem
          onDispatched: {
              // process here
          }
      }
  }
  \endcode

  Method 2 - Use filter function

  \code
  Store {
      filterFunctionEnabled: true

      function addItem(message) {

      }
  }
  \endcode

*/

/*! \qmlproperty bool Store::filterFunctionEnabled
If this property is true, whatever the store component received a new action. Beside to emit a dispatched signal, it will search for a function with a name as the action. If it exists, it will call also call the function.

\code

Store {
  filterFunctionEnabled: true

  function addItem(message) {
  }
}
\endcode

The default value is false
 */


QFStore::QFStore(QObject *parent)
    : QObject{parent}
    , m_filterFunctionEnabled{false}
{
}

QQmlListProperty<QObject> QFStore::children()
{
    return QQmlListProperty<QObject>(this, m_children);
}

void QFStore::dispatch(const QString &type, const QJSValue &message)
{
    auto engine = qmlEngine(this);
    QF_PRECHECK_DISPATCH(engine, type, message);

    for(const auto &child : m_children)
        if (auto store = qobject_cast<QFStore *>(child))
            store->dispatch(type, message);

    for(const auto &child : m_redispatchTargets)
        if ( auto store = qobject_cast<QFStore*>(child))
            store->dispatch(type, message);

    if (m_filterFunctionEnabled)
    {
        const auto meta = metaObject();

        auto signature = QMetaObject::normalizedSignature(QStringLiteral("%1(QVariant)").arg(type).toUtf8().constData());
        if (auto index = meta->indexOfMethod(signature.constData()); index >= 0)
        {
            auto method = meta->method(index);
            auto value = QVariant::fromValue<QJSValue>(message);
            method.invoke(this, Qt::DirectConnection, Q_ARG(QVariant, value));
        }

        signature = QMetaObject::normalizedSignature(QStringLiteral("%1()").arg(type).toUtf8().constData());
        if (auto index = meta->indexOfMethod(signature.constData()); index >= 0)
        {
            auto method = meta->method(index);
            method.invoke(this);
        }
    }

    emit dispatched(type, message);
}

void QFStore::bind(QObject *source)
{
    setBindSource(source);
}

void QFStore::setup()
{
    const auto bindSourceData = m_bindSource.data();

    auto creator = qobject_cast<QFActionCreator*>(bindSourceData);
    auto dispatcher = creator ? creator->dispatcher()
                              : qobject_cast<QFDispatcher *>(bindSourceData);

        // Nothing changed.
    if (m_actionCreator.data() == creator && m_dispatcher.data() == dispatcher)
        return;

    if (!m_actionCreator.isNull() && m_actionCreator.data() != creator)
        m_actionCreator->disconnect(this);

    if (!m_dispatcher.isNull() && m_dispatcher.data() != dispatcher)
        m_dispatcher->disconnect(this);

    m_actionCreator = creator;
    m_dispatcher = dispatcher;

    if (!m_actionCreator.isNull())
        connect(m_actionCreator.data(), &QFActionCreator::dispatcherChanged, this, &QFStore::setup);

    if (!m_dispatcher.isNull())
        connect(dispatcher, &QFDispatcher::dispatched, this, &QFStore::dispatch);
}

/*! \qmlproperty array Store::redispatchTargets

  By default, the Store component redispatch the received action to its children sequentially. If this property is set,
  the action will be re-dispatch to the target objects too.

  \code

    Store {
        id: bridgeStore

        redispatchTargets: [
            SingletonStore1,
            SingletonStore2
        ]
    }
  \endcode


 */

QQmlListProperty<QObject> QFStore::redispatchTargets()
{
    return QQmlListProperty<QObject>(this, m_redispatchTargets);
}


/*! \qmlproperty object Store::bindSource
 *
 * This property holds the source of action. It can be an ActionCreator / Dispatcher component
 *
 * The default value is null, and it listens to AppDispatcher
 */


QObject *QFStore::bindSource() const
{
    return m_bindSource.data();
}

void QFStore::setBindSource(QObject *source)
{
    m_bindSource = source;
    setup();
    emit bindSourceChanged();
}
