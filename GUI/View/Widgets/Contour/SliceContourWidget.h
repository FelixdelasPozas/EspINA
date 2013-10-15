/*
 * SliceContourWidget.h
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef SLICECONTOURWIDGET_H_
#define SLICECONTOURWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "vtkPlaneContourWidget.h"
#include "EspinaWidget.h"
#include "ContourWidget.h"
#include <Core/EspinaTypes.h>
#include <App/Tools/Brushes/Brush.h>

class vtkPolyData;

namespace EspINA
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

}// namespace EspINA

#endif /* SLICECONTOURWIDGET_H_ */
