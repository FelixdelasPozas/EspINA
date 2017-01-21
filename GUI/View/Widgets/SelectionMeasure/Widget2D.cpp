/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include "Widget2D.h"
#include "vtkWidget2D.h"
#include <GUI/View/RenderView.h>
#include <GUI/View/View2D.h>

// VTK
#include <vtkRenderer.h>
#include <vtkCamera.h>

// Qt
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::SelectionMeasure;

class Widget2D::Command
: public vtkCommand
{
public:
  vtkTypeMacro(Command, vtkCommand);

  /** \brief VTK-style New() constructor, required for using vtkSmartPointer.
   *
   */
  static Command *New()
  { return new Command(); }

  void setWidget(vtkWidget2D *widget);

  virtual void Execute(vtkObject *caller, unsigned long int eventId, void *callData);

private:
  explicit Command()
  : m_widget(nullptr)
  {}

  virtual ~Command() {};

private:
  vtkWidget2D *m_widget;
};

//----------------------------------------------------------------------------
void Widget2D::Command::setWidget(vtkWidget2D *widget)
{
  m_widget = widget;
}

//----------------------------------------------------------------------------
void Widget2D::Command::Execute(vtkObject *caller, long unsigned int eventId, void *callData)
{
  if(strcmp("vtkOpenGLCamera", caller->GetClassName()) == 0)
  {
    auto camera = dynamic_cast<vtkCamera *>(caller);
    Q_ASSERT(camera);

    m_widget->drawActors();
  }
}

//----------------------------------------------------------------------------
Widget2D::Widget2D(SelectionSPtr selection)
: m_index                {0}
, m_selection            {selection}
, m_widget               {vtkSmartPointer<vtkWidget2D>::New()}
, m_command              {vtkSmartPointer<Command>::New()}
, m_camera               {nullptr}
, m_slice                {0}
{
  if(selection)
  {
    m_selectedSegmentations = selection->segmentations();
  }
}

//----------------------------------------------------------------------------
void Widget2D::setPlane(Plane plane)
{
  m_widget->setPlane(plane);

  m_index = normalCoordinateIndex(plane);
}

//----------------------------------------------------------------------------
void Widget2D::setRepresentationDepth(Nm depth)
{
}

//----------------------------------------------------------------------------
TemporalRepresentation2DSPtr Widget2D::cloneImplementation()
{
  return std::make_shared<Widget2D>(m_selection);
}

//----------------------------------------------------------------------------
bool Widget2D::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return (crosshair[m_index] != m_slice);
}

//----------------------------------------------------------------------------
bool Widget2D::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return false;
}

//----------------------------------------------------------------------------
bool Widget2D::acceptSceneBoundsChange(const Bounds &bounds) const
{
  return false;
}

//----------------------------------------------------------------------------
bool Widget2D::acceptInvalidationFrame(const GUI::Representations::FrameCSPtr frame) const
{
  return false;
}

//----------------------------------------------------------------------------
void Widget2D::initializeImplementation(RenderView *view)
{
  m_camera = view->mainRenderer()->GetActiveCamera();

  m_camera->AddObserver(vtkCommand::ModifiedEvent, m_command);

  m_command->setWidget(m_widget);

  m_widget->setRepresentationDepth(view2D_cast(view)->widgetDepth());

  updateSelectionMeasure();

  synchronizeSelectionChanges();

  connect(m_selection.get(), SIGNAL(selectionChanged()),
          this,              SLOT(onSelectionChanged()));
}

//----------------------------------------------------------------------------
void Widget2D::uninitializeImplementation()
{
  m_camera->RemoveObserver(m_command);
  m_command->setWidget(nullptr);

  disconnect(m_selection.get(), SIGNAL(selectionChanged()),
             this,              SLOT(onSelectionChanged()));

  desynchronizeSelectionChanges();
}

//----------------------------------------------------------------------------
vtkAbstractWidget *Widget2D::vtkWidget()
{
  return m_widget;
}

//----------------------------------------------------------------------------
void Widget2D::setCrosshair(const NmVector3 &crosshair)
{
  m_slice = crosshair[m_index];

  updateVisibility();
}

//----------------------------------------------------------------------------
void Widget2D::synchronizeSelectionChanges()
{
  for(auto segmentation : m_selectedSegmentations)
  {
    connect(segmentation, SIGNAL(outputModified()),
            this,         SLOT(updateSelectionMeasure()));
  }
}

//----------------------------------------------------------------------------
void Widget2D::desynchronizeSelectionChanges()
{
  for(auto segmentation : m_selectedSegmentations)
  {
    disconnect(segmentation, SIGNAL(outputModified()),
               this,         SLOT(updateSelectionMeasure()));
  }
}

//----------------------------------------------------------------------------
void Widget2D::updateVisibility()
{
  auto axis    = toAxis(m_index);
  auto visible = contains(m_widget->bounds(), axis, m_slice);
  m_widget->setSlice(m_slice);

  if(visible != m_widget->GetEnabled())
  {
    m_widget->SetEnabled(visible);
  }
}

//----------------------------------------------------------------------------
void Widget2D::onSelectionChanged()
{
  desynchronizeSelectionChanges();

  m_selectedSegmentations = m_selection->segmentations();

  synchronizeSelectionChanges();

  updateSelectionMeasure();
}

//----------------------------------------------------------------------------
void Widget2D::updateSelectionMeasure()
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

  updateVisibility();
}
