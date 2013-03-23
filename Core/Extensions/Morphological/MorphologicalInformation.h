/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
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


#ifndef MORPHOLOGICALINFORMATION_H
#define MORPHOLOGICALINFORMATION_H

#include "Core/Extensions/SegmentationExtension.h"

#include "Core/EspinaTypes.h"

// ITK
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

namespace EspINA
{
  const ModelItem::ExtId MorphologicalInformationID = "MorphologicalExtension";

  class MorphologicalInformation
  : public Segmentation::Information
  {

    typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
    typedef itk::LabelMap<LabelObjectType> LabelMapType;
    typedef itk::LabelImageToShapeLabelMapFilter<itkVolumeType, LabelMapType> Image2LabelFilterType;

    struct ExtensionData
    {
      ExtensionData();

      double Size;
      double PhysicalSize;
      double Centroid[3];
      //int    Region[3];
      double BinaryPrincipalMoments[3];
      double BinaryPrincipalAxes[3][3];
      double FeretDiameter;
      double EquivalentEllipsoidSize[3];
    };

    typedef Cache<SegmentationPtr, ExtensionData> ExtensionCache;

    static ExtensionCache s_cache;

    const static QString EXTENSION_FILE;

  public:
    explicit MorphologicalInformation();
    virtual ~MorphologicalInformation();

    virtual ModelItem::ExtId id();

    virtual ModelItem::ExtIdList dependencies() const
    { return Segmentation::Extension::dependencies(); }

    virtual Segmentation::InfoTagList availableInformations() const;

    virtual bool validTaxonomy(const QString &qualifiedName) const 
    { return true;}

    virtual QVariant information(const Segmentation::InfoTag &tag);


    virtual bool isCacheFile(const QString &file) const
    { return EXTENSION_FILE == file; }

    virtual void loadCache(QuaZipFile  &file,
                           const QDir  &tmpDir,
                           IEspinaModel *model);

    virtual bool saveCache(Snapshot &snapshot);

    virtual Segmentation::InformationExtension clone();

    virtual void initialize();

    virtual void invalidate(SegmentationPtr segmentation = NULL);

  private:
    void updateInformation();

  private:
    Image2LabelFilterType::Pointer m_labelMap;
    mutable LabelObjectType       *m_statistic;

    mutable bool m_validFeret;
  };

}// namespace EspINA

#endif // MORPHOLOGICALINFORMATION_H
