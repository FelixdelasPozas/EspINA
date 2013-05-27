/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

// EspINA
#include "CrosshairRenderer.h"
#include "GUI/Representations/CrosshairRepresentation.h"
#include <Core/Model/Channel.h>
#include <GUI/ViewManager.h>
#include "GUI/QtWidget/EspinaRenderView.h"

// VTK
#include <vtkSmartPointer.h>
#include <vtkPropPicker.h>

using namespace EspINA;

//-----------------------------------------------------------------------------
CrosshairRenderer::CrosshairRenderer(QObject* parent)
: IRenderer(parent)
, m_picker(vtkSmartPointer<vtkPropPicker>::New())
{
  m_picker->PickFromListOn();
}

//-----------------------------------------------------------------------------
CrosshairRenderer::~CrosshairRenderer()
{
  foreach(PickableItemPtr item, m_representations.keys())
  {
    foreach(GraphicalRepresentationSPtr rep, m_representations[item])
    {
      foreach(vtkProp *prop, rep->getActors())
      {
        m_view->removeActor(prop);
        m_picker->DeletePickList(prop);
      }
    }
    m_representations[item].clear();
  }
  m_representations.clear();
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep)
{
  CrosshairRepresentationSPtr crossRep = boost::dynamic_pointer_cast<CrosshairRepresentation>(rep);
  if (crossRep.get() != NULL)
  {
    if (m_representations.keys().contains(item))
      m_representations[item] << rep;
    else
    {
      GraphicalRepresentationSList list;
      list << rep;
      m_representations.insert(item, list);
    }

    if (m_enable)
      foreach(vtkProp* prop, rep->getActors())
      {
        m_view->addActor(prop);
        m_picker->AddPickList(prop);
      }
  }
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::removeRepresentation(GraphicalRepresentationSPtr rep)
{
  CrosshairRepresentationSPtr crossRep = boost::dynamic_pointer_cast<CrosshairRepresentation>(rep);
  if (crossRep.get() != NULL)
  {
    foreach(PickableItemPtr item, m_representations.keys())
      if (m_representations[item].contains(rep))
      {
        if (m_enable)
          foreach(vtkProp* prop, rep->getActors())
          {
            m_view->removeActor(prop);
            m_picker->DeletePickList(prop);
          }

        m_representations[item].removeAll(rep);

        if (m_representations[item].empty())
          m_representations.remove(item);
      }
  }
}

//-----------------------------------------------------------------------------
bool CrosshairRenderer::hasRepresentation(GraphicalRepresentationSPtr rep)
{
  CrosshairRepresentationSPtr crossRep = boost::dynamic_pointer_cast<CrosshairRepresentation>(rep);
  if (crossRep.get() != NULL)
    foreach(PickableItemPtr item, m_representations.keys())
      if (m_representations[item].contains(rep))
        return true;

  return false;
}

//-----------------------------------------------------------------------------
bool CrosshairRenderer::managesRepresentation(GraphicalRepresentationSPtr rep)
{
  CrosshairRepresentationSPtr crossRep = boost::dynamic_pointer_cast<CrosshairRepresentation>(rep);
  return (crossRep.get() != NULL);
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::hide()
{
  if (!this->m_enable)
    return;

  foreach (PickableItemPtr item, m_representations.keys())
    foreach(GraphicalRepresentationSPtr rep, m_representations[item])
      foreach(vtkProp* prop, rep->getActors())
      {
        m_view->removeActor(prop);
        m_picker->DeletePickList(prop);
      }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::show()
{
  if (this->m_enable)
    return;

  foreach (PickableItemPtr item, m_representations.keys())
    foreach(GraphicalRepresentationSPtr rep, m_representations[item])
      foreach(vtkProp* prop, rep->getActors())
      {
        m_view->addActor(prop);
        m_picker->AddPickList(prop);
      }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
unsigned int CrosshairRenderer::getNumberOfvtkActors()
{
  unsigned int numActors = 0;

  foreach (PickableItemPtr item, m_representations.keys())
    foreach(GraphicalRepresentationSPtr rep, m_representations[item])
      if (rep->isVisible())
        numActors += 6;

  return numActors;
}


//-----------------------------------------------------------------------------
ViewManager::Selection CrosshairRenderer::pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderabledItems itemType, bool repeat)
{
  ViewManager::Selection selection;
  QList<vtkProp*> removedProps;

  if (!renderer || !renderer.GetPointer() || !itemType.testFlag(EspINA::CHANNEL))
    return selection;

  while (m_picker->Pick(x, y, 0, renderer))
  {
    vtkProp *pickedProp = m_picker->GetViewProp();
    Q_ASSERT(pickedProp);

    m_picker->DeletePickList(pickedProp);
    removedProps << pickedProp;

    foreach(PickableItemPtr item, m_representations.keys())
      foreach(GraphicalRepresentationSPtr rep, m_representations[item])
        if (rep->isVisible() && rep->hasActor(pickedProp) && !selection.contains(item))
        {
          selection << item;

          if (!repeat)
          {
            foreach(vtkProp *actor, removedProps)
              m_picker->AddPickList(actor);

            return selection;
          }

          // channels have multiple actors, we must eliminate five more of them and continue searching
          foreach(vtkProp *actor, rep->getActors())
          if (actor != pickedProp)
          {
            m_picker->DeletePickList(actor);
            removedProps << actor;
          }

          break;
        }
  }

  foreach(vtkProp *actor, removedProps)
    m_picker->AddPickList(actor);

  return selection;
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::getPickCoordinates(double *point)
{
  m_picker->GetPickPosition(point);
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3])
{
  foreach(PickableItemPtr item, m_representations.keys())
    foreach(GraphicalRepresentationSPtr rep, m_representations[item])
    {
      CrosshairRepresentationSPtr crossRep = boost::dynamic_pointer_cast<CrosshairRepresentation>(rep);
      crossRep->setCrosshairColors(axialColor, coronalColor, sagittalColor);
    }
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setCrosshair(Nm point[3])
{
  foreach(PickableItemPtr item, m_representations.keys())
    foreach(GraphicalRepresentationSPtr rep, m_representations[item])
    {
      CrosshairRepresentationSPtr crossRep = boost::dynamic_pointer_cast<CrosshairRepresentation>(rep);
      crossRep->setCrosshair(point);
    }
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setPlanePosition(PlaneType plane, Nm dist)
{
  foreach(PickableItemPtr item, m_representations.keys())
    foreach(GraphicalRepresentationSPtr rep, m_representations[item])
    {
      CrosshairRepresentationSPtr crossRep = boost::dynamic_pointer_cast<CrosshairRepresentation>(rep);
      crossRep->setPlanePosition(plane, dist);
    }
}
