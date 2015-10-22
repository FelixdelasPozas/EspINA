/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "OrthogonalWidget2D.h"
#include "vtkOrthogonalWidget2D.h"
#include "OrthogonalRepresentation.h"

#include <GUI/View/Widgets/EspinaInteractorAdapter.h>
#include <GUI/Representations/Frame.h>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::OrthogonalRegion;

using SliceWidgetAdapter = EspinaInteractorAdapter<vtkOrthogonalWidget2D>;

//----------------------------------------------------------------------------
class OrthogonalWidget2D::Command
: public vtkCommand
{
public:
  vtkTypeMacro(Command, vtkCommand);

  /** \brief Creates a new instance.
   *
   */
  static Command *New()
  { return new Command(); }

  void setRepresentation(OrthogonalRepresentation *region)
  { m_region = region; }

  virtual void Execute(vtkObject *caller, unsigned long int eventId, void *callData);

private:
  explicit Command() {}

  OrthogonalRepresentation *m_region;
};

//----------------------------------------------------------------------------
void OrthogonalWidget2D::Command::Execute(vtkObject *caller, long unsigned int eventId, void *callData)
{
  auto widget = static_cast<vtkOrthogonalWidget2D *>(caller);

  m_region->setBounds(widget->GetBounds());
}


//----------------------------------------------------------------------------
OrthogonalWidget2D::OrthogonalWidget2D(OrthogonalRepresentationSPtr representation)
: m_representation(representation)
, m_widget(vtkSmartPointer<vtkOrthogonalWidget2D>::New())
, m_command(vtkSmartPointer<Command>::New())
, m_index(0)
, m_slice(0)
{
  m_command->setRepresentation(m_representation.get());
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::setPlane(Plane plane)
{
  m_index = normalCoordinateIndex(plane);

  m_widget->SetPlane(plane);
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::setRepresentationDepth(Nm depth)
{
  m_widget->SetDepth(depth);
}

//----------------------------------------------------------------------------
TemporalRepresentation2DSPtr OrthogonalWidget2D::clone()
{
  return std::make_shared<OrthogonalWidget2D>(m_representation);
}

//----------------------------------------------------------------------------
bool OrthogonalWidget2D::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return m_slice != crosshair[m_index];
}

//----------------------------------------------------------------------------
bool OrthogonalWidget2D::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return m_resolution != resolution[m_index];
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::initializeImplementation(RenderView *view)
{
  onModeChanged      (m_representation->mode());
  onResolutionChanged(m_representation->resolution());
  onBoundsChanged    (m_representation->bounds());
  onColorChanged     (m_representation->representationColor());
  onPatternChanged   (m_representation->representationPattern());

  connect(m_representation.get(), SIGNAL(modeChanged(OrthogonalRepresentation::Mode)),
          this,                   SLOT(onModeChanged(OrthogonalRepresentation::Mode)));

  connect(m_representation.get(), SIGNAL(resolutionChanged(NmVector3)),
          this,                   SLOT(onResolutionChanged(NmVector3)));

  connect(m_representation.get(), SIGNAL(boundsChanged(Bounds)),
          this,      SLOT(onBoundsChanged(Bounds)));

  connect(m_representation.get(), SIGNAL(colorChanged(QColor)),
          this,                   SLOT(onColorChanged(QColor)));

  connect(m_representation.get(), SIGNAL(patternChanged(int)),
          this,                   SLOT(onPatternChanged(int)));

  m_widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::uninitializeImplementation()
{
  disconnect(m_representation.get(), SIGNAL(modeChanged(OrthogonalRepresentation::Mode)),
             this,                   SLOT(onModeChanged(OrthogonalRepresentation::Mode)));

  disconnect(m_representation.get(), SIGNAL(resolutionChanged(NmVector3)),
             this,                   SLOT(onResolutionChanged(NmVector3)));

  disconnect(m_representation.get(), SIGNAL(boundsChanged(Bounds)),
             this,                   SLOT(onBoundsChanged(Bounds)));

  disconnect(m_representation.get(), SIGNAL(colorChanged(QColor)),
             this,                   SLOT(onColorChanged(QColor)));

  disconnect(m_representation.get(), SIGNAL(patternChanged(int)),
             this,                   SLOT(onPatternChanged(int)));

  m_widget->RemoveObserver(m_command);
}

//----------------------------------------------------------------------------
vtkAbstractWidget *OrthogonalWidget2D::vtkWidget()
{
  return m_widget;
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::display(const FrameCSPtr& frame)
{
  m_slice = frame->crosshair[m_index];

  m_widget->SetSlice(m_slice);
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::onModeChanged(const OrthogonalRepresentation::Mode mode)
{
  if (mode == OrthogonalRepresentation::Mode::RESIZABLE)
  {
    m_widget->ProcessEventsOn();
  }
  else
  {
    m_widget->ProcessEventsOff();
  }
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::onResolutionChanged(const NmVector3 &resolution)
{
  m_resolution = resolution[m_index];
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::onBoundsChanged(const Bounds &bounds)
{
  m_widget->SetBounds(bounds);
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::onColorChanged(const QColor &color)
{
  double rgb[3] = {color.redF(), color.greenF(), color.blueF()};

  m_widget->setRepresentationColor(rgb);
}

//----------------------------------------------------------------------------
void OrthogonalWidget2D::onPatternChanged(const int pattern)
{
  m_widget->setRepresentationPattern(pattern);
}