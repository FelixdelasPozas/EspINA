/*
 * ContourWidget.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Alvarez
 */

#include "ContourWidget.h"

// EspINA
#include "GUI/View/SliceView.h"
#include "GUI/View/Widgets/Contour/SliceContourWidget.h"
#include "GUI/View/Widgets/Contour/vtkPlaneContourWidget.h"
#include "GUI/View/Widgets/EspinaInteractorAdapter.h"

// C++
#include <iostream>

using namespace EspINA;

typedef EspinaInteractorAdapter<vtkPlaneContourWidget> ContourWidgetAdapter;
using ContourWidgetAdapter = EspinaInteractorAdapter<vtkPlaneContourWidget>

//----------------------------------------------------------------------------
ContourWidget::ContourWidget()
: m_axialSliceContourWidget(nullptr)
, m_coronalSliceContourWidget(nullptr)
, m_sagittalSliceContourWidget(nullptr)
, m_color(Qt::black)
{
}

//----------------------------------------------------------------------------
ContourWidget::~ContourWidget()
{
  if (nullptr != m_axialSliceContourWidget)
  {
    m_axialSliceContourWidget->SetEnabled(false);
    delete m_axialSliceContourWidget;
  }

  if (nullptr != m_coronalSliceContourWidget)
  {
    m_coronalSliceContourWidget->SetEnabled(false);
    delete m_coronalSliceContourWidget;
  }

  if (nullptr != m_sagittalSliceContourWidget)
  {
    m_sagittalSliceContourWidget->SetEnabled(false);
    delete m_sagittalSliceContourWidget;
  }
}

//----------------------------------------------------------------------------
vtkAbstractWidget *ContourWidget::create3DWidget(View3D *view)
{
  return nullptr;
}

//----------------------------------------------------------------------------
SliceWidget *ContourWidget::createSliceWidget(View2D *view)
{
  ContourWidgetAdapter *widget = new ContourWidgetAdapter();
  widget->setContourWidget(this);
  widget->setPolygonColor(this->m_color);
  m_widgets << widget;

  switch(view->plane())
  {
    case AXIAL:
      Q_ASSERT(nullptr == m_axialSliceContourWidget);
      m_axialSliceContourWidget = new SliceContourWidget(widget);
      return m_axialSliceContourWidget;
      break;
    case CORONAL:
      Q_ASSERT(nullptr == m_coronalSliceContourWidget);
      m_coronalSliceContourWidget = new SliceContourWidget(widget);
      return m_coronalSliceContourWidget;
      break;
    case SAGITTAL:
      Q_ASSERT(nullptr == m_sagittalSliceContourWidget);
      m_sagittalSliceContourWidget = new SliceContourWidget(widget);
      return m_sagittalSliceContourWidget;
      break;
    default:
      Q_ASSERT(false);
      break;
  }

  Q_ASSERT(false);
  return nullptr;
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
  if (nullptr != m_axialSliceContourWidget)
    m_axialSliceContourWidget->SetEnabled(enable);

  if (nullptr != m_coronalSliceContourWidget)
    m_coronalSliceContourWidget->SetEnabled(enable);

  if (nullptr != m_sagittalSliceContourWidget)
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

    if (resultList[0].PolyData != nullptr)
    {
      resultList[0].PolyData->Delete();
      m_axialSliceContourWidget->Initialize();
    }

    if (resultList[1].PolyData != nullptr)
    {
      resultList[1].PolyData->Delete();
      m_coronalSliceContourWidget->Initialize();
    }

    if (resultList[2].PolyData != nullptr)
    {
      resultList[2].PolyData->Delete();
      m_sagittalSliceContourWidget->Initialize();
    }
  }
}

//----------------------------------------------------------------------------
void ContourWidget::endContourFromWidget()
{
  emit endContour();
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

//----------------------------------------------------------------------------
void ContourWidget::initialize()
{
  if (m_axialSliceContourWidget)
    m_axialSliceContourWidget->Initialize();

  if (m_coronalSliceContourWidget)
    m_coronalSliceContourWidget->Initialize();

  if (m_sagittalSliceContourWidget)
    m_sagittalSliceContourWidget->Initialize();
}

//----------------------------------------------------------------------------
void ContourWidget::initialize(ContourData contour)
{
  if (contour.PolyData == nullptr)
    initialize();

  switch (contour.Plane)
  {
    case AXIAL:
      if (m_axialSliceContourWidget)
        m_axialSliceContourWidget->Initialize(contour);

      if (m_coronalSliceContourWidget)
        m_coronalSliceContourWidget->Initialize();

      if (m_sagittalSliceContourWidget)
        m_sagittalSliceContourWidget->Initialize();
      break;
    case CORONAL:
      if (m_axialSliceContourWidget)
        m_axialSliceContourWidget->Initialize();

      if (m_coronalSliceContourWidget)
        m_coronalSliceContourWidget->Initialize(contour);

      if (m_sagittalSliceContourWidget)
        m_sagittalSliceContourWidget->Initialize();
      break;
    case SAGITTAL:
      if (m_axialSliceContourWidget)
        m_axialSliceContourWidget->Initialize();

      if (m_coronalSliceContourWidget)
        m_coronalSliceContourWidget->Initialize();

      if (m_sagittalSliceContourWidget)
        m_sagittalSliceContourWidget->Initialize(contour);
      break;
    default:
      Q_ASSERT(false);
      break;
  }
}
