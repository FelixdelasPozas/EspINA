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
#include "Widget2D.h"
#include "vtkWidget2D.h"
#include "Representation.h"
#include <GUI/View/Widgets/EspinaInteractorAdapter.h>

using namespace ESPINA;
using namespace ESPINA::GUI::View::Widgets;
using namespace ESPINA::GUI::View::Widgets::OrthogonalRegion;

using SliceWidgetAdapter = EspinaInteractorAdapter<vtkWidget2D>;

//----------------------------------------------------------------------------
class Widget2D::Command
: public vtkCommand
{
public:
  vtkTypeMacro(Command, vtkCommand);

  /** \brief Creates a new instance.
   *
   */
  static Command *New()
  { return new Command(); }

  void setRepresentation(Representation *representation)
  { m_representation = representation; }

  virtual void Execute(vtkObject *caller, unsigned long int eventId, void *callData);

private:
  explicit Command() {}

  Representation *m_representation;
};

//----------------------------------------------------------------------------
void Widget2D::Command::Execute(vtkObject *caller, long unsigned int eventId, void *callData)
{
  auto widget = static_cast<vtkWidget2D *>(caller);

  m_representation->setBounds(widget->GetBounds());
}



//----------------------------------------------------------------------------
Widget2D::Widget2D(Representation &representation)
: m_representation(representation)
, m_widget(vtkSmartPointer<vtkWidget2D>::New())
, m_command(vtkSmartPointer<Command>::New())
, m_index(0)
, m_slice(0)
{
  m_command->setRepresentation(&m_representation);
}

//----------------------------------------------------------------------------
void Widget2D::setPlane(Plane plane)
{
  m_index = normalCoordinateIndex(plane);

  m_widget->SetPlane(plane);
}

//----------------------------------------------------------------------------
void Widget2D::setRepresentationDepth(Nm depth)
{
  m_widget->SetDepth(depth);
}

//----------------------------------------------------------------------------
EspinaWidget2DSPtr Widget2D::clone()
{
  return std::make_shared<Widget2D>(m_representation);
}

//----------------------------------------------------------------------------
bool Widget2D::acceptCrosshairChange(const NmVector3 &crosshair) const
{
  return true;
}

//----------------------------------------------------------------------------
bool Widget2D::acceptSceneResolutionChange(const NmVector3 &resolution) const
{
  return true;
}

//----------------------------------------------------------------------------
void Widget2D::initializeImplementation(RenderView *view)
{
  onResolutionChanged(m_representation.resolution());
  onBoundsChanged    (m_representation.bounds());
  onColorChanged     (m_representation.representationColor());
  onPatternChanged   (m_representation.representationPattern());

  connect(&m_representation, SIGNAL(resolutionChanged(NmVector3)),
          this,              SLOT(onResolutionChanged(NmVector3)));

  connect(&m_representation, SIGNAL(boundsChanged(Bounds)),
          this,              SLOT(onBoundsChanged(Bounds)));

  connect(&m_representation, SIGNAL(colorChanged(QColor)),
          this,              SLOT(onColorChanged(QColor)));

  connect(&m_representation, SIGNAL(resolutionChanged(NmVector3)),
          this,              SLOT(onResolutionChanged(NmVector3)));

  m_widget->AddObserver(vtkCommand::EndInteractionEvent, m_command);
}

//----------------------------------------------------------------------------
void Widget2D::uninitializeImplementation()
{
  disconnect(&m_representation, SIGNAL(resolutionChanged(NmVector3)),
             this,              SLOT(onResolutionChanged(NmVector3)));

  disconnect(&m_representation, SIGNAL(boundsChanged(Bounds)),
             this,              SLOT(onBoundsChanged(Bounds)));

  disconnect(&m_representation, SIGNAL(colorChanged(QColor)),
             this,              SLOT(onColorChanged(QColor)));

  disconnect(&m_representation, SIGNAL(resolutionChanged(NmVector3)),
             this,              SLOT(onResolutionChanged(NmVector3)));

  m_widget->RemoveObserver(m_command);
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
void Widget2D::updateVisibility()
{
  auto axis    = toAxis(m_index);
  auto visible = contains(m_widget->GetBounds(), axis, m_slice);

  m_widget->SetEnabled(visible);
}

//----------------------------------------------------------------------------
void Widget2D::onResolutionChanged(const NmVector3 &resolution)
{
}

//----------------------------------------------------------------------------
void Widget2D::onBoundsChanged(const Bounds &bounds)
{
  m_widget->SetBounds(bounds);
}

//----------------------------------------------------------------------------
void Widget2D::onColorChanged(const QColor &color)
{
  double rgb[3] = {color.redF(), color.greenF(), color.blueF()};

  m_widget->setRepresentationColor(rgb);
}

//----------------------------------------------------------------------------
void Widget2D::onPatternChanged(const int pattern)
{
  m_widget->setRepresentationPattern(pattern);
}


//
//
// //----------------------------------------------------------------------------
// OrthogonalRegion::OrthogonalRegion(Bounds bounds)
// : m_bounds     {bounds}
// , m_pattern    {0xFFFF}
// , m_command    {vtkSmartPointer<vtkOrthogonalRegionCommand>::New()}
// {
//   m_command->setWidget(this);
//
//   m_resolution[0] = m_resolution[1] = m_resolution[2] = 1;
//
//   // default color
//   m_color[0] = m_color[1] = 1.0;
//   m_color[2] = 0.0;
// }
//