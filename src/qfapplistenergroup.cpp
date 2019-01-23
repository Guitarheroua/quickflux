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

QVector<int> QFAppListenerGroup::listenerIds() const
{
    return m_listenerIds;
}

void QFAppListenerGroup::setListenerIds(const QVector<int> &listenerIds)
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

QVector<int> QFAppListenerGroup::search(QQuickItem *item)
{
    QVector<int> res;

    auto listener = qobject_cast<QFAppListener*>(item);

    if (listener) {
        res.append(listener->listenerId());
        listener->setWaitFor(QVector<int>() << m_listenerId);
    }

    auto childs = item->childItems();

    for (const auto &child : childs) {
        auto subRes = search(child);
        if (!subRes.empty()) {
            res.append(subRes);
        }
    }
    return res;
}

void QFAppListenerGroup::setListenerWaitFor()
{
    m_listener->setWaitFor(m_waitFor);
}

QVector<int> QFAppListenerGroup::waitFor() const
{
    return m_waitFor;
}

void QFAppListenerGroup::setWaitFor(const QVector<int> &waitFor)
{
    m_waitFor = waitFor;
    setListenerWaitFor();
    emit waitForChanged();
}

