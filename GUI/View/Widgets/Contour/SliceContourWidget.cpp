/*

    Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ESPINA
#include <GUI/View/Widgets/Contour/SliceContourWidget.h>
#include <GUI/View/Widgets/Contour/vtkPlaneContourRepresentationGlyph.h>
#include <vtkObjectFactory.h>
#include <vtkPolyData.h>

// Qt
#include <QPolygon>

using namespace ESPINA;

//-----------------------------------------------------------------------------
SliceContourWidget::SliceContourWidget(vtkPlaneContourWidget* widget)
: m_initialized          {false}
, m_plane                {Plane::UNDEFINED}
, m_pos                  {0}
, m_contourWidget        {widget}
, m_storedContour        {nullptr}
, m_storedContourPosition{-1}
, m_storedContourMode    {BrushSelector::BrushMode::BRUSH}
{
}

//-----------------------------------------------------------------------------
SliceContourWidget::~SliceContourWidget()
{
  m_contourWidget->Delete();

  if (m_storedContour)
  {
    m_storedContour->Delete();
  }
}

//-----------------------------------------------------------------------------
void SliceContourWidget::setSlice(Nm pos, Plane plane)
{
  if (!m_initialized)
  {
    m_initialized = true;
    m_plane = plane;
    m_pos = pos;
    m_contourWidget->SetOrientation(plane);
    m_contourWidget->setSlice(pos);
  }
  else
  {
    if (plane != m_plane || pos == m_pos)
      return;

    if ((m_storedContour) && (m_storedContourPosition == pos))
    {
      m_contourWidget->setActualContourMode(m_storedContourMode);
      m_contourWidget->Initialize(m_storedContour);

      m_storedContour->Delete();
      m_storedContour = nullptr;
      m_storedContourPosition = -1;
    }
    else
    {
      auto rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(this->m_contourWidget->GetRepresentation());
      auto polyData = rep->GetContourRepresentationAsPolyData();
      if (polyData && (polyData->GetPoints()->GetNumberOfPoints() != 0))
      {
        Q_ASSERT(!m_storedContour);

        m_storedContour = vtkPolyData::New();
        m_storedContour->DeepCopy(polyData);
        m_storedContourPosition = m_pos;
        m_storedContourMode = m_contourWidget->getContourMode();
      }

      m_contourWidget->Initialize(nullptr);
    }

    m_pos = pos;
  }
}

//-----------------------------------------------------------------------------
QPair<BrushSelector::BrushMode, vtkPolyData *> SliceContourWidget::getContour()
{
  if (!m_initialized) Q_ASSERT(false);

  QPair<BrushSelector::BrushMode, vtkPolyData *> result(BrushSelector::BrushMode::BRUSH, nullptr);

  auto rep = reinterpret_cast<vtkPlaneContourRepresentationGlyph*>(this->m_contourWidget->GetRepresentation());
  auto polyData = rep->GetContourRepresentationAsPolyData();
  if (polyData && (polyData->GetPoints()->GetNumberOfPoints() != 0))
  {
    auto contour = vtkPolyData::New();
    contour->DeepCopy(polyData);

    // points in the contour must be corrected according to slice.
    vtkPoints* contourPoints = contour->GetPoints();
    for (int ndx = 0; ndx < contourPoints->GetNumberOfPoints(); ndx++)
    {
      double coords[3];
      contourPoints->GetPoint(ndx, coords);
      coords[normalCoordinateIndex(m_contourWidget->GetOrientation())] = m_pos;
      contourPoints->SetPoint(ndx, coords);
    }

    result.first = m_contourWidget->getContourMode();
    result.second = contour;
  }
  else
  {
    if (m_storedContour)
    {
      // points in the contour must be corrected according to slice.
      auto contourPoints = m_storedContour->GetPoints();
      for (int ndx = 0; ndx < contourPoints->GetNumberOfPoints(); ndx++)
      {
        double coords[3];
        contourPoints->GetPoint(ndx, coords);
        coords[normalCoordinateIndex(m_contourWidget->GetOrientation())] = m_storedContourPosition;
        contourPoints->SetPoint(ndx, coords);
      }

      result.first = m_storedContourMode;
      result.second = m_storedContour;

      m_storedContour = nullptr;
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
void SliceContourWidget::setMode(BrushSelector::BrushMode mode)
{
   this->m_contourWidget->setContourMode(mode);
}

//-----------------------------------------------------------------------------
void SliceContourWidget::Initialize()
{
  m_contourWidget->Initialize(nullptr);
}

//-----------------------------------------------------------------------------
void SliceContourWidget::Initialize(ContourWidget::ContourData contour)
{
  if (m_plane != contour.plane || contour.polyData == nullptr)
  {
    Initialize();
    return;
  }

  if (m_storedContour)
  {
    m_storedContour->Delete();
    m_storedContour = nullptr;
    m_storedContourPosition = -1;
  }

  Nm contourPos = contour.polyData->GetPoints()->GetPoint(0)[normalCoordinateIndex(m_plane)];

  if (m_pos == contourPos)
  {
    m_contourWidget->setActualContourMode(contour.mode);
    m_contourWidget->Initialize(contour.polyData);
  }
  else
  {
    m_storedContour = vtkPolyData::New();
    m_storedContour->DeepCopy(contour.polyData);
    m_storedContourPosition = contourPos;
    m_storedContourMode = contour.mode;
  }
}
