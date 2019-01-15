#pragma once

#include <QObject>
#include <QQmlParserStatus>

class QFKeyTable : public QObject, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

public:
    explicit QFKeyTable(QObject *parent = nullptr);

public slots:
    QString genHeaderFile(const QString& className);
    QString genSourceFile(const QString& className, const QString& headerFile);

protected:
    void classBegin();
    void componentComplete();

};
