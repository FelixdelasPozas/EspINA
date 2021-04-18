/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_ORTHOGONAL_REGION_SLICE_SELECTOR_H
#define ESPINA_ORTHOGONAL_REGION_SLICE_SELECTOR_H

// ESPINA
#include <GUI/Widgets/SliceSelector.h>
#include "OrthogonalRepresentation.h"

class QPushButton;

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      namespace Widgets
      {
        namespace OrthogonalRegion
        {
          class EspinaGUI_EXPORT OrthogonalSliceSelector
          : public GUI::Widgets::SliceSelector
          {
            Q_OBJECT
          public:
            /** \brief OrthogonalSliceSelector constructor.
             * \param[in] region Orthogonal region smartpointer.
             * \param[in] parent Raw pointer of the widget parent of this one.
             *
             */
            explicit OrthogonalSliceSelector(OrthogonalRepresentationSPtr region, QWidget *parent = nullptr);

            /** \brief OrthogonalSliceSelector class copy constructor.
             * \param[in] selector OrthogonalSliceSelector reference.
             * \param[in] parent Raw pointer of the widget parent of this one.
             *
             */
            explicit OrthogonalSliceSelector(OrthogonalSliceSelector &selector, QWidget *parent = nullptr);

            /** \brief OrthogonalSliceSelector class virtual destructor.
             *
             */
            virtual ~OrthogonalSliceSelector();

            virtual QWidget *lowerWidget() const;

            virtual QWidget *upperWidget() const;

            /** \brief Sets the widget tooltiptext label.
             * \param[in] label title of the tooltiptext
             *
             */
            void setLabel(const QString &label)
            { m_label  = label; update();}

            virtual GUI::Widgets::SliceSelectorSPtr clone(RenderView *view, Plane plane);

          protected slots:
            /** \brief Update the widgets.
             *
             */
            void update();

            /** \brief Update the widgets when the left widgets has been clicked.
             *
             */
            void lowerWidgetClicked();

            /** \brief Update the widgets when the right widgets has been clicked.
             *
             */
            void upperWidgetClicked();

          private:
            enum Edge
            {
              Lower = 0,
              Upper = 1
            };

            /** \brief Moves the given edge.
             * \param[in] edge Edge enum value.
             *
             */
            void moveEdge(const Edge edge);

            /** \brief Returns the label of the lower widget.
             *
             */
            const QString lowerLabel() const;

            /** \brief Returns the label of the upper widget.
             *
             */
            const QString upperLabel() const;

            /** \brief Returns the index of the planar view.
             *
             */
            const int normalIndex() const;

            /** \brief Returns the center in Nm of the voxel of the crosshair.
             *
             */
            const Nm voxelCenter() const;

            /** \brief returns the size of the voxel of the plane in Nm.
             *
             */
            const Nm voxelSize() const;

            /** \brief Returns the half size of the voxel of the plane in Nm.
             *
             */
            const Nm halfVoxelSize() const;

            /** \brief Returns the lower slice position of the orthogonal region.
             *
             */
            const Nm lowerSlice() const;

            /** \brief Returns the upper slice position of the orthogonal region.
             *
             */
            const Nm upperSlice() const;

            /** \brief Updates the widgets tooltips.
             *
             */
            void updateLabel();

          private:
            OrthogonalRepresentationSPtr m_representation; /** orthogonal region. */

            RenderView  *m_view;        /** view that contains the representation. */
            Plane        m_plane;       /** plane of the planar view.              */
            QPushButton *m_lowerWidget; /** lower widget.                          */
            QPushButton *m_upperWidget; /** upper widget.                          */
            QString      m_label;       /** label of the representation, if any.   */
          };

        } // namespace OrthogonalRegion
      } // namespace Widges
    } // namespace View
  } // namespace GUI
}// namespace ESPINA


#endif // ESPINA_ORTHOGONAL_REGION_SLICE_SELECTOR_H
