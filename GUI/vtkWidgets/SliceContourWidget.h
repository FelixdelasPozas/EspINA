/*
 * SliceContourWidget.h
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef SLICECONTOURWIDGET_H_
#define SLICECONTOURWIDGET_H_

// EspINA
#include "GUI/vtkWidgets/vtkPlaneContourWidget.h"
#include "GUI/vtkWidgets/EspinaWidget.h"
#include <Core/EspinaTypes.h>
#include <App/Tools/Brushes/Brush.h>

// Qt
#include <QMap>

// VTK
#include <vtkPolyData.h>

namespace EspINA
{
class SliceContourWidget: public SliceWidget
{
  public:
    explicit SliceContourWidget(vtkPlaneContourWidget *widget);
    virtual ~SliceContourWidget();

    virtual void setSlice(Nm pos, PlaneType plane);
    virtual void SetEnabled(int);
    QPair<Brush::BrushMode, vtkPolyData *> getContour();

    void setMode(Brush::BrushMode);

    void Initialize();
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
