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

#ifndef TAXONOMYCOLORENGINE_H
#define TAXONOMYCOLORENGINE_H

#include "common/gui/ColorEngine.h"
#include <QMap>

class TaxonomyNode;
class TaxonomyColorEngine
: public ColorEngine
{
public:
  static TaxonomyColorEngine *instance()
  {
    if (NULL == m_singleton)
      m_singleton = new TaxonomyColorEngine();
    return m_singleton;
  }

  virtual QColor color(const Segmentation* seg);
  virtual vtkSmartPointer< vtkLookupTable > lut(const Segmentation* seg);
  void updateTaxonomyColor(TaxonomyNode *tax);

private:
  explicit TaxonomyColorEngine(){}

  QMap<QString, vtkSmartPointer<vtkLookupTable> > m_LUT;
  static TaxonomyColorEngine *m_singleton;
};

#endif // TAXONOMYCOLORENGINE_H
