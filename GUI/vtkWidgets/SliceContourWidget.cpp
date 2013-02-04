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

using namespace EspINA;

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
  // vtkPolyDatas are not deleted here: some filters store it (ContourFilter)
  // some do not, so they are deleted in the destructor of those filters or
  // in FilledContour.cpp right now.
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
    AddActualContour();

    if (m_contourMap.contains(pos) && (m_contourMap.value(pos)->GetPoints()->GetNumberOfPoints() != 0))
      m_contourWidget->Initialize(m_contourMap.value(pos));
    else
      m_contourWidget->Initialize();

    m_pos = pos;
  }
}

void SliceContourWidget::SetContours(QMap<Nm, vtkPolyData*> contours)
{
  if (!m_initialized)
    Q_ASSERT(false);

  QMap<Nm, vtkPolyData*>::iterator it = m_contourMap.begin();
  while (it != m_contourMap.end())
  {
    vtkPolyData *value = it.value();
    if (NULL != value)
      value->Delete();
    ++it;
  }
  m_contourMap.clear();

  m_contourMap = contours;

  if (m_contourMap.contains(m_pos) && (m_contourMap.value(m_pos)->GetPoints()->GetNumberOfPoints() != 0))
    m_contourWidget->Initialize(m_contourMap.value(m_pos));
  else
    m_contourWidget->Initialize();
}

QMap<Nm, vtkPolyData*> SliceContourWidget::GetContours()
{
  if (!m_initialized)
    Q_ASSERT(false);

  // add actual contour (maybe the slice didn't change)
  AddActualContour();

  QMap<Nm, vtkPolyData*>::iterator it = this->m_contourMap.begin();
  while(it != this->m_contourMap.end())
  {
    if (0 == it.value()->GetPoints()->GetNumberOfPoints())
    {
      it.value()->Delete();
      it = m_contourMap.erase(it);
    }
    else
      ++it;
  }

  return m_contourMap;
}

void SliceContourWidget::SetEnabled(int value)
{
  this->m_contourWidget->SetEnabled(value);
}

unsigned int SliceContourWidget::GetContoursNumber()
{
  unsigned int result = 0;

  AddActualContour();

  QMap<Nm, vtkPolyData*>::iterator it = this->m_contourMap.begin();

  while(it != this->m_contourMap.end())
  {
    if (0 != it.value()->GetPoints()->GetNumberOfPoints())
      result++;

    ++it;
  }

  return result;
}

void SliceContourWidget::AddActualContour()
{
  vtkPlaneContourRepresentationGlyph *rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(this->m_widget->GetRepresentation());
  if ((rep->GetContourRepresentationAsPolyData()->GetPoints()->GetNumberOfPoints() != 0) && rep->GetClosedLoop())
  {
    vtkPolyData *contour = vtkPolyData::New();
    contour->DeepCopy(rep->GetContourRepresentationAsPolyData());

    // points in the contour must be corrected according to slice.
    vtkPoints* contourPoints = contour->GetPoints();
    for (int ndx = 0; ndx < contourPoints->GetNumberOfPoints(); ndx++)
    {
      double coords[3];
      contourPoints->GetPoint(ndx, coords);
      coords[this->m_contourWidget->GetOrientation()] = this->m_pos;
      contourPoints->SetPoint(ndx, coords);
    }

    m_contourMap.insert(m_pos, contour);
  }
  else
  {
    // actual contour is NULL, remove any previous contour for this slice (if any exists)
    if ((m_contourMap.find(m_pos) != m_contourMap.end()))
        m_contourMap.remove(m_pos);
  }
}
