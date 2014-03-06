/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef ESPINA_RULER_SLICE_WIDGET_H_
#define ESPINA_RULER_SLICE_WIDGET_H_

// EspINA
#include <Core/Utils/Bounds.h>
#include <Core/Utils/Spatial.h>
#include <GUI/View/Widgets/EspinaWidget.h>

namespace EspINA
{
  class EspinaGUI_EXPORT RulerSliceWidget
  : public SliceWidget
  {
    public:
      /* \brief RulerSliceWidget class constructor.
       * \param[in] widget Encapsulated vtkAbstractWidget.
       *
       */
      explicit RulerSliceWidget(vtkAbstractWidget *widget);

      /* \brief RulerSliceWidget class destructor.
       *
       */
      virtual ~RulerSliceWidget();

      /* \brief Enables/Disables this widget and the one it encapsulates.
       * \param[in] value int value to pass to vtkAbstractWidget.
       */
      virtual void setEnabled(int value);

      /* \brief Sets encapsulated widget bounds to measure.
       * \param[in] bounds Bounds of the area to measure.
       */
      virtual void setBounds(Bounds bounds);

      /* \brief Implements SliceWidget::setSlice.
       *
       */
      virtual void setSlice(Nm pos, Plane plane);

    private:
      Nm    m_pos;
      Plane m_plane;

      bool m_insideBounds;
      bool m_enabled;
  };

} // namespace EspINA
#endif // ESPINA_RULER_SLICE_WIDGET_H_
