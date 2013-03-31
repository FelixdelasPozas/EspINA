/*
 * SliceContourWidget.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include "SliceContourWidget.h"
#include "vtkPlaneContourRepresentationGlyph.h"

// VTK
#include <vtkObjectFactory.h>

// Qt
#include <QPolygon>

using namespace EspINA;

//-----------------------------------------------------------------------------
SliceContourWidget::SliceContourWidget(vtkPlaneContourWidget* widget)
: SliceWidget(widget)
, m_initialized(false)
, m_plane(AXIAL)
, m_pos(0)
, m_contourWidget(widget)
{
}

//-----------------------------------------------------------------------------
SliceContourWidget::~SliceContourWidget()
{
  m_contourWidget->Delete();
}

//-----------------------------------------------------------------------------
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
    if (plane != m_plane || pos == m_pos)
      return;

    m_pos = pos;
  }
}

//-----------------------------------------------------------------------------
QPair<Brush::BrushMode, vtkPolyData *> SliceContourWidget::getContour()
{
  if (!m_initialized)
    Q_ASSERT(false);

  QPair<Brush::BrushMode, vtkPolyData *> result(Brush::BRUSH, NULL);

  vtkPlaneContourRepresentationGlyph *rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(this->m_widget->GetRepresentation());
  if ((rep->GetContourRepresentationAsPolyData()->GetPoints()->GetNumberOfPoints() != 0) && rep->GetClosedLoop())
  {
    vtkPolyData* contour = vtkPolyData::New();
    contour->DeepCopy(rep->GetContourRepresentationAsPolyData());

    // points in the contour must be corrected according to slice.
    vtkPoints* contourPoints = contour->GetPoints();
    for (int ndx = 0; ndx < contourPoints->GetNumberOfPoints(); ndx++)
    {
      double coords[3];
      contourPoints->GetPoint(ndx, coords);
      coords[this->m_contourWidget->GetOrientation()] = m_pos;
      contourPoints->SetPoint(ndx, coords);
    }

    result.first = m_contourWidget->getContourMode();
    result.second = contour;
  }

  return result;
}

//-----------------------------------------------------------------------------
void SliceContourWidget::SetEnabled(int value)
{
  this->m_contourWidget->SetEnabled(value);
}

//-----------------------------------------------------------------------------
void SliceContourWidget::setMode(Brush::BrushMode mode)
{
   this->m_contourWidget->setContourMode(mode);
}

//-----------------------------------------------------------------------------
void SliceContourWidget::Initialize()
{
  m_contourWidget->Initialize(NULL);
}
