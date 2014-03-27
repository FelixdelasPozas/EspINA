/*
 * SliceContourWidget.cpp
 *
 *  Created on: Sep 8, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// EspINA
#include "SliceContourWidget.h"
#include "vtkPlaneContourRepresentationGlyph.h"

// VTK
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

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
, m_storedContour(NULL)
, m_storedContourPosition(-1)
, m_storedContourMode(Brush::BRUSH)
{
}

//-----------------------------------------------------------------------------
SliceContourWidget::~SliceContourWidget()
{
  m_contourWidget->Delete();

  if (m_storedContour != NULL)
    m_storedContour->Delete();
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

    if ((m_storedContour != NULL) && (m_storedContourPosition == pos))
    {
      m_contourWidget->setActualContourMode(m_storedContourMode);
      m_contourWidget->Initialize(m_storedContour);

      m_storedContour->Delete();
      m_storedContour = NULL;
      m_storedContourPosition = -1;
    }
    else
    {
      vtkPlaneContourRepresentationGlyph *rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(this->m_widget->GetRepresentation());
      if ((rep->GetContourRepresentationAsPolyData()->GetPoints()->GetNumberOfPoints() != 0) && rep->GetClosedLoop())
      {
        Q_ASSERT(m_storedContour == NULL);

        m_storedContour = vtkPolyData::New();
        m_storedContour->DeepCopy(rep->GetContourRepresentationAsPolyData());
        m_storedContourPosition = m_pos;
        m_storedContourMode = m_contourWidget->getContourMode();
      }

      m_contourWidget->Initialize(NULL);
    }

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
      coords[m_contourWidget->GetOrientation()] = m_pos;
      contourPoints->SetPoint(ndx, coords);
    }

    result.first = m_contourWidget->getContourMode();
    result.second = contour;
  }
  else
  {
    if (m_storedContour != NULL)
    {
      // points in the contour must be corrected according to slice.
      vtkPoints* contourPoints = m_storedContour->GetPoints();
      for (int ndx = 0; ndx < contourPoints->GetNumberOfPoints(); ndx++)
      {
        double coords[3];
        contourPoints->GetPoint(ndx, coords);
        coords[m_contourWidget->GetOrientation()] = m_storedContourPosition;
        contourPoints->SetPoint(ndx, coords);
      }

      result.first = m_storedContourMode;
      result.second = m_storedContour;

      m_storedContour = NULL;
    }
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

//-----------------------------------------------------------------------------
void SliceContourWidget::Initialize(ContourWidget::ContourData contour)
{
  if (m_plane != contour.Plane || contour.PolyData == NULL)
  {
    Initialize();
    return;
  }

  if (m_storedContour != NULL)
  {
    m_storedContour->Delete();
    m_storedContour = NULL;
    m_storedContourPosition = -1;
  }

  Nm contourPos = contour.PolyData->GetPoints()->GetPoint(0)[m_plane];

  if (m_pos == contourPos)
  {
    m_contourWidget->setActualContourMode(contour.Mode);
    m_contourWidget->Initialize(contour.PolyData);
  }
  else
  {
    m_storedContour = vtkPolyData::New();
    m_storedContour->DeepCopy(contour.PolyData);
    m_storedContourPosition = contourPos;
    m_storedContourMode = contour.Mode;
  }
}
