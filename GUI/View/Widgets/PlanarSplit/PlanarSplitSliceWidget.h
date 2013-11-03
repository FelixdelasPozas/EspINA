/*
 * PlanarSplitSliceWidget.h
 *
 *  Created on: Nov 5, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef ESPINA_PLANAR_SPLIT_WIDGET_H
#define ESPINA_PLANAR_SPLIT_WIDGET_H

// EspINA
#include <GUI/View/Widgets/EspinaWidget.h>

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

    virtual void setSlice(Nm pos, Plane plane);

    virtual void setOrientation(Plane plane);

    virtual void setPoints(vtkSmartPointer<vtkPoints>);
    vtkSmartPointer<vtkPoints> getPoints();

    virtual void setEnabled(bool);

    virtual vtkAbstractWidget* getWidget() { return this->m_widget; };
    virtual void disableWidget();

  private:
    Plane m_plane;
    bool  m_mainWidget;
  };

}// namespace EspINA

#endif // ESPINA_PLANAR_SPLIT_WIDGET_H
