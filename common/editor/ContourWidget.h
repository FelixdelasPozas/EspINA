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

class vtkPlaneContourWidget;

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
    virtual void setEnabled(bool enable);

  private:
    vtkPlaneContourWidget *m_axialPlaneWidget;
    vtkPlaneContourWidget *m_coronalPlaneWidget;
    vtkPlaneContourWidget *m_sagittalPlaneWidget;
};

#endif /* CONTOURWIDGET_H_ */
