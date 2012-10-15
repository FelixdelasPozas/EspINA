/*
 * ContourWidget.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Alvarez
 */

#include "ContourWidget.h"
#include <vtkPlaneContourWidget.h>
#include <SliceContourWidget.h>
#include <iostream>

ContourWidget::ContourWidget()
: m_axialSliceContourWidget(NULL)
, m_coronalSliceContourWidget(NULL)
, m_sagittalSliceContourWidget(NULL)
, m_color(Qt::black)
{
}

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

vtkAbstractWidget *ContourWidget::createWidget()
{
  return NULL;
}

void ContourWidget::deleteWidget(vtkAbstractWidget *widget)
{
  Q_ASSERT(false);
}

SliceWidget *ContourWidget::createSliceWidget(PlaneType plane)
{
  vtkPlaneContourWidget *widget = vtkPlaneContourWidget::New();
  widget->setPolygonColor(this->m_color);

  switch(plane)
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

  // dead code
  return NULL;
}

void ContourWidget::setEnabled(bool enable)
{
  if (NULL != m_axialSliceContourWidget)
    m_axialSliceContourWidget->SetEnabled(enable);

  if (NULL != m_coronalSliceContourWidget)
    m_coronalSliceContourWidget->SetEnabled(enable);

  if (NULL != m_sagittalSliceContourWidget)
    m_sagittalSliceContourWidget->SetEnabled(enable);
}

QMap<PlaneType, QMap<Nm, vtkPolyData*> > ContourWidget::GetContours()
{
  QMap<PlaneType, QMap<Nm, vtkPolyData*> > contours;
  contours.insert(AXIAL, this->m_axialSliceContourWidget->GetContours());
  contours.insert(CORONAL, this->m_coronalSliceContourWidget->GetContours());
  contours.insert(SAGITTAL, this->m_sagittalSliceContourWidget->GetContours());

  return contours;
}

void ContourWidget::SetContours(QMap<PlaneType, QMap<Nm, vtkPolyData*> > contours)
{
  this->m_axialSliceContourWidget->SetContours(contours[AXIAL]);
  this->m_coronalSliceContourWidget->SetContours(contours[CORONAL]);
  this->m_sagittalSliceContourWidget->SetContours(contours[SAGITTAL]);
}

unsigned int ContourWidget::GetContoursNumber()
{
  unsigned int result = 0;
  result += this->m_axialSliceContourWidget->GetContoursNumber();
  result += this->m_coronalSliceContourWidget->GetContoursNumber();
  result += this->m_sagittalSliceContourWidget->GetContoursNumber();

  return result;
}

void ContourWidget::setPolygonColor(QColor color)
{
  m_color = color;
}

QColor ContourWidget::getPolygonColor()
{
  return m_color;
}
