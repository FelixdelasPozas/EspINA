/*
 * PlanarSplitWidget.h
 *
 *  Created on: Nov 5, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef PLANARSPLITWIDGET_H_
#define PLANARSPLITWIDGET_H_

#include "EspinaGUI_Export.h"

// EspINA
#include <GUI/View/Widgets/EspinaWidget.h>

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

  class EspinaGUI_EXPORT PlanarSplitWidget
  : public EspinaWidget
  , public vtkCommand
  {
  public:
    virtual ~PlanarSplitWidget();

    vtkTypeMacro(PlanarSplitWidget,vtkCommand);

    static PlanarSplitWidget *New()
    {return new PlanarSplitWidget();};

    // EspinaWidget implementation
    virtual vtkAbstractWidget *create3DWidget(View3D *view);

    virtual SliceWidget *createSliceWidget(View2D *view);

    virtual bool processEvent(vtkRenderWindowInteractor *iren,
                              long unsigned int event);
    virtual void setEnabled(bool enable);

    // get/set
    virtual void setPlanePoints(vtkSmartPointer<vtkPoints>);
    virtual vtkSmartPointer<vtkPoints> getPlanePoints();
    virtual vtkSmartPointer<vtkPlane> getImplicitPlane(const double spacing[3]);
    //TODO: virtual vtkSmartPointer<vtkImageStencilData> getStencilForVolume(SegmentationVolumeSPtr volume);
    virtual void setSegmentationBounds(double *);
    virtual bool planeIsValid();

    virtual WidgetType getMainWidget();

    // vtkCommand
    virtual void Execute (vtkObject *caller, unsigned long eventId, void *callData);

    virtual bool manipulatesSegmentations() { return true; };

  private:
    explicit PlanarSplitWidget();

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
