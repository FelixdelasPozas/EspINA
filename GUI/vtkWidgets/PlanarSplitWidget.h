/*
 * PlanarSplitWidget.h
 *
 *  Created on: Nov 5, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef PLANARSPLITWIDGET_H_
#define PLANARSPLITWIDGET_H_


// EspINA
#include "GUI/vtkWidgets/EspinaWidget.h"
#include <Core/EspinaTypes.h>
#include <Core/EspinaVolume.h>

// vtk
#include <vtkSmartPointer.h>
#include <vtkCommand.h>

// Qt
#include <QList>

class vtkAbstractWidget;
class vtkPoints;
class vtkPlane;
class vtkAbstractWidget;
class vtkImageStencilSource;
class vtkImplicitPlaneWidget2;
class vtkAlgorithmOutput;
class vtkImageStencilData;

namespace EspINA
{
  class SliceWidget;
  class PlanarSplitSliceWidget;

  enum WidgetType { AXIAL_WIDGET = 2, CORONAL_WIDGET = 1, SAGITTAL_WIDGET = 0, VOLUME_WIDGET = 3, NONE = 4 };

  class PlanarSplitWidget
  : public EspinaWidget
  , public vtkCommand
  {
  public:
    explicit PlanarSplitWidget();
    virtual ~PlanarSplitWidget();

    // EspinaWidget implementation
    virtual vtkAbstractWidget *createWidget();
    virtual void deleteWidget(vtkAbstractWidget *widget);
    virtual SliceWidget *createSliceWidget(PlaneType plane);

    virtual bool processEvent(vtkRenderWindowInteractor *iren,
                              long unsigned int event);
    virtual void setEnabled(bool enable);

    // get/set
    virtual void setPlanePoints(vtkSmartPointer<vtkPoints>);
    virtual vtkSmartPointer<vtkPoints> getPlanePoints();
    virtual vtkSmartPointer<vtkPlane> getImplicitPlane();
    virtual vtkSmartPointer<vtkImageStencilData> getStencilForVolume(EspinaVolume::Pointer volume);
    virtual void setSegmentationBounds(double *);
    virtual bool planeIsValid();

    virtual WidgetType getMainWidget();

    // vtkCommand
    virtual void Execute (vtkObject *caller, unsigned long eventId, void *callData);

    virtual bool manipulatesSegmentations() { return true; };

  private:
    PlanarSplitSliceWidget *m_axial;
    PlanarSplitSliceWidget *m_coronal;
    PlanarSplitSliceWidget *m_sagittal;
    vtkImplicitPlaneWidget2 *m_volume;
    QList<vtkAbstractWidget*> m_widgets;
    WidgetType m_mainWidget;
    vtkAlgorithmOutput *m_vtkVolumeInformation;
  };

}// namespace EspINA

#endif /* PLANARSPLITWIDGET_H_ */
