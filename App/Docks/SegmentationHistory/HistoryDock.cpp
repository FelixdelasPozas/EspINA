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
#include "HistoryDock.h"
#include "DefaultHistory.h"
#include "EmptyHistory.h"
#include <Support/FilterHistory.h>

#include <QDebug>
#include <QLayout>
#include <QLabel>

using namespace ESPINA;

//----------------------------------------------------------------------------
HistoryDock::HistoryDock(ModelAdapterSPtr          model,
                         FilterDelegateFactorySPtr factory,
                         ViewManagerSPtr           viewManager,
                         QUndoStack               *undoStack,
                         QWidget                  *parent)
: DockWidget(parent)
, m_baseModel(model)
, m_factory(factory)
, m_viewManager(viewManager)
, m_undoStack(undoStack)
, m_segmentation(nullptr)
{
  setObjectName(tr("History Panel"));

  setWindowTitle(tr("History"));

  connect(m_viewManager->selection().get(), SIGNAL(selectionChanged()),
          this,                             SLOT(updateDock()));
}

//----------------------------------------------------------------------------
HistoryDock::~HistoryDock()
{
}

//----------------------------------------------------------------------------
void HistoryDock::reset()
{
  m_filter.reset();
  m_segmentation = nullptr;

  updateDock();
}

//----------------------------------------------------------------------------
void HistoryDock::showEvent(QShowEvent *e)
{
  QWidget::showEvent(e);

  updateDock();
}

//----------------------------------------------------------------------------
void HistoryDock::updateDock()
{
  if (!isVisible())
    return;

  SegmentationAdapterPtr segmentation = nullptr;
  bool changeWidget = false;

  auto selectedSegmentations = m_viewManager->selection()->segmentations();

  if (selectedSegmentations.size() == 1)
  {
    segmentation = selectedSegmentations.first();
  }

  // Update if segmentation are different
  if (segmentation != m_segmentation)
  {
    if (m_segmentation)
    {
      disconnect(m_segmentation, SIGNAL(outputModified()),
                 this,           SLOT(updateDock()));
    }

    m_segmentation = segmentation;

    if (m_segmentation)
    {
      connect(m_segmentation, SIGNAL(outputModified()),
                 this,        SLOT(updateDock()));
    }

    changeWidget = true;
  }
  else if ((m_filter && m_filter != segmentation->filter())
    ||     (m_segmentation == nullptr && segmentation == nullptr && widget() == nullptr))
  {
    changeWidget = true;
  }

  if (changeWidget)
  {
    auto prevWidget = widget();

    if (prevWidget)
    {
      delete prevWidget;
    }

    if (m_segmentation)
    {
      m_filter = segmentation->filter();

      try
      {
        auto delegate = m_factory->createDelegate(m_segmentation);
        setWidget(delegate->createWidget(m_viewManager, m_undoStack));
      }
      catch (...)
      {
        setWidget(new DefaultHistory(m_segmentation));
      }
    }
    else
    {
      m_filter.reset();
      m_segmentation = nullptr;

      setWidget(new EmptyHistory());
    }
  }
}

