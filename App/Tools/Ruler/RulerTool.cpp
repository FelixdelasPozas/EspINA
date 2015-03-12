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
#include "RulerTool.h"
#include <GUI/View/Widgets/Ruler/RulerWidget.h>
#include <Core/EspinaTypes.h>
#include <Core/Utils/Bounds.h>
#include <GUI/Model/ViewItemAdapter.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include "GUI/Model/ItemAdapter.h"

// VTK
#include <vtkMath.h>

// Qt
#include <QAction>

namespace ESPINA
{
  //----------------------------------------------------------------------------
  RulerTool::RulerTool(ViewManagerSPtr vm)
  : m_enabled    {false}
  , m_action     {new QAction(QIcon(":/espina/measure3D.png"), tr("Ruler Tool"),this) }
  , m_widget     {nullptr}
  , m_viewManager{vm}
  , m_selection  {new Selection()}
  {
    m_action->setCheckable(true);
    connect(m_action, SIGNAL(triggered(bool)), this, SLOT(initTool(bool)), Qt::QueuedConnection);
  }

  //----------------------------------------------------------------------------
  RulerTool::~RulerTool()
  {
    if(m_widget)
    {
      m_widget->setEnabled(false);
      m_widget = nullptr;
    }
  }

  //----------------------------------------------------------------------------
  void RulerTool::initTool(bool value)
  {
    if (value)
    {
      m_widget = EspinaWidgetSPtr(new RulerWidget());
      m_viewManager->addWidget(m_widget);
      connect(m_viewManager->selection().get(), SIGNAL(selectionStateChanged()),
              this,                             SLOT(selectionChanged()));
      selectionChanged();
    }
    else
    {
      m_viewManager->removeWidget(m_widget);
      disconnect(m_viewManager->selection().get(), SIGNAL(selectionStateChanged()),
                 this,                             SLOT(selectionChanged()));
      m_widget = nullptr;
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
  void RulerTool::selectionChanged()
  {
    if (!m_widget) return;

    auto selection = m_viewManager->selection();
    if (!m_selection->items().empty())
    {
      for(auto item: m_selection->items())
      {
        if (isSegmentation(item))
        {
          disconnect(item->output().get(), SIGNAL(modified()),
                     this,                 SLOT(selectionChanged()));
        }
      }
    }

    Bounds segmentationBounds, channelBounds;

    if (!selection->items().empty())
    {
      for(auto item: selection->items())
      {
        if (isSegmentation(item))
        {
          connect(item->output().get(), SIGNAL(modified()),
                  this,                 SLOT(selectionChanged()));
          if (segmentationBounds.areValid())
          {
            Bounds bounds = segmentationPtr(item)->bounds();
            for (int i = 0, j = 1; i < 6; i += 2, j += 2)
            {
              segmentationBounds[i] = std::min(bounds[i], segmentationBounds[i]);
              segmentationBounds[j] = std::max(bounds[j], segmentationBounds[j]);
            }
          }
          else
          {
            segmentationBounds = segmentationPtr(item)->bounds();
          }
        }
        else if (isChannel(item))
        {
          if (channelBounds.areValid())
          {
            Bounds bounds = channelPtr(item)->bounds();

            for (int i = 0, j = 1; i < 6; i += 2, j += 2)
            {
              channelBounds[i] = std::min(bounds[i], channelBounds[i]);
              channelBounds[j] = std::max(bounds[j], channelBounds[j]);
            }
          }
          else
          {
            channelBounds = channelPtr(item)->bounds();
          }
        }
      }
    }

    auto widget = dynamic_cast<RulerWidget *>(m_widget.get());
    Q_ASSERT(widget);

    if (segmentationBounds.areValid())
    {
      widget->setBounds(segmentationBounds);
    }
    else
    {
      if (channelBounds.areValid())
      {
        widget->setBounds(channelBounds);
      }
      else
      {
        widget->setBounds(segmentationBounds);
      }
    }

    m_selection = selection;
  }

  //----------------------------------------------------------------------------
  QList<QAction*> RulerTool::actions() const
  {
    QList<QAction *> actionList;
    actionList << m_action;

    return actionList;
  }

  //----------------------------------------------------------------------------
  bool RulerEventHandler::filterEvent(QEvent *e, RenderView *view)
  {
    // this is a passive tool
    return false;
  }
} /* namespace ESPINA */
