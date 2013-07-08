/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef MARGINDETECTOR_H
#define MARGINDETECTOR_H

#include "EspinaCore_Export.h"

#include <QThread>

#include <Core/EspinaTypes.h>

namespace EspINA
{
class AdaptiveEdges;

class EspinaCore_EXPORT EdgeDetector
: public QThread
{
public:
  explicit EdgeDetector(AdaptiveEdges *extension, QObject* parent = 0);
  virtual ~EdgeDetector();

protected:
  virtual void run();

private:
  AdaptiveEdges *m_extension;
};

}// namespace EspINA

#endif // MARGINDETECTOR_H