/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Felix de las Pozas Alvarez <felixdelaspozas@gmail.com>

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
#include <GUI/View/View2D.h>
#include "ContourRenderer.h"
#include "GUI/Representations/ContourRepresentation.h"

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
  void ContourRenderer::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
  {
    ContourRepresentationSPtr contour = std::dynamic_pointer_cast<ContourRepresentation>(rep);
    if (contour.get() != nullptr)
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
        for (auto prop: rep->getActors())
        {
          prop->SetPickable(true);
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  void ContourRenderer::removeRepresentation(RepresentationSPtr rep)
  {
    ContourRepresentationSPtr contour = std::dynamic_pointer_cast<ContourRepresentation>(rep);
    if (contour.get() != nullptr)
    {
      for (auto item: m_representations.keys())
        if (m_representations[item].contains(rep))
        {
          if (m_enable)
            for (auto prop: rep->getActors())
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
  bool ContourRenderer::managesRepresentation(RepresentationSPtr rep)
  {
    ContourRepresentationSPtr contour = std::dynamic_pointer_cast<ContourRepresentation>(rep);
    return (contour.get() != nullptr);
  }

  //-----------------------------------------------------------------------------
  ViewItemAdapterList ContourRenderer::pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType, bool repeat)
  {
    // FIXME: apparently the contours can't be picked, even when the actors
    // have been marked as pickable a call to m_picker->Pick() always returns
    // empty (just actors hard to pinpoint?)
    ViewItemAdapterList selection;
    QList<vtkProp *> removedProps;

    if (!renderer || !renderer.GetPointer() || !itemType.testFlag(EspINA::SEGMENTATION))
      return selection;

    View2D *view = static_cast<View2D *>(m_view);
    Nm pickPoint[3] = { static_cast<Nm>(x), static_cast<Nm>(y), ((view->plane() == Plane::XY) ? -View2D::SEGMENTATION_SHIFT : View2D::SEGMENTATION_SHIFT) };

    while (m_picker->Pick(pickPoint, renderer))
    {
      vtkProp *pickedProp = m_picker->GetViewProp();
      Q_ASSERT(pickedProp);

      Nm point[3];
      m_picker->GetPickPosition(point);
      point[normalCoordinateIndex(view->plane())] = z;

      m_picker->DeletePickList(pickedProp);
      removedProps << pickedProp;

      for (auto item: m_representations.keys())
      {
        if (!(item->type() == ViewItemAdapter::Type::SEGMENTATION && itemType.testFlag(RenderableType::SEGMENTATION)))
          continue;

        NmVector3 vecPoint{ point[0], point[1], point[2] };
        for (auto rep: m_representations[item])
          if (rep->isVisible() && rep->hasActor(pickedProp) && rep->isInside(vecPoint) && !selection.contains(item))
          {
            selection << item;

            if (!repeat)
            {
              for (auto actor: removedProps)
                m_picker->AddPickList(actor);

              return selection;
            }

            break;
          }
      }
    }

    for (auto actor: removedProps)
      m_picker->AddPickList(actor);

    return selection;
  }

  //-----------------------------------------------------------------------------
  void ContourRenderer::setView(RenderView* view)
  {
    m_view = view;
  }

} /* namespace EspINA */
