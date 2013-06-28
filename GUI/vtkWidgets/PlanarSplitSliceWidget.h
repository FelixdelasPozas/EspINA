/*
 * PlanarSplitSliceWidget.h
 *
 *  Created on: Nov 5, 2012
 *      Author: F�lix de las Pozas �lvarez
 */

#ifndef PLANARSPLITSLICEWIDGET_H_
#define PLANARSPLITSLICEWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include "GUI/vtkWidgets//EspinaWidget.h"
#include <Core/EspinaTypes.h>

// vtk
#include <vtkSmartPointer.h>

// forward declarations
class vtkPoints;

namespace EspINA
{
  class vtkPlanarSplitWidget;

  class EspinaGUI_EXPORT PlanarSplitSliceWidget
  : public SliceWidget
  {
  public:
    explicit PlanarSplitSliceWidget(vtkAbstractWidget *widget);
    virtual ~PlanarSplitSliceWidget();

    virtual void setSlice(Nm pos, PlaneType plane);

    virtual void setOrientation(PlaneType);

    virtual void setPoints(vtkSmartPointer<vtkPoints>);
    vtkSmartPointer<vtkPoints> getPoints();

    virtual void setEnabled(bool);

    virtual vtkAbstractWidget* getWidget() { return this->m_widget; };
    virtual void disableWidget();

  private:
    PlaneType m_plane;
    bool m_mainWidget;
  };

}// namespace EspINA

#endif /* PLANARSPLITSLICEWIDGET_H_ */
