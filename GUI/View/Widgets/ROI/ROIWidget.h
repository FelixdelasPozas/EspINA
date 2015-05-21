/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_ROI_WIDGET_H_
#define ESPINA_ROI_WIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include <Core/Utils/Spatial.h>
#include <Core/Utils/vtkVoxelContour2D.h>
#include <Core/Utils/Vector3.hxx>
#include <GUI/View/RenderView.h>
#include <GUI/View/Widgets/EspinaWidget.h>

// VTK
#include <vtkActor.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>

// Qt
#include <QObject>

namespace ESPINA
{
  class ViewManager;
  class View2D;
  class RenderView;

  class EspinaGUI_EXPORT ROIWidget
  : public QObject
  , public EspinaWidget
  {
    Q_OBJECT
    public:
      /** \brief ROIWidget class constructor.
       * \param[in] roi Region of interest object smart pointer.
       *
       */
      explicit ROIWidget(ROISPtr roi);

      /** \brief ROIWidget class virtual destructor.
       *
       */
      virtual ~ROIWidget();

      /** \brief Implements EspinaWidget::registerView().
       *
       */
      virtual void registerView  (RenderView *view);

      /** \brief Implements EspinaWidget::unregisterView().
       *
       */
      virtual void unregisterView(RenderView *view);

      /** \brief Implements EspinaWidget::setEnabled().
       *
       */
      virtual void setEnabled(bool enable);

      void setColor(const QColor &color);

    private slots:
      /** \brief Update the representation when the view changes the slice.
       * \parma[in] plane, orientation plane.
       * \param[in] pos new plane position.
       */
      void sliceChanged(Plane plane, Nm pos);

      /** \brief Updates the representations.
       *
       */
      void updateROIRepresentations();

    private:
      void updateActor(View2D *view);

      struct Pipeline
      {
        vtkSmartPointer<vtkVoxelContour2D> contour;
        vtkSmartPointer<vtkPolyDataMapper> mapper;
        vtkSmartPointer<vtkActor>          actor;

        Pipeline() {}

        ~Pipeline() {}
      };

      QMap<View2D *, Pipeline> m_representations;

      ROISPtr m_ROI;
      QColor  m_color;
  };

} // namespace ESPINA

#endif // ESPINA_ROI_WIDGET_H_
