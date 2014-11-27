/*
 * ContourWidget.h
 *
 *  Created on: Sep 8, 2012
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef CONTOURWIDGET_H_
#define CONTOURWIDGET_H_

#include <GUI/View/Widgets/EspinaWidget.h>

#include <App/Tools/Brushes/Brush.h>

#include <QMap>
#include <QList>
#include <QObject>
#include <QColor>

class vtkPolyData;

namespace ESPINA
{
  class SliceContourWidget;

  class EspinaGUI_EXPORT ContourWidget
  : public QObject
  , public EspinaWidget
  {
  Q_OBJECT
  public:
    struct ContourInternals
    {
      PlaneType        Plane;
      Brush::BrushMode Mode;
      vtkPolyData     *PolyData;

      ContourInternals(PlaneType plane, Brush::BrushMode mode, vtkPolyData *contour) : Plane(plane), Mode(mode), PolyData(contour) {};
      ContourInternals() : Plane(AXIAL), Mode(Brush::BRUSH), PolyData(NULL) {};
    };
    typedef struct ContourInternals ContourData;

    typedef QList<ContourData> ContourList;

    explicit ContourWidget();
    virtual ~ContourWidget();

    virtual vtkAbstractWidget *create3DWidget(View3D *view);

    virtual SliceWidget *createSliceWidget(View2D *view);

    virtual bool processEvent(vtkRenderWindowInteractor* iren,
                              long unsigned int event);
    virtual void setEnabled(bool enable);

    virtual void setPolygonColor(QColor);
    virtual QColor getPolygonColor();
    ContourList getContours();

    void startContourFromWidget();
    void endContourFromWidget();

    void setMode(Brush::BrushMode);

    // reset all contours in all planes without rasterize
    void initialize();
    void initialize(ContourData contour);

  signals:
    void rasterizeContours(ContourWidget::ContourList);
    void endContour();

  private:
    SliceContourWidget *m_axialSliceContourWidget;
    SliceContourWidget *m_coronalSliceContourWidget;
    SliceContourWidget *m_sagittalSliceContourWidget;
    QList<vtkAbstractWidget *> m_widgets;
    QColor m_color;
  };

} // namespace ESPINA;

#endif /* CONTOURWIDGET_H_ */
