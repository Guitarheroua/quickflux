#include <QtCore>
#include <QMetaObject>
#include "qfkeytable.h"

/*!
  \qmltype KeyTable
  \brief Defines a key table

  KeyTable is an object with properties equal to its key name. Once it's construction is completed,
  it will search all of its string property. If it is a string type and not assigned to any value,
  it will set its value by its name. It can be used to create ActionTypes.qml in QuickFlux Application.

Example

\code

KeyTable {

    // It will be set to "customField1" in Component.onCompleted callback.
    property string customField1;

    // Since it is already assigned a value, KeyTable will not modify this property.
    property string customField2 : "value";

}

\endcode


 */

static QMap<int,QString> createTypes() {
    static QMap<int,QString> types
    {
        {QVariant::String, QStringLiteral("QString")},
        {QVariant::Int, QStringLiteral("int")},
        {QVariant::Double, QStringLiteral("qreal")},
        {QVariant::Bool, QStringLiteral("bool")},
        {QVariant::PointF, QStringLiteral("QPointF")},
        {QVariant::RectF, QStringLiteral("QRectF")},
    };

    return types;
}

QFKeyTable::QFKeyTable(QObject *parent)
    : QObject{parent}
{
}

QString QFKeyTable::genHeaderFile(const QString& className)
{
    auto header = QStringList{} << QStringLiteral("#pragma once") << QStringLiteral("#include <QString>\n");
    auto clazz = QStringList{} << QStringLiteral("class %1 {\n").arg(className) << QStringLiteral("public:\n");

    const auto meta = metaObject();
    auto types = createTypes();

    auto includedPointHeader = false;
    auto includedRectHeader = false;
    auto count = meta->propertyCount();

    for (auto i = 0 ; i < count ; i++) {
        const auto p = meta->property(i);

        QString name(p.name());

        if (name == QStringLiteral("objectName"))
            continue;

        if (types.contains(p.type()))
        {
            clazz << QStringLiteral("    static %2 %1;\n").arg(name, types[p.type()]);

            if (p.type() == QVariant::PointF && !includedPointHeader)
            {
                includedPointHeader = true;
                header << QStringLiteral("#include <QPointF>");
            }
            else if (p.type() == QVariant::RectF && !includedRectHeader)
            {
                includedRectHeader = true;
                header << QStringLiteral("#include <QRectF>");
            }
        }

    }

    clazz << QStringLiteral("};\n");

    auto content = QStringList{} << header << clazz;

    return content.join('\n');
}

QString QFKeyTable::genSourceFile(const QString &className, const QString &headerFile)
{
    auto source = QStringList{} << QStringLiteral("#include \"%1\"\n").arg(headerFile);
    auto types = createTypes();
    const auto meta = metaObject();

    auto count = meta->propertyCount();
    for (auto i = 0 ; i < count ; i++)
    {
        const auto p = meta->property(i);
        QString name(p.name());
        if (name == QStringLiteral("objectName"))
            continue;

        if (types.contains(p.type()))
        {
            auto v = property(p.name());

            switch (p.type())
            {
            case QVariant::String:
                source << QStringLiteral("%4 %1::%2 = \"%3\";\n").arg(className, p.name(), v.toString(), types[p.type()]);
                break;
            case QVariant::PointF:
            {
                auto pt = v.toPointF();
                source << QStringLiteral("QPointF %1::%2 = QPointF(%3,%4);\n").arg(className, p.name(), QString::number(pt.x()), QString::number(pt.y()));
                break;
            }
            case QVariant::RectF:
            {
                auto rect = v.toRectF();
                source << QStringLiteral("QRectF %1::%2 = QRect(%3,%4,%5,%6);\n")
                              .arg(className
                                   , p.name()
                                       , QString::number(rect.x())
                                       , QString::number(rect.y())
                                       , QString::number(rect.width())
                                       , QString::number(rect.height()));
                break;
            }
            default:
                source << QStringLiteral("%4 %1::%2 = %3;\n").arg(className, p.name(), v.toString(), types[p.type()]);
                break;
            }
        }
    }

    return source.join('\n');
}

void QFKeyTable::classBegin()
{
}

void QFKeyTable::componentComplete()
{
    const auto meta = metaObject();

    auto count = meta->propertyCount();
    for (auto i = 0 ; i < count ; i++)
    {
        const auto p = meta->property(i);
        QString name(p.name());

        if (p.type() != QVariant::String || name == QStringLiteral("objectName"))
            continue;

        if (auto v = property(p.name()); !v.isNull())
            continue;

        setProperty(p.name(), name);
    }

}
