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

#include "common/EspinaTypes.h"

// ITK
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

class MorphologicalExtension
: public SegmentationExtension
{
  static const ExtId ID;

  typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
  typedef itk::LabelMap<LabelObjectType> LabelMapType;
  typedef itk::LabelImageToShapeLabelMapFilter<EspinaVolume, LabelMapType> Image2LabelFilterType;


public:
  explicit MorphologicalExtension();
  virtual ~MorphologicalExtension();

  virtual ExtId id();
  virtual void initialize(Segmentation* seg);

  virtual ExtIdList dependencies() const
  {
    return SegmentationExtension::dependencies();
  }

  virtual InfoList availableInformations() const
  {
    return SegmentationExtension::availableInformations();
  }

  virtual RepList availableRepresentations() const
  {
    return SegmentationExtension::availableRepresentations();
  }

  virtual QVariant information(InfoTag tag) const;

  virtual SegmentationRepresentation *representation(QString rep);

  virtual SegmentationExtension* clone();

private:
  Image2LabelFilterType::Pointer m_labelMap;
  mutable LabelObjectType       *m_statistic;
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
