#include <QtCore>
#include "quickfluxfunctions.h"

void QuickFlux::printException(const QJSValue &value)
{
    if (value.isError()) {
        QString message = QString("%1:%2: %3: %4")
                          .arg(value.property(QLatin1String{"fileName"}).toString()
                              , value.property(QLatin1String{"lineNumber"}).toString()
                              , value.property(QLatin1String{"name"}).toString()
                              , value.property(QLatin1String{"message"}).toString());
        qWarning() << message;
    }
}
