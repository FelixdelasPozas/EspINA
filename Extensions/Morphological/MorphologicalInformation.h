/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef ESPINA_MORPHOLOGICAL_INFORMATION_H
#define ESPINA_MORPHOLOGICAL_INFORMATION_H

#include "Core/EspinaCore_Export.h"

#include <Core/Analysis/Extension.h>

// ITK
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

namespace ESPINA
{
  class EspinaCore_EXPORT MorphologicalInformation
  : public SegmentationExtension
  {
    using LabelObjectType = itk::StatisticsLabelObject<unsigned int, 3>;
    using LabelMapType    = itk::LabelMap<LabelObjectType>;
    using Image2LabelFilterType = itk::LabelImageToShapeLabelMapFilter<itkVolumeType, LabelMapType>;

  public:
    static const Type TYPE;

  public:
    explicit MorphologicalInformation(const InfoCache &cache = InfoCache(),
                                      const State     &state = State());

    virtual ~MorphologicalInformation();

    virtual QString type() const
    { return TYPE; }

    virtual State state() const;

    virtual Snapshot snapshot() const;

    virtual TypeList dependencies() const
    { return TypeList(); }

    virtual bool invalidateOnChange() const
    { return true; }

    virtual InfoTagList availableInformations() const;

    virtual bool validCategory(const QString& classificationName) const
    { return true;}

  protected:
    virtual QVariant cacheFail(const QString& tag) const;

    virtual void onExtendedItemSet(Segmentation* item);

  private:
    void updateInformation() const;

  private:
    Image2LabelFilterType::Pointer m_labelMap;
    mutable LabelObjectType       *m_statistic;

    mutable bool m_validFeret;

    double Size;
    double PhysicalSize;
    double Centroid[3];
    //int    Region[3];
    double BinaryPrincipalMoments[3];
    double BinaryPrincipalAxes[3][3];
    double FeretDiameter;
    double EquivalentEllipsoidSize[3];
  };

}// namespace ESPINA

#endif // ESPINA_MORPHOLOGICAL_INFORMATION_H
