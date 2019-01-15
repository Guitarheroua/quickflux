#include <QFAppDispatcher>
#include "qfapplistenergroup.h"
#include "qfapplistener.h"
#include "priv/qflistener.h"

QFAppListenerGroup::QFAppListenerGroup(QQuickItem* parent)
    : QQuickItem(parent)
      , m_listenerId{0}
      , m_listener{}
{

}

QList<int> QFAppListenerGroup::listenerIds() const
{
    return m_listenerIds;
}

void QFAppListenerGroup::setListenerIds(const QList<int> &listenerIds)
{
    m_listenerIds = listenerIds;
    emit listenerIdsChanged();
}

void QFAppListenerGroup::componentComplete()
{
    QQuickItem::componentComplete();

    auto engine = qmlEngine(this);
    Q_ASSERT(engine);

    auto dispatcher = QFAppDispatcher::instance(engine);

    m_listener = new QFListener(this);
    m_listenerId = dispatcher->addListener(m_listener);
    setListenerWaitFor();

    auto ids = search(this);
    setListenerIds(ids);
}

QList<int> QFAppListenerGroup::search(QQuickItem *item)
{
    QList<int> res;

    auto listener = qobject_cast<QFAppListener*>(item);

    if (listener) {
        res.append(listener->listenerId());
        listener->setWaitFor(QList<int>() << m_listenerId);
    }

    auto childs = item->childItems();

    for (const auto &child : childs) {
        auto subRes = search(child);
        if (subRes.size() > 0) {
            res.append(subRes);
        }
    }
    return res;
}

void QFAppListenerGroup::setListenerWaitFor()
{
    m_listener->setWaitFor(m_waitFor);
}

QList<int> QFAppListenerGroup::waitFor() const
{
    return m_waitFor;
}

void QFAppListenerGroup::setWaitFor(const QList<int> &waitFor)
{
    m_waitFor = waitFor;
    setListenerWaitFor();
    emit waitForChanged();
}

