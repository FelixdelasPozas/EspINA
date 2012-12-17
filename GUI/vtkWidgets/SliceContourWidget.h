/*
 * SliceContourWidget.h
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef SLICECONTOURWIDGET_H_
#define SLICECONTOURWIDGET_H_

#include "GUI/vtkWidgets/vtkPlaneContourWidget.h"
#include "GUI/vtkWidgets/EspinaWidget.h"
#include <Core/EspinaTypes.h>

#include <QMap>

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

    void SetContours(QMap<Nm, vtkPolyData*> contours);
    QMap<Nm, vtkPolyData*> GetContours();
    unsigned int GetContoursNumber();
  private:
    // helper method
    void AddActualContour();

    bool m_initialized;
    PlaneType m_plane;
    Nm m_pos;
    QMap<Nm, vtkPolyData*> m_contourMap;
    vtkPlaneContourWidget *m_contourWidget;
};

}// namespace EspINA

#endif /* SLICECONTOURWIDGET_H_ */
