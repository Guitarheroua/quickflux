#pragma once

#include <QQuickItem>
#include <QPointer>
#include "qfappscript.h"

class QFAppScriptGroup : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QJSValue scripts READ scripts WRITE setScripts NOTIFY scriptsChanged)

public:
    QFAppScriptGroup(QQuickItem* parent = nullptr);

    QJSValue scripts() const;

    void setScripts(const QJSValue &scripts);

signals:
    void scriptsChanged();

public slots:
    void exitAll();

private slots:
    void onStarted();

private:
    QJSValue m_scripts;
    QVector<QPointer<QFAppScript> > objects;
};
