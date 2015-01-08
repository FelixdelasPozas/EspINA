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
#include "SliceRenderer.h"
#include "GUI/Representations/SliceRepresentation.h"
#include "GUI/Representations/SliceCachedRepresentation.h"
#include <GUI/Selectors/Selector.h>
#include "GUI/View/RenderView.h"
#include "GUI/Model/ItemAdapter.h"
#include "GUI/View/View2D.h"

// VTK
#include <vtkPropPicker.h>

// Qt
#include <QApplication>
#include <QDebug>
#include <QMap>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  SliceRenderer::SliceRenderer(QObject *parent)
  : RepresentationRenderer{parent}
  , m_picker              {nullptr}
  {
  }

  //-----------------------------------------------------------------------------
  SliceRenderer::~SliceRenderer()
  {
    for (auto item: m_representations.keys())
    {
      if (m_enable)
      {
        for (auto rep: m_representations[item])
        {
          for (auto prop: rep->getActors())
          {
            m_view->removeActor(prop);
          }
        }
      }

      m_representations[item].clear();
    }
    m_representations.clear();

    if(m_picker != nullptr)
    {
      m_picker->GetPickList()->RemoveAllItems();
    }
  }

  //-----------------------------------------------------------------------------
  void SliceRenderer::setView(RenderView *view)
  {
    Renderer::setView(view);

    m_picker = vtkSmartPointer<vtkPropPicker>::New();
    m_picker->InitializePickList();
    m_picker->PickFromListOn();
  }

  //-----------------------------------------------------------------------------
  void SliceRenderer::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
  {
    auto segSlice     = std::dynamic_pointer_cast<SegmentationSliceRepresentation>(rep);
    auto channelSlice = std::dynamic_pointer_cast<ChannelSliceRepresentation>(rep);

    if ((segSlice.get() != nullptr) || (channelSlice.get() != nullptr))
    {
      if (m_representations.keys().contains(item))
      {
        m_representations[item] << rep;
      }
      else
      {
        RepresentationSList list;
        list << rep;
        m_representations.insert(item, list);
      }

      if (m_enable)
      {
        for (auto prop: rep->getActors())
        {
          m_view->addActor(prop);
          m_picker->AddPickList(prop);
        }
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
      for (auto item: m_representations.keys())
      {
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
  }

  //-----------------------------------------------------------------------------
  bool SliceRenderer::canRender(ItemAdapterPtr item) const
  {
    if(isChannel(item) || isSegmentation(item))
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
  bool SliceRenderer::managesRepresentation(const QString &repType) const
  {
    return ((repType == ChannelSliceRepresentation::TYPE) || (repType == SegmentationSliceRepresentation::TYPE));
  }

  //-----------------------------------------------------------------------------
  bool SliceRenderer::hasRepresentation(RepresentationSPtr rep) const
  {
    for (auto item: m_representations.keys())
    {
      if (m_representations[item].contains(rep)) return true;
    }

    return false;
  }

  //-----------------------------------------------------------------------------
  void SliceRenderer::hide()
  {
    if (!m_enable) return;

    for (auto item: m_representations.keys())
    {
      for (auto rep: m_representations[item])
      {
        for (auto prop: rep->getActors())
        {
          m_view->removeActor(prop);
          m_picker->DeletePickList(prop);
        }
      }
    }

    emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  void SliceRenderer::show()
  {
     if (m_enable) return;

     QApplication::setOverrideCursor(Qt::WaitCursor);
     for (auto item: m_representations.keys())
     {
       for (auto rep: m_representations[item])
       {
         for (auto prop: rep->getActors())
         {
           m_view->addActor(prop);
           m_picker->AddPickList(prop);
         }
       }
     }

     QApplication::restoreOverrideCursor();
     emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  unsigned int SliceRenderer::numberOfvtkActors() const
  {
    unsigned int returnVal = 0;
    for (auto item:  m_representations.keys())
    {
      for (auto rep: m_representations[item])
      {
        if (rep->isVisible()) ++returnVal;
      }
    }

    return returnVal;
  }

  //-----------------------------------------------------------------------------
  ViewItemAdapterList SliceRenderer::pick(int x, int y, Nm z,
                                          vtkSmartPointer<vtkRenderer> renderer,
                                          RenderableItems itemType,
                                          bool repeat)
  {
    ViewItemAdapterList selection;
    QList<vtkProp *> removedProps;
    auto view = reinterpret_cast<View2D *>(m_view);

    if (!renderer || !renderer.GetPointer() || (!itemType.testFlag(RenderableType::CHANNEL) && !itemType.testFlag(RenderableType::SEGMENTATION)))
    {
      return selection;
    }

    while (m_picker->Pick(x,y,0, renderer))
    {
      double point[3];
      m_picker->GetPickPosition(point);
      m_lastValidPickPosition = NmVector3{ point[0], point[1], point[2] };
      point[normalCoordinateIndex(view->plane())] = z;

      vtkProp *pickedProp = m_picker->GetViewProp();
      Q_ASSERT(pickedProp);

      m_picker->DeletePickList(pickedProp);
      removedProps << pickedProp;

      for (auto item: m_representations.keys())
      {
        if (!((isChannel(item) && itemType.testFlag(RenderableType::CHANNEL)) || (isSegmentation(item) && itemType.testFlag(RenderableType::SEGMENTATION))))
        {
          continue;
        }

        for (auto rep: m_representations[item])
        {
          NmVector3 vecPoint{ point[0], point[1], point[2] };
          if (rep->isVisible() && rep->hasActor(pickedProp) && rep->isInside(vecPoint) && !selection.contains(item))
          {
            selection << item;

            if (!repeat)
            {
              for (auto actor: removedProps)
              {
                m_picker->AddPickList(actor);
              }

              return selection;
            }

            break;
          }
        }
      }
    }

    for (auto actor: removedProps)
    {
      m_picker->AddPickList(actor);
    }

    return selection;
  }

} /* namespace ESPINA */
