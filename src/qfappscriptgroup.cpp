#include "qfappscriptgroup.h"

/*! \qmltype AppScriptGroup
    \inqmlmodule QuickFlux

    AppScriptGroup hold a group of AppScript objects which are mutually exclusive in execution.
    Whatever a AppScript is going to start, it will terminate all other AppScript objects.
    So that only one AppScript is running at a time.

\code

Item {

    AppScript {
        id: script1
        script: {
            // write script here
        }
    }

    AppScript {
        id: script2
        script: {
            // write script here
        }
    }

    AppScriptGroup {
        scripts: [script1, script2]
    }

    Component.onCompleted: {

        script1.run();

        script2.run();

        // At this point, AppScriptGroup will force script1 to terminate since script2 has been started.

    }

}

\endcode

 */

QFAppScriptGroup::QFAppScriptGroup(QQuickItem* parent)
    : QQuickItem{parent}
{
}

/*! \qmlproperty array AppScriptGroup::scripts
   This property hold an array of AppScript object.
   They are mutually exclusive in execution.

\code

AppScript {
    id: script1
}

AppScript {
    id: script2
}

AppScriptGroup {
    scripts: [script1, script2]
}

\endcode
 */

QJSValue QFAppScriptGroup::scripts() const
{
    return m_scripts;
}

void QFAppScriptGroup::setScripts(const QJSValue &scripts)
{
    for (const auto &object : objects)
        if (object.data())
            object->disconnect(this);

    objects.clear();
    m_scripts = scripts;

    if (!scripts.isArray())
    {
        qWarning() << QStringLiteral("AppScriptGroup: Invalid scripts property");
        return;
    }

    auto count = scripts.property(QStringLiteral("length")).toInt();

    for (auto i = 0 ; i < count ; i++)
    {
        auto item = scripts.property(static_cast<quint32>(i));

        if (!item.isQObject())
        {
            qWarning() << QStringLiteral("AppScriptGroup: Invalid scripts property");
            continue;
        }

        auto object = qobject_cast<QFAppScript*>(item.toQObject());

        if (!object)
        {
            qWarning() << QStringLiteral("AppScriptGroup: Invalid scripts property");
            continue;
        }

        objects << object;
        connect(object, &QFAppScript::started, this, &QFAppScriptGroup::onStarted);
    }

    emit scriptsChanged();
}

/*! \qmlmethod AppScriptGroup::exitAll()

  Terminate all AppScript objects

 */

void QFAppScriptGroup::exitAll()
{
    for (const auto &object : objects)
        if (object.data())
            object->exit();
}

void QFAppScriptGroup::onStarted()
{
    auto source = qobject_cast<QFAppScript*>(sender());

    for (const auto &object : objects)
    {
        if (object.isNull())
            continue;

        if (object.data() != source)
            object->exit();
    }
}

