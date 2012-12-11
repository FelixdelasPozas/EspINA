/*
 * ContourWidget.h
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Alvarez
 */

#ifndef CONTOURWIDGET_H_
#define CONTOURWIDGET_H_

#include <common/widgets/EspinaWidget.h>
#include <EspinaTypes.h>

#include <QMap>
#include <QList>
#include <QObject>
#include <QColor>

class SliceContourWidget;
class vtkPolyData;

class ContourWidget
: public QObject
, public EspinaWidget
{
    Q_OBJECT
  public:
    explicit ContourWidget();
    virtual ~ContourWidget();

    virtual vtkAbstractWidget *createWidget();
    virtual void deleteWidget(vtkAbstractWidget *widget);
    virtual SliceWidget *createSliceWidget(PlaneType plane);

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

#endif /* CONTOURWIDGET_H_ */
