#include "EspinaTypes.h"

using namespace EspINA;

QString EspINA::condition(const QString &icon, const QString &description)
{
  return QString("<img src='%1' width=16 height=16>: %2").arg(icon).arg(description);
}
