/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 F�lix de las Pozas �lvarez <felixdelaspozas@gmail.com>

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
#include <GUI/Selectors/Selector.h>
#include "GUI/View/RenderView.h"
#include "GUI/Model/ItemAdapter.h"
#include "GUI/View/SliceView.h"

// VTK
#include <vtkPropPicker.h>

// Qt
#include <QApplication>
#include <QDebug>
#include <QMap>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  SliceRenderer::SliceRenderer(QObject *parent)
  : Renderer(parent)
  , m_picker(vtkSmartPointer<vtkPropPicker>::New())
  {
    m_picker->PickFromListOn();
  }

  //-----------------------------------------------------------------------------
  SliceRenderer::~SliceRenderer()
  {
    foreach(ViewItemAdapterPtr item, m_representations.keys())
    {
      if (m_enable)
        foreach(RepresentationSPtr rep, m_representations[item])
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
  void SliceRenderer::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
  {
    SegmentationSliceRepresentationSPtr segSlice = std::dynamic_pointer_cast<SegmentationSliceRepresentation>(rep);
    ChannelSliceRepresentationSPtr channelSlice = std::dynamic_pointer_cast<ChannelSliceRepresentation>(rep);
    if ((segSlice.get() != nullptr) || (channelSlice.get() != nullptr))
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
        foreach(vtkProp* prop, rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
    }
  }

  //-----------------------------------------------------------------------------
  void SliceRenderer::removeRepresentation(RepresentationSPtr rep)
  {
    SegmentationSliceRepresentationSPtr segSlice = std::dynamic_pointer_cast<SegmentationSliceRepresentation>(rep);
    ChannelSliceRepresentationSPtr channelSlice = std::dynamic_pointer_cast<ChannelSliceRepresentation>(rep);

    if ((segSlice.get() != nullptr) || (channelSlice.get() != nullptr))
    {
      foreach(ViewItemAdapterPtr item, m_representations.keys())
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
  bool SliceRenderer::managesRepresentation(RepresentationSPtr rep)
  {
    SegmentationSliceRepresentationSPtr segSlice = std::dynamic_pointer_cast<SegmentationSliceRepresentation>(rep);
    ChannelSliceRepresentationSPtr channelSlice = std::dynamic_pointer_cast<ChannelSliceRepresentation>(rep);
    return ((segSlice.get() != nullptr) || (channelSlice.get() != nullptr));
  }

  //-----------------------------------------------------------------------------
  bool SliceRenderer::hasRepresentation(RepresentationSPtr rep)
  {
    foreach (ViewItemAdapterPtr item, m_representations.keys())
      if (m_representations[item].contains(rep))
        return true;

    return false;
  }

  //-----------------------------------------------------------------------------
  void SliceRenderer::hide()
  {
    if (!m_enable)
      return;

    foreach (ViewItemAdapterPtr item, m_representations.keys())
      foreach(RepresentationSPtr rep, m_representations[item])
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

     QApplication::setOverrideCursor(Qt::WaitCursor);
     foreach (ViewItemAdapterPtr item, m_representations.keys())
       foreach(RepresentationSPtr rep, m_representations[item])
         foreach(vtkProp* prop, rep->getActors())
         {
           m_view->addActor(prop);
           m_picker->AddPickList(prop);
         }

     QApplication::restoreOverrideCursor();
     emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  unsigned int SliceRenderer::numberOfvtkActors()
  {
    unsigned int returnVal = 0;
    foreach (ViewItemAdapterPtr item, m_representations.keys())
      foreach(RepresentationSPtr rep, m_representations[item])
        if (rep->isVisible()) ++returnVal;

    return returnVal;
  }

  //-----------------------------------------------------------------------------
  SelectableView::Selection SliceRenderer::pick(int x,
                                                int y,
                                                Nm z,
                                                vtkSmartPointer<vtkRenderer> renderer,
                                                RenderableItems itemType,
                                                bool repeat)
  {
    SelectableView::Selection selection;
    QList<vtkProp *> removedProps;
    SliceView *view = reinterpret_cast<SliceView *>(m_view);

    if (!renderer || !renderer.GetPointer() || (!itemType.testFlag(EspINA::CHANNEL) && !itemType.testFlag(EspINA::SEGMENTATION)))
      return selection;

    Nm pickPoint[3] = { static_cast<Nm>(x), static_cast<Nm>(y), ((view->plane() == Plane::XY) ? -SliceView::SEGMENTATION_SHIFT : SliceView::SEGMENTATION_SHIFT) };

    while (m_picker->Pick(pickPoint, renderer))
    {
      vtkProp *pickedProp = m_picker->GetViewProp();
      Q_ASSERT(pickedProp);

      Nm point[3];
      m_picker->GetPickPosition(point);
      point[normalCoordinateIndex(view->plane())] = z;

      m_picker->DeletePickList(pickedProp);
      removedProps << pickedProp;

      foreach(ViewItemAdapterPtr item, m_representations.keys())
      {
        if (!((item->type() == ViewItemAdapter::Type::CHANNEL && itemType.testFlag(RenderableType::CHANNEL)) ||
              (item->type() == ViewItemAdapter::Type::SEGMENTATION && itemType.testFlag(RenderableType::SEGMENTATION))))
          continue;

        foreach(RepresentationSPtr rep, m_representations[item])
        {
          NmVector3 vecPoint{ point[0], point[1], point[2] };
          if (rep->isVisible() && rep->hasActor(pickedProp) && rep->isInside(vecPoint) && !selection.contains(item))
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
    }

    foreach(vtkProp *actor, removedProps)
      m_picker->AddPickList(actor);

    return selection;
  }

  //-----------------------------------------------------------------------------
  NmVector3 SliceRenderer::pickCoordinates() const
  {
    Nm point[3];
    m_picker->GetPickPosition(point);

    return NmVector3{ point[0], point[1], point[2] };
  }


} /* namespace EspINA */
