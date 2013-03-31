/*
 * ContourWidget.h
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Alvarez
 */

#ifndef CONTOURWIDGET_H_
#define CONTOURWIDGET_H_

#include "GUI/vtkWidgets/EspinaWidget.h"
#include <Core/EspinaTypes.h>
#include <App/Tools/Brushes/Brush.h>

#include <QMap>
#include <QList>
#include <QObject>
#include <QColor>

class vtkPolyData;

namespace EspINA
{
  class SliceContourWidget;

  class ContourWidget
  : public QObject
  , public EspinaWidget
  {
  Q_OBJECT
  public:
    struct ContourInternals
    {
      PlaneType        contourPlane;
      Brush::BrushMode contourMode;
      vtkPolyData     *contourPoints;

      ContourInternals(PlaneType plane, Brush::BrushMode mode, vtkPolyData *contour) : contourPlane(plane), contourMode(mode), contourPoints(contour) {};
    };
    typedef struct ContourInternals ContourData;

    typedef QList<ContourData> ContourList;

    explicit ContourWidget();
    virtual ~ContourWidget();

    virtual vtkAbstractWidget *create3DWidget(VolumeView *view);

    virtual SliceWidget *createSliceWidget(SliceView *view);

    virtual bool processEvent(vtkRenderWindowInteractor* iren,
                              long unsigned int event);
    virtual void setEnabled(bool enable);

    virtual void setPolygonColor(QColor);
    virtual QColor getPolygonColor();
    ContourList getContours();

    void startContourFromWidget();
    void endContourFromWidget();

    void setMode(Brush::BrushMode);

  signals:
    void rasterizeContours(ContourWidget::ContourList);
    void storeData();

  private:
    SliceContourWidget *m_axialSliceContourWidget;
    SliceContourWidget *m_coronalSliceContourWidget;
    SliceContourWidget *m_sagittalSliceContourWidget;
    QList<vtkAbstractWidget *> m_widgets;
    QColor m_color;
  };

} // namespace EspINA;

#endif /* CONTOURWIDGET_H_ */
