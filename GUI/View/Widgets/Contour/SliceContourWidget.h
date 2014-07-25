/*
 * SliceContourWidget.h
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef SLICECONTOURWIDGET_H_
#define SLICECONTOURWIDGET_H_


// ESPINA
#include "vtkPlaneContourWidget.h"
#include "ContourWidget.h"

#include <Core/EspinaTypes.h>
#include <App/Tools/Brushes/Brush.h>

class vtkPolyData;

namespace ESPINA
{
class EspinaGUI_EXPORT SliceContourWidget
: public SliceWidget
{
  public:
    explicit SliceContourWidget(vtkPlaneContourWidget *widget);
    virtual ~SliceContourWidget();

    virtual void setSlice(Nm pos, PlaneType plane);
    virtual void SetEnabled(int);
    QPair<Brush::BrushMode, vtkPolyData *> getContour();

    void setMode(Brush::BrushMode);

    void Initialize();
    void Initialize(ContourWidget::ContourData contour);
  private:

    bool                   m_initialized;
    PlaneType              m_plane;
    Nm                     m_pos;
    vtkPlaneContourWidget *m_contourWidget;

    vtkPolyData     *m_storedContour;
    Nm               m_storedContourPosition;
    Brush::BrushMode m_storedContourMode;
};

}// namespace ESPINA

#endif /* SLICECONTOURWIDGET_H_ */
