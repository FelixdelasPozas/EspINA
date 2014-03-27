/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_RULER_WIDGET_H_
#define ESPINA_RULER_WIDGET_H_

// EspINA
#include <Core/Utils/Bounds.h>
#include <GUI/View/Widgets/EspinaWidget.h>

// EspINA

// VTK
class vtkCubeAxesActor2D;

namespace EspINA
{
  class vtkRulerWidget;
  class vtkRulerWidget3D;
  class RulerSliceWidget;

  class EspinaGUI_EXPORT RulerWidget
  : public EspinaWidget
  {
    public:
      /* \brief RulerWidget class constructor.
       *
       */
      explicit RulerWidget();

      /* \brief RulerWidget class destructor.
       *
       */
      virtual ~RulerWidget();

      /* \brief Implements EspinaWidget::create3DWidget.
       *
       */
      virtual vtkAbstractWidget *create3DWidget(View3D *view);

      /* \brief Implements EspinaWidget::createSliceWidget.
       *
       */
      virtual SliceWidget *createSliceWidget(View2D *view);

      /* \brief Implements EspinaWidget::processEvents.
       *
       */
      virtual bool processEvent(vtkRenderWindowInteractor *iren,
                                long unsigned int event);

      /* \brief Implements EspinaWidget::setEnabled.
       *
       */
      virtual void setEnabled(bool enable);

      /* \brief Sets widgets' bounds.
       * \param[in] bounds Bounds of the selection.
       *
       */
      void setBounds(Bounds bounds);

    private:
      vtkRulerWidget *m_axial;
      vtkRulerWidget *m_coronal;
      vtkRulerWidget *m_sagittal;
      vtkRulerWidget3D *m_volume;
      QList<RulerSliceWidget*> m_rulerSliceWidgets;
  };

} // namespace EspINA
#endif // ESPINA_RULER_WIDGET_H_
