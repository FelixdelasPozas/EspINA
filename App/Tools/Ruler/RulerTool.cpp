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
#include "RulerTool.h"
#include <GUI/vtkWidgets/RulerWidget.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/PickableItem.h>
#include <Core/Model/Channel.h>
#include <Core/Model/Segmentation.h>
#include <Core/OutputRepresentations/VolumeRepresentation.h>

// VTK
#include <vtkMath.h>

namespace EspINA
{
  //----------------------------------------------------------------------------
  RulerTool::RulerTool(ViewManager *vm)
  : m_enabled(false)
  , m_inUse(false)
  , m_widget(NULL)
  , m_viewManager(vm)
  , m_selection(ViewManager::Selection())
  {
  }
  
  //----------------------------------------------------------------------------
  RulerTool::~RulerTool()
  {
    if(m_widget)
    {
      m_widget->setEnabled(false);
      delete m_widget;
      m_widget = NULL;
    }
  }

  //----------------------------------------------------------------------------
  bool RulerTool::filterEvent(QEvent *e, EspinaRenderView *view)
  {
    // this is a passive tool
    return false;
  }

  //----------------------------------------------------------------------------
  void RulerTool::setInUse(bool value)
  {
    if(m_inUse == value)
      return;

    m_inUse = value;

    if (m_inUse)
    {
      m_widget = new RulerWidget();
      m_viewManager->addWidget(m_widget);
      connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
              this,          SLOT(selectionChanged(ViewManager::Selection, bool)));
      selectionChanged(m_viewManager->selection(), false);
    }
    else
    {
      m_viewManager->removeWidget(m_widget);
      disconnect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection, bool)),
                 this,          SLOT(selectionChanged(ViewManager::Selection, bool)));
      delete m_widget;
      m_widget = NULL;
    }
  }

  //----------------------------------------------------------------------------
  void RulerTool::setEnabled(bool value)
  {
    m_enabled = value;
  }

  //----------------------------------------------------------------------------
  bool RulerTool::enabled() const
  {
    return m_enabled;
  }

  //----------------------------------------------------------------------------
  void RulerTool::selectionChanged(ViewManager::Selection selection, bool unused)
  {
    if (!m_selection.isEmpty())
    {
      foreach(PickableItemPtr item, m_selection)
      {
        if (item->type() == EspINA::SEGMENTATION)
        {
          disconnect(item->output().get(), SIGNAL(modified()),
                     this,                 SLOT(selectedElementChanged()));
        }
      }
    }

    Nm segmentationBounds[6], channelBounds[6];
    vtkMath::UninitializeBounds(segmentationBounds);
    vtkMath::UninitializeBounds(channelBounds);

    if (!selection.isEmpty())
    {
      foreach(PickableItemPtr item, selection)
      {
        switch(item->type())
        {
          case EspINA::CHANNEL:
            if (!vtkMath::AreBoundsInitialized(channelBounds))
              channelPtr(item)->volume()->bounds(channelBounds);
            else
            {
              Nm bounds[6];
              channelPtr(item)->volume()->bounds(bounds);
              for (int i = 0, j = 1; i < 6; i += 2, j += 2)
              {
                channelBounds[i] = std::min(bounds[i], channelBounds[i]);
                channelBounds[j] = std::max(bounds[j], channelBounds[j]);
              }
            }
            break;
          case EspINA::SEGMENTATION:
          {
            connect(item->output().get(), SIGNAL(modified()),
                    this,                 SLOT(selectedElementChanged()));
            if (!vtkMath::AreBoundsInitialized(segmentationBounds))
              segmentationVolume(segmentationPtr(item)->output())->bounds(segmentationBounds);
            else
            {
              Nm bounds[6];
              segmentationVolume(segmentationPtr(item)->output())->bounds(bounds);
              for (int i = 0, j = 1; i < 6; i += 2, j += 2)
              {
                segmentationBounds[i] = std::min(bounds[i], segmentationBounds[i]);
                segmentationBounds[j] = std::max(bounds[j], segmentationBounds[j]);
              }
            }
            break;
          }
          default:
            Q_ASSERT(false);
            break;
        }
      }
    }

    if (vtkMath::AreBoundsInitialized(segmentationBounds))
      m_widget->setBounds(segmentationBounds);
    else
      if (vtkMath::AreBoundsInitialized(channelBounds))
        m_widget->setBounds(channelBounds);
      else
        m_widget->setBounds(segmentationBounds);

    m_selection = selection;
  }

  //----------------------------------------------------------------------------
  void RulerTool::selectedElementChanged()
  {
    selectionChanged(m_selection, false);
  }

} /* namespace EspINA */
