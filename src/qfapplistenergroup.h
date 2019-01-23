#ifndef QFAPPLISTENERGROUP_H
#define QFAPPLISTENERGROUP_H

#include <QQuickItem>
#include <QQmlParserStatus>
#include "priv/qflistener.h"

class QFAppListenerGroup : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QVector<int> listenerIds READ listenerIds WRITE setListenerIds NOTIFY listenerIdsChanged)
    Q_PROPERTY(QVector<int> waitFor READ waitFor WRITE setWaitFor NOTIFY waitForChanged)

public:
    QFAppListenerGroup(QQuickItem* parent = nullptr);

    QVector<int> listenerIds() const;

    void setListenerIds(const QVector<int> &listenerIds);

    QVector<int> waitFor() const;

    void setWaitFor(const QVector<int> &waitFor);

public slots:

signals:

    void listenerIdsChanged();
    void waitForChanged();

private:
    virtual void componentComplete();

    QVector<int> search(QQuickItem* item);

    void setListenerWaitFor();

    QVector<int> m_waitFor;

    QVector<int> m_listenerIds;
    int m_listenerId;
    QFListener* m_listener;
};

#endif // QFAPPLISTENERGROUP_H
