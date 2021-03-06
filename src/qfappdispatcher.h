#ifndef QFAPPDISPATCHER_H
#define QFAPPDISPATCHER_H

#include <QObject>
#include "qfdispatcher.h"

class QFAppDispatcher : public QFDispatcher
{
    Q_OBJECT
public:
    explicit QFAppDispatcher(QObject *parent = nullptr);

    /// Obtain the singleton instance of AppDispatcher for specific QQmlEngine
    static QFAppDispatcher* instance(QQmlEngine* engine);

    /// Obtain a singleton object from package for specific QQmlEngine
    static QObject* singletonObject(QQmlEngine* engine, const QString &package,
                                    int versionMajor,
                                    int versionMinor,
                                    const QString &typeName);
};

#endif // QFAPPDISPATCHER_H
