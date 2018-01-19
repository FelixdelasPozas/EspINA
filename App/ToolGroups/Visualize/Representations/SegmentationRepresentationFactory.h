/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_SEGMENTATION_REPRESENTATION_FACTORY_H
#define ESPINA_SEGMENTATION_REPRESENTATION_FACTORY_H

#include <Support/Representations/BasicRepresentationSwitch.h>
#include <Support/Representations/RepresentationFactory.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/Settings/SegmentationSlicePoolSettings.h>
#include <GUI/Widgets/NumericalInput.h>

namespace ESPINA
{
  /** \class SegmentationRepresentationFactory.
   * \brief Representation factory for segmentation's representations.
   *
   */
  class SegmentationRepresentationFactory
  : public RepresentationFactory
  {
    public:
      /** \brief SegmentationRepresentationFactory class constructor.
       *
       */
      explicit SegmentationRepresentationFactory();

    private:
      /** \brief Creates the segmentation representations.
       * \param[in] context application context.
       * \param[in] supportedViews view flags of the views the representations will be shown.
       *
       */
      virtual Representation doCreateRepresentation(Support::Context& context, ViewTypeFlags supportedViews) const;

      /** \brief Creates the slice representations for segmentations.
       * \param[out] representation Representation object.
       * \param[in] context application context.
       * \param[in] supportedViews view flags of the views the representations will be shown.
       *
       */
      void createSliceRepresentation(Representation &representation, Support::Context &context, ViewTypeFlags supportedViews) const;

      /** \brief Creates the slice contour representation.
       * \param[out] representation Representation object.
       * \param[in] context application context.
       *
       */
      void createContourRepresentation(Representation &representation, Support::Context &context) const;

      /** \brief Creates the skeleton representation and adds it to the Representation object.
       * \param[out] representation Representation object.
       * \param[in] context application context.
       * \param[in] supportedViews view flags of the views the representations will be shown.
       *
       */
      void createSkeletonRepresentation(Representation &representation, Support::Context &context, ViewTypeFlags supportedViews) const;

      /** \brief Creates the mesh representation and adds it to the Representation object.
       * \param[out] representation Representation object.
       * \param[in] context application context.
       *
       */
      void createMeshRepresentation(Representation &representation, Support::Context &context) const;

      /** \brief Creates the volumetric representation and adds it to the representation object.
       * \param[out] representation Representation object.
       * \param[in] context application context.
       *
       */
      void createVolumetricRepresentation(Representation &representation, Support::Context &context) const;

      /** \brief Sets the order of the representation for the given tool.
       * \param[in] order order string id.
       * \param[inout] representation object tool.
       *
       */
      void groupSwitch(const QString &order, Support::Widgets::ToolSPtr tool) const;

      /** \brief Creates the managers related to segmentation representations.
       * \param[out] representation Representation object.
       * \param[in] context application context.
       * \param[in] supportedViews view flags of the vies the representations will be shown.
       *
       */
      void createMiscellaneousManagers(Representation &representation, Support::Context &context, ViewTypeFlags supportedViews) const;

    private:
      static const unsigned int WINDOW_SIZE; /** window size for buffered representations. */
  };
}

#endif // ESPINA_SEGMENTATION_REPRESENTATION_FACTORY_H
