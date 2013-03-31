/*
 * ContourWidget.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Alvarez
 */

// EspINA
#include "ContourWidget.h"
#include "GUI/vtkWidgets/EspinaInteractorAdapter.h"
#include "GUI/vtkWidgets/SliceContourWidget.h"
#include "GUI/vtkWidgets/vtkPlaneContourWidget.h"
#include <GUI/QtWidget/SliceView.h>

// C++
#include <iostream>

using namespace EspINA;

typedef EspinaInteractorAdapter<vtkPlaneContourWidget> ContourWidgetAdapter;

//----------------------------------------------------------------------------
ContourWidget::ContourWidget()
: m_axialSliceContourWidget(NULL)
, m_coronalSliceContourWidget(NULL)
, m_sagittalSliceContourWidget(NULL)
, m_color(Qt::black)
{
}

//----------------------------------------------------------------------------
ContourWidget::~ContourWidget()
{
  if (NULL != m_axialSliceContourWidget)
  {
    m_axialSliceContourWidget->SetEnabled(false);
    delete m_axialSliceContourWidget;
  }

  if (NULL != m_coronalSliceContourWidget)
  {
    m_coronalSliceContourWidget->SetEnabled(false);
    delete m_coronalSliceContourWidget;
  }

  if (NULL != m_sagittalSliceContourWidget)
  {
    m_sagittalSliceContourWidget->SetEnabled(false);
    delete m_sagittalSliceContourWidget;
  }
}

//----------------------------------------------------------------------------
vtkAbstractWidget *ContourWidget::create3DWidget(VolumeView *view)
{
  return NULL;
}

//----------------------------------------------------------------------------
SliceWidget *ContourWidget::createSliceWidget(SliceView *view)
{
  ContourWidgetAdapter *widget = new ContourWidgetAdapter();
  widget->setContourWidget(this);
  widget->setPolygonColor(this->m_color);
  m_widgets << widget;

  switch(view->plane())
  {
    case AXIAL:
      Q_ASSERT(NULL == m_axialSliceContourWidget);
      m_axialSliceContourWidget = new SliceContourWidget(widget);
      return m_axialSliceContourWidget;
      break;
    case CORONAL:
      Q_ASSERT(NULL == m_coronalSliceContourWidget);
      m_coronalSliceContourWidget = new SliceContourWidget(widget);
      return m_coronalSliceContourWidget;
      break;
    case SAGITTAL:
      Q_ASSERT(NULL == m_sagittalSliceContourWidget);
      m_sagittalSliceContourWidget = new SliceContourWidget(widget);
      return m_sagittalSliceContourWidget;
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  Q_ASSERT(false);
  return NULL;
}

//----------------------------------------------------------------------------
bool ContourWidget::processEvent(vtkRenderWindowInteractor* iren,
                                 long unsigned int event)
{
  foreach(vtkAbstractWidget *widget, m_widgets)
  {
    if (widget->GetInteractor() == iren)
    {
      ContourWidgetAdapter *sw = dynamic_cast<ContourWidgetAdapter *>(widget);
      return sw->ProcessEventsHandler(event);
    }
  }

  return false;
}

//----------------------------------------------------------------------------
void ContourWidget::setEnabled(bool enable)
{
  if (NULL != m_axialSliceContourWidget)
    m_axialSliceContourWidget->SetEnabled(enable);

  if (NULL != m_coronalSliceContourWidget)
    m_coronalSliceContourWidget->SetEnabled(enable);

  if (NULL != m_sagittalSliceContourWidget)
    m_sagittalSliceContourWidget->SetEnabled(enable);
}

//----------------------------------------------------------------------------
void ContourWidget::setPolygonColor(QColor color)
{
  m_color = color;
}

//----------------------------------------------------------------------------
QColor ContourWidget::getPolygonColor()
{
  return m_color;
}

//----------------------------------------------------------------------------
ContourWidget::ContourList ContourWidget::getContours()
{
  ContourList resultList;

  QPair<Brush::BrushMode, vtkPolyData*> axialContour = m_axialSliceContourWidget->getContour();
  QPair<Brush::BrushMode, vtkPolyData*> coronalContour = m_coronalSliceContourWidget->getContour();
  QPair<Brush::BrushMode, vtkPolyData*> sagittalContour = m_sagittalSliceContourWidget->getContour();

  if (axialContour.second || coronalContour.second || sagittalContour.second)
  {
    resultList << ContourData(AXIAL, axialContour.first, axialContour.second);
    resultList << ContourData(CORONAL, coronalContour.first, coronalContour.second);
    resultList << ContourData(SAGITTAL, sagittalContour.first, sagittalContour.second);
  }

  return resultList;
}

//----------------------------------------------------------------------------
void ContourWidget::startContourFromWidget()
{
  ContourList resultList = getContours();

  if (!resultList.empty())
  {
    emit rasterizeContours(resultList);

    if (resultList[0].contourPoints != NULL)
    {
      resultList[0].contourPoints->Delete();
      m_axialSliceContourWidget->Initialize();
    }

    if (resultList[1].contourPoints != NULL)
    {
      resultList[1].contourPoints->Delete();
      m_coronalSliceContourWidget->Initialize();
    }

    if (resultList[2].contourPoints != NULL)
    {
      resultList[2].contourPoints->Delete();
      m_sagittalSliceContourWidget->Initialize();
    }
  }
}

//----------------------------------------------------------------------------
void ContourWidget::endContourFromWidget()
{
  emit storeData();
}

//----------------------------------------------------------------------------
void ContourWidget::setMode(Brush::BrushMode mode)
{
  if (m_axialSliceContourWidget)
    m_axialSliceContourWidget->setMode(mode);

  if (m_coronalSliceContourWidget)
    m_coronalSliceContourWidget->setMode(mode);

  if (m_sagittalSliceContourWidget)
    m_sagittalSliceContourWidget->setMode(mode);
}
