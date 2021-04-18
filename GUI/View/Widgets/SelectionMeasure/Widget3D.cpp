/*

 Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "Widget3D.h"
#include "vtkWidget3D.h"
#include <GUI/View/RenderView.h>

// VTK
#include <vtkRenderWindow.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::SelectionMeasure;


//----------------------------------------------------------------------------
Widget3D::Widget3D(SelectionSPtr selection)
: m_selection{selection}
, m_widget   {vtkSmartPointer<vtkWidget3D>::New()}
{
  if(selection)
  {
    m_selectedSegmentations = selection->segmentations();
  }
}

//----------------------------------------------------------------------------
TemporalRepresentation3DSPtr Widget3D::cloneImplementation()
{
  return std::make_shared<Widget3D>(m_selection);
}

//----------------------------------------------------------------------------
bool Widget3D::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return false;
}

//----------------------------------------------------------------------------
bool Widget3D::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return false;
}

//----------------------------------------------------------------------------
bool Widget3D::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return false;
}

//----------------------------------------------------------------------------
bool Widget3D::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  return false;
}

//----------------------------------------------------------------------------
void Widget3D::initializeImplementation(RenderView *view)
{
  m_widget->SetCurrentRenderer(view->mainRenderer());
  m_widget->SetInteractor(view->renderWindow()->GetInteractor());

  updateSelectionMeasure();

  synchronizeSelectionChanges();

  connect(m_selection.get(), SIGNAL(selectionChanged()),
          this,              SLOT(onSelectionChanged()));
}

//----------------------------------------------------------------------------
void Widget3D::uninitializeImplementation()
{
  m_widget->SetCurrentRenderer(nullptr);
  m_widget->SetInteractor(nullptr);

  disconnect(m_selection.get(), SIGNAL(selectionChanged()),
             this,              SLOT(onSelectionChanged()));

  desynchronizeSelectionChanges();
}

//----------------------------------------------------------------------------
vtkAbstractWidget *Widget3D::vtkWidget()
{
  return m_widget;
}

//----------------------------------------------------------------------------
void Widget3D::synchronizeSelectionChanges()
{
  for(auto segmentation : m_selectedSegmentations)
  {
    connect(segmentation, SIGNAL(outputModified()),
            this,         SLOT(updateSelectionMeasure()));
  }
}

//----------------------------------------------------------------------------
void Widget3D::desynchronizeSelectionChanges()
{
  for(auto segmentation : m_selectedSegmentations)
  {
    disconnect(segmentation, SIGNAL(outputModified()),
               this,         SLOT(updateSelectionMeasure()));
  }
}

//----------------------------------------------------------------------------
void Widget3D::onSelectionChanged()
{
  desynchronizeSelectionChanges();

  m_selectedSegmentations = m_selection->segmentations();

  synchronizeSelectionChanges();

  updateSelectionMeasure();
}

//----------------------------------------------------------------------------
void Widget3D::updateSelectionMeasure()
{
  Bounds selectionMeasure;

  for (auto segmentation : m_selectedSegmentations)
  {
    updateBoundingBox(selectionMeasure, segmentation->bounds());
  }

  if (!selectionMeasure.areValid() && m_selection->activeChannel())
  {
    updateBoundingBox(selectionMeasure, m_selection->activeChannel()->bounds());
  }

  m_widget->setBounds(selectionMeasure);
}
