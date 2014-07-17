/*
 
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

// EspINA
#include <Core/Analysis/Data/Volumetric/ROI.h>
#include <Core/Utils/NmVector3.h>
#include <Core/Utils/Spatial.h>
#include <GUI/View/RenderView.h>
#include <GUI/View/Widgets/Contour/vtkVoxelContour2D.h>
#include <GUI/View/Widgets/EspinaWidget.h>

// VTK
#include <vtkActor.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkPolyDataMapper.h>
#include <vtkSmartPointer.h>

// Qt
#include <QObject>

namespace EspINA
{
  class ViewManager;
  class View2D;
  class RenderView;
  
  class ROIWidget
  : public QObject
  , public EspinaWidget
  {
    Q_OBJECT
    public:
      /* \brief ROIWidget class constructor.
       * \param[in] roi Region of interest object smart pointer.
       *
       */
      explicit ROIWidget(ROISPtr roi);

      /* \brief ROIWidget class virtual destructor.
       *
       */
      virtual ~ROIWidget();

      /* \brief Implements EspinaWidget::registerView(RenderView *view)
       *
       */
      virtual void registerView  (RenderView *view);

      /* \brief Implements EspinaWidget::unregisterView(RenderView *view)
       *
       */
      virtual void unregisterView(RenderView *view);

      /* \brief Implements EspinaWidget::setEnabled(bool)
       *
       */
      virtual void setEnabled(bool enable);

    private slots:
      void sliceChanged(Plane, Nm);
      void updateROIRepresentations();

    private:
      void updateActor(View2D *view);

      struct pipeline
      {
        vtkSmartPointer<vtkVoxelContour2D> contour;
        vtkSmartPointer<vtkPolyDataMapper> mapper;
        vtkSmartPointer<vtkActor>          actor;

        pipeline(): contour{nullptr}, mapper{nullptr}, actor{nullptr} {};
        ~pipeline() { contour = nullptr; mapper = nullptr; actor = nullptr; }
      };

      QMap<View2D *, struct pipeline> m_representations;

      ROISPtr                     m_ROI;
  };

} // namespace EspINA

#endif // ESPINA_ROI_WIDGET_H_
