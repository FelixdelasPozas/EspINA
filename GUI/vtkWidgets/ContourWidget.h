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
  public:
    explicit ContourWidget();
    virtual ~ContourWidget();

    virtual vtkAbstractWidget *create3DWidget(VolumeView *view);

    virtual SliceWidget *createSliceWidget(SliceView *view);

    virtual bool processEvent(vtkRenderWindowInteractor* iren,
                              long unsigned int event);
    virtual void setEnabled(bool enable);

    virtual QMap<PlaneType, QMap<Nm, vtkPolyData*> > GetContours();
    virtual void SetContours(QMap<PlaneType, QMap<Nm, vtkPolyData*> >);
    virtual unsigned int GetContoursNumber();
    virtual void setPolygonColor(QColor);
    virtual QColor getPolygonColor();

  private:
    SliceContourWidget *m_axialSliceContourWidget;
    SliceContourWidget *m_coronalSliceContourWidget;
    SliceContourWidget *m_sagittalSliceContourWidget;
    QList<vtkAbstractWidget *> m_widgets;
    QColor m_color;
  };

} // namespace EspINA;

#endif /* CONTOURWIDGET_H_ */
