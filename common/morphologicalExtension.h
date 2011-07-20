/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef MORPHOLOGICALEXTENSION_H
#define MORPHOLOGICALHEXTENSION_H

#include <EspinaPlugin.h>

class vtkFilter;
class pqScalarsToColors;
class vtkProduct;

class MorphologicalExtension : public ISegmentationExtension
{
public:
  static const ExtensionId ID;
  
public:
  MorphologicalExtension();
  virtual ~MorphologicalExtension();
  
  virtual ExtensionId id();
  virtual void initialize(Segmentation* seg);
  virtual ISegmentationRepresentation *representation(QString rep);
  virtual QVariant information(QString info);
  
  virtual ISegmentationExtension* clone();
  
private:
  vtkFilter *m_features;
  double m_centroid[3];
  bool m_init;
};

#endif // MORPHOLOGICALEXTENSION_H
