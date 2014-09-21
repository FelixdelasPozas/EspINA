/*
 *    
 *    Copyright (C) 2014  Juan Morales del Olmo <juan.morales@upm.es>
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

#ifndef APPOSITION_SURFAC_EXTENSION_H
#define APPOSITION_SURFAC_EXTENSION_H

#include "AppositionSurfacePlugin_Export.h"

// ESPINA
#include <Core/Analysis/Extension.h>
#include <Core/EspinaTypes.h>
#include <Core/Utils/NmVector3.h>
#include <GUI/Model/SegmentationAdapter.h>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace ESPINA
{
  const QString SAS = QObject::tr("SAS");

  class AppositionSurfacePlugin_EXPORT AppositionSurfaceExtension
  : public SegmentationExtension
  {
    public:
      static const Type TYPE;

    public:
      explicit AppositionSurfaceExtension(const SegmentationExtension::InfoCache &cache);
      virtual ~AppositionSurfaceExtension();

      /** brief Implements Extension:type().
       *
       */
      virtual Type type() const
      { return TYPE; }

      /** brief Implements Extension::invalidateOnChange().
       *
       */
      virtual bool invalidateOnChange() const
      { return true; }

      /** brief Implements Extension::state().
       *
       */
      virtual State state() const
      { return State(); }

      /** brief Implements Extension::snapshot().
       *
       */
      virtual Snapshot snapshot() const
      { return Snapshot(); }

      /** brief Implements Extension::dependencies().
       *
       */
      virtual TypeList dependencies() const
      { return TypeList(); }

      /** brief Implements Extension::availableInformations().
       *
       */
      virtual InfoTagList availableInformations() const;

      /** brief Implements SegmentationExtension::validCategory().
       *
       */
      virtual bool validCategory(const QString &classificationName) const;

      /** brief Sets the origin segmentation for the SAS segmentation. Right now there is no
       * other way to inject that information to the SAS extension.
       */
      void setOriginSegmentation(SegmentationAdapterSPtr segmentation)
      { m_originSegmentation = segmentation; }

    protected:
      /** brief Implements Extension::cacheFail().
       *
       */
      virtual QVariant cacheFail(const InfoTag &tag) const;

      /** brief Implements Extension::onExtendedItemSet().
       *
       */
      virtual void onExtendedItemSet(Segmentation* item);

  private:
      /** brief Computes SAS area.
       * \param[in] asMesh, SAS polydata smart pointer.
       */
      Nm computeArea(const vtkSmartPointer<vtkPolyData> &asMesh) const;

      /** brief Returns true if the specified cell is part of the perimeter.
       * \param[in] asMesh, SAS polydata smart pointer.
       * \param[in] cellId, id of the cell.
       * \param[in] p1
       * \param[in] p2
       */
      bool isPerimeter(const vtkSmartPointer<vtkPolyData> &asMesh, const vtkIdType cellId, const vtkIdType p1, const vtkIdType p2) const;

      /** brief Returns the perimeter of the SAS.
       * \param[in] asMesh, SAS polydata smart pointer.
       *
       */
      Nm computePerimeter(const vtkSmartPointer<vtkPolyData> &asMesh) const;

      /** brief Returns the projection of the SAS polydata to a plane.
       * \param[in] asMesh, SAS polydata smart pointer.
       */
      vtkSmartPointer<vtkPolyData> projectPolyDataToPlane(const vtkSmartPointer<vtkPolyData> &mesh) const;

      /** brief Returns the tortuosity of the SAS.
       * \param[in] asMesh, SAS polydata smart pointer.
       * \param[in] asArea, area of the SAS.
       */
      double computeTortuosity(const vtkSmartPointer<vtkPolyData> &asMesh, const Nm asArea) const;

      /** brief Computes SAS curvatures.
       * \param[in] asMesh, SAS polydata smart pointer.
       * \param[out] gaussCurvature
       * \param[out] meanCurvature
       * \param[out] minCurvature
       * \param[out] maxCurvature
       */
      void computeCurvatures(const vtkSmartPointer<vtkPolyData> &asMesh,
                             vtkSmartPointer<vtkDoubleArray> gaussCurvature,
                             vtkSmartPointer<vtkDoubleArray> meanCurvature,
                             vtkSmartPointer<vtkDoubleArray> minCurvature,
                             vtkSmartPointer<vtkDoubleArray> maxCurvature) const;
    
      /** brief Returns true if the information has been calculated. Computes all available
       * informations.
       *
       */
      bool computeInformation() const;

  private:
      SegmentationAdapterSPtr m_originSegmentation;

  };

  using AppositionSurfaceExtensionPtr  = AppositionSurfaceExtension *;
  using AppositionSurfaceExtensionSPtr = std::shared_ptr<AppositionSurfaceExtension>;

} // namespace ESPINA

#endif // APPOSITION_SURFAC_EXTENSION_H
