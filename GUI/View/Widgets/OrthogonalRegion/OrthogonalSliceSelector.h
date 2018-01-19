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
          : public SliceSelector
          {
            Q_OBJECT
          public:
            /** \brief OrthogonalSliceSelector constructor.
             *
             */
            explicit OrthogonalSliceSelector(OrthogonalRepresentationSPtr region, QWidget *parent = nullptr);

            explicit OrthogonalSliceSelector(OrthogonalSliceSelector &selector, QWidget *parent = nullptr);

            virtual ~OrthogonalSliceSelector();

            virtual QWidget *lowerWidget() const;

            virtual QWidget *upperWidget() const;

            /** \brief Sets the widget tooltiptext label.
             * \param[in] label title of the tooltiptext
             */
            void setLabel (const QString &label)
            { m_label  = label; update();}

            virtual SliceSelectorSPtr clone(RenderView *view, Plane plane);

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
            void moveEdge(Edge edge);

            QString lowerLabel() const;

            QString upperLabel() const;

            int normalIndex() const;

            Nm voxelCenter() const;

            Nm voxelSize() const;

            Nm halfVoxelSize() const;

            Nm lowerSlice() const;

            Nm upperSlice() const;

            void updateLabel();

          private:
            OrthogonalRepresentationSPtr m_representation;

            RenderView *m_view;
            Plane       m_plane;

            QPushButton *m_lowerWidget;
            QPushButton *m_upperWidget;

            QString m_label;
          };

        }
      }
    }
  }
}// namespace ESPINA


#endif // ESPINA_ORTHOGONAL_REGION_SLICE_SELECTOR_H
