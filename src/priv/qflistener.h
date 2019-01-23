#ifndef QFLISTENER_H
#define QFLISTENER_H

#include <QObject>
#include <QJSValue>
#include <QVector>

class QFDispatcher;

/// A listener class for AppDispatcher.

class QFListener : public QObject
{
    Q_OBJECT
public:
    explicit QFListener(QObject *parent = nullptr);
    ~QFListener() = default;

    QJSValue callback() const;

    void setCallback(const QJSValue &callback);

    void dispatch(QFDispatcher* dispatcher, const QString &type, const QJSValue &message);

    int listenerId() const;

    void setListenerId(int listenerId);

    QVector<int> waitFor() const;

    void setWaitFor(const QVector<int> &waitFor);

signals:
    void dispatched(const QString &type, const QJSValue &message);

public slots:

private:
    QJSValue m_callback;

    int m_listenerId;

    QVector<int> m_waitFor;
};

#endif // QFLISTENER_H
