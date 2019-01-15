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
        {QVariant::String, QLatin1String{"QString"}},
        {QVariant::Int, QLatin1String{"int"}},
        {QVariant::Double, QLatin1String{"qreal"}},
        {QVariant::Bool, QLatin1String{"bool"}},
        {QVariant::PointF, QLatin1String{"QPointF"}},
        {QVariant::RectF, QLatin1String{"QRectF"}},
    };

    return types;
}

QFKeyTable::QFKeyTable(QObject *parent) : QObject(parent)
{

}

QString QFKeyTable::genHeaderFile(const QString& className)
{

    QStringList header;
    QStringList clazz;
    auto includedPointHeader = false;
    auto includedRectHeader = false;

    header << QLatin1String{"#pragma once"};
    header << QLatin1String{"#include <QString>\n"};

    clazz << QString("class %1 {\n").arg(className);
    clazz << QLatin1String{"public:\n"};

    const auto meta = metaObject();

    auto types = createTypes();

    int count = meta->propertyCount();
    for (int i = 0 ; i < count ; i++) {
        const auto p = meta->property(i);

        QString name(p.name());

        if (name == QLatin1String{"objectName"}) {
            continue;
        }

        if (types.contains(p.type())) {
            clazz << QString("    static %2 %1;\n").arg(name).arg(types[p.type()]);

            if (p.type() == QVariant::PointF && !includedPointHeader) {
                includedPointHeader = true;
                header << QLatin1String{"#include <QPointF>"};
            } else if (p.type() == QVariant::RectF && !includedRectHeader) {
                includedRectHeader = true;
                header << QLatin1String{"#include <QRectF>"};
            }
        }

    }

    clazz << QLatin1String{"};\n"};

    QStringList content;
    content << header << clazz;

    return content.join("\n");
}

QString QFKeyTable::genSourceFile(const QString &className, const QString &headerFile)
{
    auto types = createTypes();

    QStringList source;

    source << QString("#include \"%1\"\n").arg(headerFile);

    const auto meta = metaObject();

    int count = meta->propertyCount();
    for (int i = 0 ; i < count ; i++) {
        const auto p = meta->property(i);
        QString name(p.name());
        if (name == QLatin1String{"objectName"}) {
            continue;
        }

        if (types.contains(p.type())) {
            QVariant v = property(p.name());

            if (p.type() == QVariant::String) {
                source << QString("%4 %1::%2 = \"%3\";\n")
                          .arg(className)
                          .arg(p.name())
                          .arg(v.toString())
                          .arg(types[p.type()]);

            } else if (p.type() == QVariant::PointF) {
                QPointF pt = v.toPointF();

                source << QString("QPointF %1::%2 = QPointF(%3,%4);\n")
                          .arg(className)
                          .arg(p.name())
                          .arg(pt.x())
                          .arg(pt.y());

            } else if (p.type() == QVariant::RectF) {

                QRectF rect = v.toRectF();

                source << QString("QRectF %1::%2 = QRect(%3,%4,%5,%6);\n")
                          .arg(className)
                          .arg(p.name())
                          .arg(rect.x())
                          .arg(rect.y())
                          .arg(rect.width())
                          .arg(rect.height());

            } else {

                source << QString("%4 %1::%2 = %3;\n")
                          .arg(className)
                          .arg(p.name())
                          .arg(v.toString())
                          .arg(types[p.type()]);
            }
        }
    }

    return source.join("\n");
}

void QFKeyTable::classBegin()
{

}

void QFKeyTable::componentComplete()
{
    const auto meta = metaObject();

    int count = meta->propertyCount();
    for (int i = 0 ; i < count ; i++) {
        const auto p = meta->property(i);
        QString name(p.name());
        if (p.type() != QVariant::String ||
            name == QLatin1String{"objectName"}) {
            continue;
        }

        auto v = property(p.name());
        if (!v.isNull()) {
            continue;
        }

        setProperty(p.name(), name);
    }

}
