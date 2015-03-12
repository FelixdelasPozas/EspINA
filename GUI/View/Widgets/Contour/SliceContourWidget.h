/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SLICE_CONTOUR_WIDGET_H_
#define ESPINA_SLICE_CONTOUR_WIDGET_H_


// ESPINA
#include <Core/EspinaTypes.h>
#include <GUI/EventHandlers/MaskPainter.h>
#include <GUI/View/Widgets/EspinaWidget.h>
#include <GUI/View/Widgets/Contour/ContourWidget.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourWidget.h>

class vtkPolyData;

namespace ESPINA
{
class EspinaGUI_EXPORT SliceContourWidget
: public EspinaWidget
{
  public:
    /** \brief SliceContourWidget class constructor.
     * \param[in] widget vtkPlaneContourWidget raw pointer to manage.
     *
     */
    explicit SliceContourWidget(vtkPlaneContourWidget *widget);

    /** \brief SliceContourWidget class virtual destructor.
     *
     */
    virtual ~SliceContourWidget();

    /** \brief Sets the slice of the widget.
     * \param[in] pos position in Nm
     * \param[in] plane orientation plane.
     */
    virtual void setSlice(Nm pos, Plane plane);

    /** \brief Enables/disables the widget.
     * \param[in] enable bool casted to integer value.
     */
    virtual void SetEnabled(int enable);

    /** \brief Returns the contour data of the widget.
     *
     */
    QPair<DrawingMode, vtkPolyData *> contour();

    /** \brief Sets the operation mode of the widget.
     * \param[in] mode Brush drawing mode.
     */
    void setDrawingMode(DrawingMode mode);

    /** \brief Deletes any contour in the widget and resets it's state.
     *
     */
    void Initialize();

    /** \brief Deletes any contour in the widget and sets the new contour as the data.
     * \param[in] contour ContourData object.
     *
     */
    void Initialize(ContourWidget::ContourData contour);
  private:
    bool                   m_initialized;
    Plane                  m_plane;
    Nm                     m_pos;
    vtkPlaneContourWidget *m_contourWidget;

    vtkPolyData *m_storedContour;
    Nm           m_storedContourPosition;
    DrawingMode  m_storedContourMode;
};

}// namespace ESPINA

#endif // ESPINA_SLICE_CONTOUR_WIDGET_H_
