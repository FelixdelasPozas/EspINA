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
: m_axialPlaneWidget(NULL)
, m_coronalPlaneWidget(NULL)
, m_sagittalPlaneWidget(NULL)
{
}

ContourWidget::~ContourWidget()
{
  if (NULL != m_axialPlaneWidget)
  {
    m_axialPlaneWidget->EnabledOn();
    m_axialPlaneWidget->Delete();
  }

  if (NULL != m_coronalPlaneWidget)
  {
    m_coronalPlaneWidget->EnabledOn();
    m_coronalPlaneWidget->Delete();
  }

  if (NULL != m_sagittalPlaneWidget)
  {
    m_sagittalPlaneWidget->EnabledOn();
    m_sagittalPlaneWidget->Delete();
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
  Q_ASSERT(widget);

  switch(plane)
  {
    case AXIAL:
      Q_ASSERT(NULL == m_axialPlaneWidget);
      m_axialPlaneWidget = widget;
      break;
    case CORONAL:
      Q_ASSERT(NULL == m_coronalPlaneWidget);
      m_coronalPlaneWidget = widget;
      break;
    case SAGITTAL:
      Q_ASSERT(NULL == m_sagittalPlaneWidget);
      m_sagittalPlaneWidget = widget;
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  return new SliceContourWidget(widget);
}

void ContourWidget::setEnabled(bool enable)
{
  if (NULL != m_axialPlaneWidget)
    m_axialPlaneWidget->SetEnabled(enable);

  if (NULL != m_coronalPlaneWidget)
    m_coronalPlaneWidget->SetEnabled(enable);

  if (NULL != m_sagittalPlaneWidget)
    m_sagittalPlaneWidget->SetEnabled(enable);
}
