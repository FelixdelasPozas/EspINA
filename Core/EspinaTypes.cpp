#include "EspinaTypes.h"

using namespace ESPINA;

QString ESPINA::condition(const QString &icon, const QString &description)
{
  return QString("<table style=\"margin: 0px\">"
                 " <tr>"
                 " <td valign=\"top\"><img src='%1' width=16 height=16></td> <td valign=\"center\">: %2</td>"
                 " </tr>"
                 "</table>"
                ).arg(icon).arg(description);
  //return QString("<img src='%1' width=16 height=16>: %2").arg(icon).arg(description);
}
