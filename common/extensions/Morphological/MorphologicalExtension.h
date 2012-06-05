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

#include "common/extensions/SegmentationExtension.h"

class pqFilter;

class MorphologicalExtension
: public SegmentationExtension
{
  static const QString ID;

public:
  explicit MorphologicalExtension();
  virtual ~MorphologicalExtension();

  virtual QString id();
  virtual void initialize(Segmentation* seg);

  virtual QStringList dependencies() const
    {return SegmentationExtension::dependencies();}

  virtual QStringList availableRepresentations() const
    {return SegmentationExtension::availableRepresentations();}

  virtual SegmentationRepresentation *representation(QString rep);

  virtual QStringList availableInformations() const
    {return SegmentationExtension::availableInformations();}

  virtual QVariant information(QString info) const;

  virtual SegmentationExtension* clone();

private:
  pqFilter *m_features;

  // Variable to cache filter results
  mutable bool   m_validInfo;
  mutable double m_Size;
  mutable double m_PhysicalSize;
  mutable double m_Centroid[3];
  mutable int    m_Region[3];
  mutable double m_BinaryPrincipalMoments[3];
  mutable double m_BinaryPrincipalAxes[9];
  mutable bool   m_validFeret;
  mutable double m_FeretDiameter;
  mutable double m_EquivalentEllipsoidSize[3];
};

#endif // MORPHOLOGICALEXTENSION_H
