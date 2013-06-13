/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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
#include "ContourRenderer.h"
#include "GUI/Representations/ContourRepresentation.h"
#include "GUI/QtWidget/EspinaRenderView.h"
#include <GUI/QtWidget/SliceView.h>

// VTK
#include <vtkPropPicker.h>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  ContourRenderer::ContourRenderer(QObject *parent)
  : MeshRenderer(parent)
  {
  }
  
  //-----------------------------------------------------------------------------
  ContourRenderer::~ContourRenderer()
  {
  }

  //-----------------------------------------------------------------------------
  void ContourRenderer::addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep)
  {
    ContourRepresentationSPtr contour = boost::dynamic_pointer_cast<ContourRepresentation>(rep);
    if (contour.get() != NULL)
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
          prop->SetPickable(true);
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  void ContourRenderer::removeRepresentation(GraphicalRepresentationSPtr rep)
  {
    ContourRepresentationSPtr contour = boost::dynamic_pointer_cast<ContourRepresentation>(rep);
    if (contour.get() != NULL)
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

          if (m_representations[item].isEmpty())
            m_representations.remove(item);
        }
    }
  }

  //-----------------------------------------------------------------------------
  bool ContourRenderer::managesRepresentation(GraphicalRepresentationSPtr rep)
  {
    ContourRepresentationSPtr contour = boost::dynamic_pointer_cast<ContourRepresentation>(rep);
    return (contour.get() != NULL);
  }

  //-----------------------------------------------------------------------------
  ViewManager::Selection ContourRenderer::pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderabledItems itemType, bool repeat)
  {
    // FIXME: apparently the contours can't be picked, even when the actors
    // have been marked as pickable a call to m_picker->Pick() always returns
    // empty (just actors hard to pinpoint?)
    ViewManager::Selection selection;
    QList<vtkProp *> removedProps;

    if (!renderer || !renderer.GetPointer() || !itemType.testFlag(EspINA::SEGMENTATION))
      return selection;

    Nm pickPoint[3] = { x, y, ((m_view->getViewType() == AXIAL) ? -SliceView::SEGMENTATION_SHIFT : SliceView::SEGMENTATION_SHIFT) };

    while (m_picker->Pick(pickPoint, renderer))
    {
      vtkProp *pickedProp = m_picker->GetViewProp();
      Q_ASSERT(pickedProp);

      Nm point[3];
      m_picker->GetPickPosition(point);
      point[m_view->getViewType()] = z;

      m_picker->DeletePickList(pickedProp);
      removedProps << pickedProp;

      foreach(PickableItemPtr item, m_representations.keys())
      {
        if (!itemType.testFlag(item->type()))
        continue;

        foreach(GraphicalRepresentationSPtr rep, m_representations[item])
          if (rep->isVisible() && rep->hasActor(pickedProp) && rep->isInside(point) && !selection.contains(item))
          {
            selection << item;

            if (!repeat)
            {
              foreach(vtkProp *actor, removedProps)
                m_picker->AddPickList(actor);

              return selection;
            }

            break;
          }
      }
    }

    foreach(vtkProp *actor, removedProps)
      m_picker->AddPickList(actor);

    return selection;
  }


} /* namespace EspINA */
