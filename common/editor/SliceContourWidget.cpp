/*
 * SliceContourWidget.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Álvarez
 */

#include "SliceContourWidget.h"
#include "vtkPlaneContourRepresentationGlyph.h"
#include <vtkObjectFactory.h>
#include <QPolygon>

SliceContourWidget::SliceContourWidget(vtkPlaneContourWidget* widget)
: SliceWidget(widget)
, m_initialized(false)
, m_plane(AXIAL)
, m_pos(0)
, m_contourWidget(widget)
{
}

SliceContourWidget::~SliceContourWidget()
{
  QMap<Nm, vtkPolyData*>::iterator it = m_contourMap.begin();
  while (it != m_contourMap.end())
  {
    vtkPolyData *value = it.value();
    if (NULL != value)
      value->Delete();
    it++;
  }
  m_contourMap.clear();
  m_contourWidget->Delete();
}

void SliceContourWidget::setSlice(Nm pos, PlaneType plane)
{
  if (!m_initialized)
  {
    m_initialized = true;
    m_plane = plane;
    m_pos = pos;
    m_contourWidget->SetOrientation(plane);
  }
  else
  {
    vtkPlaneContourRepresentationGlyph *rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(this->m_widget->GetRepresentation());
    if ((rep->GetContourRepresentationAsPolyData()->GetPoints()->GetNumberOfPoints() != 0) && rep->GetClosedLoop())
    {
      vtkPolyData *contour = vtkPolyData::New();
      contour->DeepCopy(rep->GetContourRepresentationAsPolyData());
      m_contourMap.insert(m_pos, contour);
    }

    if (m_contourMap.contains(pos) && (m_contourMap.value(pos)->GetPoints()->GetNumberOfPoints() != 0))
      m_contourWidget->Initialize(m_contourMap.value(pos));
    else
      m_contourWidget->Initialize();

    m_pos = pos;
  }
}

void SliceContourWidget::setContours(QMap<Nm, vtkPolyData*> contours)
{
  if (!m_initialized)
    Q_ASSERT(false);

  QMap<Nm, vtkPolyData*>::iterator it = m_contourMap.begin();
  while (it != m_contourMap.end())
  {
    vtkPolyData *value = it.value();
    if (NULL != value)
      value->Delete();
    it++;
  }
  m_contourMap.clear();

  m_contourMap = contours;

  if (m_contourMap.contains(m_pos) && (m_contourMap.value(m_pos)->GetPoints()->GetNumberOfPoints() != 0))
    m_contourWidget->Initialize(m_contourMap.value(m_pos));
  else
    m_contourWidget->Initialize();
}

QMap<Nm, vtkPolyData*> SliceContourWidget::getContours()
{
  if (!m_initialized)
    Q_ASSERT(false);

  return m_contourMap;
}

