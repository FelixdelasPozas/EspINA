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
#include <Deprecated/GUI/Representations/CrosshairRepresentation.h>
#include <Deprecated/GUI/Representations/Renderers/CrosshairRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkPropPicker.h>

// Qt
#include <QApplication>

using namespace ESPINA;

//-----------------------------------------------------------------------------
CrosshairRenderer::CrosshairRenderer(QObject* parent)
: ChannelRenderer{parent}
, m_picker       {nullptr}
{
}

//-----------------------------------------------------------------------------
CrosshairRenderer::~CrosshairRenderer()
{
  for(auto item: m_representations.keys())
  {
    if (m_enable)
      for(auto rep: m_representations[item])
        for(auto prop: rep->getActors())
          m_view->removeActor(prop);

    m_representations[item].clear();
  }
  m_representations.clear();

  if(m_picker != nullptr)
    m_picker->GetPickList()->RemoveAllItems();
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setView(RenderView *view)
{
  Renderer::setView(view);

  m_picker = vtkSmartPointer<vtkPropPicker>::New();
  m_picker->InitializePickList();
  m_picker->PickFromListOn();
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
{
  auto crossRep = std::dynamic_pointer_cast<CrosshairRepresentation>(rep);
  if (crossRep.get() != nullptr)
  {
    if (m_representations.keys().contains(item))
      m_representations[item] << rep;
    else
    {
      RepresentationSList list;
      list << rep;
      m_representations.insert(item, list);
    }

    if (m_enable)
      for(auto prop: rep->getActors())
      {
        m_view->addActor(prop);
        m_picker->AddPickList(prop);
      }
  }
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::removeRepresentation(RepresentationSPtr rep)
{
  auto crossRep = std::dynamic_pointer_cast<CrosshairRepresentation>(rep);
  if (crossRep.get() != nullptr)
  {
    for(auto item: m_representations.keys())
      if (m_representations[item].contains(rep))
      {
        if (m_enable)
          for(auto prop: rep->getActors())
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
bool CrosshairRenderer::hasRepresentation(RepresentationSPtr rep) const
{
  auto crossRep = std::dynamic_pointer_cast<CrosshairRepresentation>(rep);
  if (crossRep.get() != nullptr)
    for(auto item: m_representations.keys())
      if (m_representations[item].contains(rep))
        return true;

  return false;
}

//-----------------------------------------------------------------------------
bool CrosshairRenderer::canRender(ItemAdapterPtr item) const
{
  if(isChannel(item))
  {
    auto viewItem = dynamic_cast<ViewItemAdapterPtr>(item);
    if(viewItem != nullptr)
    {
      return hasVolumetricData(viewItem->output());
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
bool CrosshairRenderer::managesRepresentation(const QString &repType) const
{
  return (repType == CrosshairRepresentation::TYPE);
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::hide()
{
  if (!m_enable)
    return;

  for (auto item: m_representations.keys())
    for(auto rep: m_representations[item])
    {
      rep->setVisible(false);
      for(auto prop: rep->getActors())
      {
        m_view->removeActor(prop);
        m_picker->DeletePickList(prop);
      }
    }

  emit renderRequested();
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::show()
{
  if (m_enable)
    return;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  for(auto item: m_representations.keys())
    for(auto rep: m_representations[item])
    {
      rep->setVisible(true);
      for(auto prop: rep->getActors())
      {
        m_view->addActor(prop);
        m_picker->AddPickList(prop);
      }
    }

  QApplication::restoreOverrideCursor();
  emit renderRequested();
}

//-----------------------------------------------------------------------------
unsigned int CrosshairRenderer::numberOfvtkActors() const
{
  unsigned int numActors = 0;

  for(auto item: m_representations.keys())
    for(auto rep: m_representations[item])
      if (rep->isVisible())
        numActors += 6;

  return numActors;
}


//-----------------------------------------------------------------------------
ViewItemAdapterList CrosshairRenderer::pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType, bool repeat)
{
  ViewItemAdapterList selection;
  QList<vtkProp*> removedProps;

  if (!renderer || !renderer.GetPointer() || !itemType.testFlag(ESPINA::CHANNEL))
    return selection;

  while (m_picker->Pick(x, y, 0, renderer))
  {
    double point[3];
    m_picker->GetPickPosition(point);
    m_lastValidPickPosition = NmVector3{ point[0], point[1], point[2] };

    auto pickedProp = m_picker->GetViewProp();
    Q_ASSERT(pickedProp);

    m_picker->DeletePickList(pickedProp);
    removedProps << pickedProp;

    for(auto item: m_representations.keys())
      for(auto rep: m_representations[item])
        if (rep->isVisible() && rep->hasActor(pickedProp) && !selection.contains(item))
        {
          selection << item;

          if (!repeat)
          {
            for(auto actor: removedProps)
              m_picker->AddPickList(actor);

            return selection;
          }

          // channels have multiple actors, we must eliminate five more of them and continue searching
          for(auto actor: rep->getActors())
          if (actor != pickedProp)
          {
            m_picker->DeletePickList(actor);
            removedProps << actor;
          }

          break;
        }
  }

  for(auto actor: removedProps)
    m_picker->AddPickList(actor);

  return selection;
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setCrosshairColors(double axialColor[3], double coronalColor[3], double sagittalColor[3])
{
  for(auto item: m_representations.keys())
    for(auto rep: m_representations[item])
    {
      auto crossRep = std::dynamic_pointer_cast<CrosshairRepresentation>(rep);
      crossRep->setCrosshairColors(axialColor, coronalColor, sagittalColor);
    }
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setCrosshair(NmVector3 point)
{
  for(auto item: m_representations.keys())
    for(auto rep: m_representations[item])
    {
      auto crossRep = std::dynamic_pointer_cast<CrosshairRepresentation>(rep);
      crossRep->setCrosshair(point);
    }
}

//-----------------------------------------------------------------------------
void CrosshairRenderer::setPlanePosition(Plane plane, Nm pos)
{
  for(auto item: m_representations.keys())
    for(auto rep: m_representations[item])
    {
      auto crossRep = std::dynamic_pointer_cast<CrosshairRepresentation>(rep);
      crossRep->setPlanePosition(plane, pos);
    }
}
