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
#ifndef APPOSITIONSURFACEXTENSION_H
#define APPOSITIONSURFACEXTENSION_H

// EspINA
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/EspinaTypes.h>

namespace EspINA
{
  const QString SAS = QObject::tr("SAS");

  class AppositionSurfaceExtension
  : public Segmentation::Information
  {
      struct ExtensionData
      {
          explicit ExtensionData();

          Nm Area;
          Nm Perimeter;
          double Tortuosity;
          QString SynapticSource;
          double MeanGaussCurvature;
          double StdDevGaussCurvature;
          double MeanMeanCurvature;
          double StdDevMeanCurvature;
          double MeanMinCurvature;
          double StdDevMinCurvature;
          double MeanMaxCurvature;
          double StdDevMaxCurvature;
      };

      typedef Cache<SegmentationPtr, ExtensionData> ExtensionCache;

      static ExtensionCache s_cache;

      const static QString EXTENSION_FILE;

    public:
      static const ModelItem::ExtId ID;

      static const Segmentation::InfoTag AREA;
      static const Segmentation::InfoTag PERIMETER;
      static const Segmentation::InfoTag TORTUOSITY;
      static const Segmentation::InfoTag SYNAPSE;
      static const Segmentation::InfoTag COMPUTATION_TIME;
      static const Segmentation::InfoTag MEAN_GAUSS_CURVATURE;
      static const Segmentation::InfoTag STD_DEV_GAUS_CURVATURE;
      static const Segmentation::InfoTag MEAN_MEAN_CURVATURE;
      static const Segmentation::InfoTag STD_DEV_MEAN_CURVATURE;
      static const Segmentation::InfoTag MEAN_MIN_CURVATURE;
      static const Segmentation::InfoTag STD_DEV_MIN_CURVATURE;
      static const Segmentation::InfoTag MEAN_MAX_CURVATURE;
      static const Segmentation::InfoTag STD_DEV_MAX_CURVATURE;

    public:
      explicit AppositionSurfaceExtension();
      virtual ~AppositionSurfaceExtension();

      virtual ModelItem::ExtId id();

      virtual ModelItem::ExtIdList dependencies() const
      { return Segmentation::Extension::dependencies(); }

      virtual Segmentation::InfoTagList availableInformations() const;

      virtual bool validTaxonomy(const QString &qualifiedName) const;

      virtual void setSegmentation(SegmentationPtr seg);

      virtual QVariant information(const Segmentation::InfoTag &tag);

      virtual bool isCacheFile(const QString &file) const;

      virtual void loadCache(QuaZipFile &file, const QDir &tmpDir, IEspinaModel *model);

      virtual bool saveCache(Snapshot &cacheList);

      virtual Segmentation::InformationExtension clone();

      virtual void initialize();

      virtual void invalidate(SegmentationPtr segmentation = 0);

  private:
    Nm computeArea(vtkPolyData *asMesh) const;

    bool isPerimeter(vtkPolyData *asMesh, vtkIdType cellId, vtkIdType p1, vtkIdType p2) const;

    Nm computePerimeter(vtkPolyData *asMesh) const;

    vtkSmartPointer<vtkPolyData> projectPolyDataToPlane(vtkPolyData* mesh) const;

    double computeTortuosity(vtkPolyData *asMesh, Nm asArea) const;

    bool computeInformation();

  };

  typedef QSharedPointer<AppositionSurfaceExtension> AppositionSurfaceExtensionSPtr;

} // namespace EspINA

#endif // APPOSITIONSURFACEEXTENSION_H
