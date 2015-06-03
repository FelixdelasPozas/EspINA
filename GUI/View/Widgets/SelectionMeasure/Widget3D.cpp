/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "Widget3D.h"

#include "vtkWidget3D.h"
#include <GUI/View/RenderView.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::SelectionMeasure;


//----------------------------------------------------------------------------
Widget3D::Widget3D(SelectionSPtr selection)
: m_selection(selection)
, m_widget(vtkSmartPointer<vtkWidget3D>::New())
, m_selectedSegmentations(selection->segmentations())
{
}

//----------------------------------------------------------------------------
TemporalRepresentation3DSPtr Widget3D::clone()
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
void Widget3D::initializeImplementation(RenderView *view)
{
  updateSelectionMeasure();

  synchronizeSelectionChanges();

  connect(m_selection.get(), SIGNAL(selectionChanged()),
          this,              SLOT(onSelectionChanged()));
}

//----------------------------------------------------------------------------
void Widget3D::uninitializeImplementation()
{
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

  if (!selectionMeasure.areValid())
  {
    for (auto channel : m_selection->channels())
    {
      updateBoundingBox(selectionMeasure, channel->bounds());
    }
  }

  m_widget->setBounds(selectionMeasure);
}