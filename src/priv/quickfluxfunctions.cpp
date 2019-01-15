#include <QtCore>
#include "quickfluxfunctions.h"

void QuickFlux::printException(QJSValue value)
{
    if (value.isError()) {
        QString message = QString("%1:%2: %3: %4")
                          .arg(value.property(QLatin1String{"fileName"}).toString())
                          .arg(value.property(QLatin1String{"lineNumber"}).toString())
                          .arg(value.property(QLatin1String{"name"}).toString())
                          .arg(value.property(QLatin1String{"message"}).toString());
        qWarning() << message;
    }
}
