#include <QtCore>
#include <QVariantMap>
#include "qfhydrate.h"
#include "qfobject.h"
#include "qfstore.h"
#include <functional>

static QVariantMap dehydrator(QObject* source);

static auto dehydratorFunction = [](const QStringList& ignoreList) -> std::function<QVariantMap (QObject *)>
{
    return [ignoreList](QObject* source) {
        QVariantMap dest;
        const auto meta = source->metaObject();
        for (auto i = 0 ; i < meta->propertyCount(); i++)
        {
            const auto property = meta->property(i);
            const auto name = property.name();
            auto stringName = QString{name};

            if (ignoreList.indexOf(stringName) >= 0)
                continue;

            auto value = source->property(name);

            if (value.canConvert<QObject *>())
            {
                auto object = value.value<QObject *>();
                if (!object)
                    continue;

                value = dehydrator(object);
            }
            dest[stringName] = value;
        }

        return dest;
    };
};

static auto dehydrateQObject = dehydratorFunction(QStringList() << QStringLiteral("parent") << QStringLiteral("objectName"));
static auto dehydrateQFObject = dehydratorFunction(QStringList() << QStringLiteral("parent") << QStringLiteral("objectName") << QStringLiteral("children"));
static auto dehydrateQFStore = dehydratorFunction(QStringList() << QStringLiteral("parent") << QStringLiteral("objectName") << QStringLiteral("children") << QStringLiteral("bindSource") << QStringLiteral("redispatchTargets") << QStringLiteral("filterFunctionEnabled"));

/// Default dehydrator function
static QVariantMap dehydrator(QObject* source) {
    if (qobject_cast<QFStore*>(source)) {
        return dehydrateQFStore(source);
    } else if (qobject_cast<QFObject*>(source)) {
        return dehydrateQFObject(source);
    } else {
        return dehydrateQObject(source);
    }
}

/*!
   \qmltype Hydrate
   \inqmlmodule QuickFlux

\code
import QuickFlux 1.1
\endcode

Hydrate provides an interface to rehydrate / hydrate a Store component. Rehydration and dehydration are just another words for deserialize and serialize. It could be used to convert Store into JSON object, and vice versa.

Remarks: Hydrate supports any QObject based type as the target of deserialize and serialize.

\code
Hydrate.rehydrate(store, {
  value1: 1,
  value2: 2.0,
  value3: "",
  value4: {
    subValue1: 1
  }
});

var data = Hydrate.dehydrate(MainStore);
console.log(JSON.stringify(data));
\endcode

It is added since Quick Flux 1.1

*/

QFHydrate::QFHydrate(QObject *parent)
    : QObject{parent}
{
}

/*!
  \qmlmethod Hydrate::rehydrate(target, source)

Deserialize data from source and write to target object.

\code
Hydrate.rehydrate(store, {
  value1: 1,
  value2: 2.0,
  value3: "",
  value4: {
    subValue1: 1
  }
\endcode

 */

void QFHydrate::rehydrate(QObject *dest, const QVariantMap &source)
{
    const auto meta = dest->metaObject();

    auto iter = source.begin();
    while (iter != source.end()) {
        auto key = iter.key().toLocal8Bit();

        if (auto index = meta->indexOfProperty(key.constData()); index < 0)
        {
            qWarning() << QStringLiteral("Hydrate.rehydrate: %1 property is not existed").arg(iter.key());
            iter++;
            continue;
        }

        auto orig = dest->property(key.constData());
        auto value = source[iter.key()];

        if (orig.canConvert<QObject*>())
        {
            if (value.type() != QVariant::Map)
                qWarning() << QStringLiteral("Hydrate.rehydrate: expect a QVariantMap property but it is not: %1");
            else
                rehydrate(orig.value<QObject*>(), value.toMap());

        }
        else if (orig != value)
        {
            dest->setProperty(key.constData(), value);
        }

        iter++;
    }
}

/*!
  \qmlmethod Hydrate::dehydrate(object)

Serialize data from a object

\code
var data = Hydrate.dehydrate(MainStore);
console.log(JSON.stringify(data));
\endcode

 */

QVariantMap QFHydrate::dehydrate(QObject *source)
{
    return dehydrator(source);
}
