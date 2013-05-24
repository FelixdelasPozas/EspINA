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
#include "SliceRenderer.h"
#include "GUI/Representations/SliceRepresentation.h"
#include <GUI/ViewManager.h>
#include "GUI/QtWidget/EspinaRenderView.h"
#include <Core/Model/PickableItem.h>

// VTK
#include <vtkPropPicker.h>
#include <QDebug>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  SliceRenderer::SliceRenderer(QObject *parent)
  : IRenderer(parent)
  , m_picker(vtkSmartPointer<vtkPropPicker>::New())
  {
    m_picker->PickFromListOn();
  }

  //-----------------------------------------------------------------------------
  SliceRenderer::~SliceRenderer()
  {
    foreach(PickableItemPtr item, m_representations.keys())
    {
      foreach(GraphicalRepresentationSPtr rep, m_representations[item])
      {
        if (m_enable)
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
  void SliceRenderer::addRepresentation(PickableItemPtr item, GraphicalRepresentationSPtr rep)
  {
    ChannelSliceRepresentationSPtr channelSlice = boost::dynamic_pointer_cast<ChannelSliceRepresentation>(rep);
    SegmentationSliceRepresentationSPtr segmentationSlice = boost::dynamic_pointer_cast<SegmentationSliceRepresentation>(rep);
    if (channelSlice.get() != NULL || segmentationSlice.get() != NULL)
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
  void SliceRenderer::removeRepresentation(GraphicalRepresentationSPtr rep)
  {
    ChannelSliceRepresentationSPtr channelSlice = boost::dynamic_pointer_cast<ChannelSliceRepresentation>(rep);
    SegmentationSliceRepresentationSPtr segmentationSlice = boost::dynamic_pointer_cast<SegmentationSliceRepresentation>(rep);
    if (channelSlice.get() != NULL || segmentationSlice.get() != NULL)
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
  bool SliceRenderer::managesRepresentation(GraphicalRepresentationSPtr rep)
  {
    ChannelSliceRepresentationSPtr channelSlice = boost::dynamic_pointer_cast<ChannelSliceRepresentation>(rep);
    SegmentationSliceRepresentationSPtr segmentationSlice = boost::dynamic_pointer_cast<SegmentationSliceRepresentation>(rep);
    return (channelSlice.get() != NULL || segmentationSlice.get() != NULL);
  }

  //-----------------------------------------------------------------------------
  bool SliceRenderer::hasRepresentation(GraphicalRepresentationSPtr rep)
  {
    foreach (PickableItemPtr item, m_representations.keys())
      if (m_representations[item].contains(rep))
        return true;

    return false;
  }

  //-----------------------------------------------------------------------------
  void SliceRenderer::hide()
  {
    if (!m_enable)
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
  void SliceRenderer::show()
  {
     if (m_enable)
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
  unsigned int SliceRenderer::getNumberOfvtkActors()
  {
    unsigned int returnVal = 0;
    foreach (PickableItemPtr item, m_representations.keys())
      foreach(GraphicalRepresentationSPtr rep, m_representations[item])
        if (rep->isVisible()) ++returnVal;

    return returnVal;
  }

  //-----------------------------------------------------------------------------
  ViewManager::Selection SliceRenderer::pick(int x, int y, vtkSmartPointer<vtkRenderer> renderer, RenderabledItems itemType, bool repeat)
  {
    ViewManager::Selection selection;
    QList<vtkProp *> removedProps;

    if (!renderer || !renderer.GetPointer() || (!itemType.testFlag(EspINA::CHANNEL) && !itemType.testFlag(EspINA::SEGMENTATION)))
      return selection;

    while (m_picker->Pick(x, y, 0, renderer))
    {
      vtkProp *pickedProp = m_picker->GetViewProp();
      Q_ASSERT(pickedProp);

      m_picker->DeletePickList(pickedProp);
      removedProps << pickedProp;

      foreach(PickableItemPtr item, m_representations.keys())
      {
        if (!itemType.testFlag(item->type()))
        continue;

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

          break;
        }
      }
    }

    foreach(vtkProp *actor, removedProps)
      m_picker->AddPickList(actor);

    return selection;
  }

  //-----------------------------------------------------------------------------
  void SliceRenderer::getPickCoordinates(Nm *point)
  {
    m_picker->GetPickPosition(point);
  }


} /* namespace EspINA */
