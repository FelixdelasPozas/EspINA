/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This program is free software: you can redistribute it and/or modify
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
#include <Core/Utils/NmVector3.h>
#include <Core/Utils/Spatial.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <vtkDiscreteMarchingCubes.h>
#include <vtkSmartPointer.h>

// Qt
#include <QObject>

namespace EspINA
{
  
  class ROIWidget
  : public QObject
  , public EspinaWidget
  {
    Q_OBJECT
    public:
      /* \brief ROIWidget class constructor.
       * \param[in] vm ViewManagerSPtr
       *
       */
      explicit ROIWidget(ViewManagerSPtr);

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
      void ROIChanged();

    private:
      QMap<RenderView *, vtkSmartPointer<vtkActor>>                 m_views;
      QMap<RenderView *, vtkSmartPointer<vtkDiscreteMarchingCubes>> m_marchingCubes;
      QMap<RenderView *, vtkSmartPointer<vtkVoxelContour2D>>        m_contours;

      ViewManagerSPtr                m_vm;
      vtkSmartPointer<vtkTexture>    m_texture;
  };

} // namespace EspINA

#endif // ESPINA_ROI_WIDGET_H_
