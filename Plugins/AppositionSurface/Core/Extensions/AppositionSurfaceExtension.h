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
#include <Core/Types.h>
#include <Core/Utils/Vector3.hxx>
#include <Core/Analysis/Extensions.h>
#include <GUI/Model/SegmentationAdapter.h>

// VTK
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

namespace ESPINA
{
  class ASExtensionFactory;

  /** \class AppositionSurfaceExtension
   * \brief Extends the SAS segmentations with information.
   *
   */
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceExtension
  : public Core::SegmentationExtension
  {
    public:
      static const Type TYPE;
      static const Type OLD_TYPE;

    public:
      /** \brief AppositionSurfaceExtension class virtual destructor.
       *
       */
      virtual ~AppositionSurfaceExtension()
      {}

      virtual Type type() const
      { return TYPE; }

      virtual bool invalidateOnChange() const
      { return true; }

      virtual State state() const
      { return State(); }

      virtual Snapshot snapshot() const
      { return Snapshot(); }

      virtual const TypeList dependencies() const
      { return TypeList(); }

      virtual const InformationKeyList availableInformation() const;

      virtual bool validCategory(const QString &classificationName) const;

      virtual bool validData(const OutputSPtr output) const;

      /** \brief Helper method that adds the SAS prefix to the given key.
       * \param[in] value segmentation extension key.
       *
       */
      static Core::SegmentationExtension::Key addSASPrefix(const Key& value);

      /** \brief Helper method that removes the SAS prefix from the given key.
       * \param[in] value segmentation extension key.
       *
       */
      static Core::SegmentationExtension::Key removeSASPrefix(const Key &value);

    protected:
      virtual QVariant cacheFail(const InformationKey &tag) const;

      virtual void onExtendedItemSet(Segmentation* item);

  private:
      /** \brief AppositionSurfaceExtension class constructor.
       * \param[in] infocache InfoCache object reference.
       */
      explicit AppositionSurfaceExtension(const Core::SegmentationExtension::InfoCache &cache);

      /** \brief Computes SAS area.
       * \param[in] asMesh SAS polydata smart pointer.
       */
      Nm computeArea(const vtkSmartPointer<vtkPolyData> asMesh) const;

      /** \brief Returns true if the specified cell is part of the perimeter.
       * \param[in] asMesh SAS polydata smart pointer.
       * \param[in] cellId id of the cell.
       * \param[in] p1
       * \param[in] p2
       */
      bool isPerimeter(const vtkSmartPointer<vtkPolyData> asMesh, const vtkIdType cellId, const vtkIdType p1, const vtkIdType p2) const;

      /** \brief Returns the perimeter of the SAS.
       * \param[in] asMesh SAS polydata smart pointer.
       *
       */
      Nm computePerimeter(const vtkSmartPointer<vtkPolyData> asMesh) const;

      /** \brief Returns the projection of the SAS polydata to a plane.
       * \param[in] asMesh SAS polydata smart pointer.
       */
      vtkSmartPointer<vtkPolyData> projectPolyDataToPlane(const vtkSmartPointer<vtkPolyData> mesh) const;

      /** \brief Returns the tortuosity of the SAS.
       * \param[in] asMesh SAS polydata smart pointer.
       * \param[in] asArea area of the SAS.
       */
      double computeTortuosity(const vtkSmartPointer<vtkPolyData> asMesh, const Nm asArea) const;

      /** \brief Computes SAS curvatures.
       * \param[in] asMesh SAS polydata smart pointer.
       * \param[out] gaussCurvature
       * \param[out] meanCurvature
       * \param[out] minCurvature
       * \param[out] maxCurvature
       */
      void computeCurvatures(const vtkSmartPointer<vtkPolyData> asMesh,
                             vtkSmartPointer<vtkDoubleArray> gaussCurvature,
                             vtkSmartPointer<vtkDoubleArray> meanCurvature,
                             vtkSmartPointer<vtkDoubleArray> minCurvature,
                             vtkSmartPointer<vtkDoubleArray> maxCurvature) const;
    
      /** \brief Returns true if the information has been calculated. Computes all available
       * informations.
       *
       */
      bool computeInformation() const;

      /** \brief Tries to get the SAS origin synapse from the model.
       *
       */
      void obtainOriginSynapse() const;

      static const QString SAS_PREFIX;

      mutable bool             m_hasErrors; /** false if the computation process finished without errors, true otherwise. */
      mutable SegmentationSPtr m_synapse;   /** synapse the SAS is generated from.                                        */

      friend class ASExtensionFactory;
  };

  using AppositionSurfaceExtensionPtr  = AppositionSurfaceExtension *;
  using AppositionSurfaceExtensionSPtr = std::shared_ptr<AppositionSurfaceExtension>;

} // namespace ESPINA

#endif // APPOSITION_SURFAC_EXTENSION_H
