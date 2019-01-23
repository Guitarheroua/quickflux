#include <QtCore>
#include "quickfluxfunctions.h"

void QuickFlux::printException(const QJSValue &value)
{
    if (value.isError()) {
        auto message = QStringLiteral("%1:%2: %3: %4")
                           .arg(value.property(QStringLiteral("fileName")).toString()
                                    , value.property(QStringLiteral("lineNumber")).toString()
                                    , value.property(QStringLiteral("name")).toString()
                                    , value.property(QStringLiteral("message")).toString());
        qWarning() << message;
    }
}
