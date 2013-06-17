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


#ifndef USERCOLORENGINE_H
#define USERCOLORENGINE_H

#include "EspinaCore_Export.h"

#include "Core/ColorEngines/IColorEngine.h"

#include <QMap>

namespace EspINA
{
  class EspinaCore_EXPORT UserColorEngine
  : public ColorEngine
  {
  public:
    explicit UserColorEngine();

    virtual QColor color(SegmentationPtr seg);
    virtual LUTPtr lut  (SegmentationPtr seg);
    virtual ColorEngine::Composition supportedComposition() const
    { return ColorEngine::Color; }

  private:
    QColor nextColor();

  private:
    QMap<QString, QColor> m_userColors;
    QList<QColor>         m_colors;
    int                   m_lastColor;
    QMap<QString, vtkSmartPointer<vtkLookupTable> > m_LUT;
  };

}// namespace EspINA

#endif // USERCOLORENGINE_H
