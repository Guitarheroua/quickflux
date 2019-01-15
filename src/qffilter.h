#ifndef QFFILTER_H
#define QFFILTER_H

#include <QObject>
#include <QQmlParserStatus>
#include <QJSValue>
#include <QVariant>
#include <QQmlListProperty>
#include <QQmlEngine>
#include <QPointer>

// Filter represents a filter rule in AppListener

class QFFilter : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_PROPERTY(QString type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QStringList types READ types WRITE setTypes NOTIFY typesChanged)
    Q_INTERFACES(QQmlParserStatus)
    Q_PROPERTY(QQmlListProperty<QObject> __children READ children)
    Q_CLASSINFO("DefaultProperty", "__children")

public:
    explicit QFFilter(QObject *parent = nullptr);

    QString type() const;

    void setType(const QString &type);

    QStringList types() const;

    void setTypes(const QStringList &types);

    QQmlListProperty<QObject> children();

signals:
    void dispatched(const QString &type, const QJSValue &message);

    void typeChanged();

    void typesChanged();

protected:
    void classBegin();
    void componentComplete();

private slots:
    void filter(const QString &type, const QJSValue &message);
    void filter(const QString &type, const QVariant &message);

private:
    QStringList m_types;
    QList<QObject*> m_children;
    QPointer<QQmlEngine> m_engine;
};

#endif // QFFILTER_H
