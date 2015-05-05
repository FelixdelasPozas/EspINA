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
HistoryDock::HistoryDock(FilterDelegateFactorySPtr delegateFactory,
                         Support::Context   &context)

: m_context(context)
, m_delegateFactory(delegateFactory)
, m_segmentation(nullptr)
{
  setObjectName(tr("History Panel"));

  setWindowTitle(tr("History"));

  connect(getSelection(m_context).get(), SIGNAL(selectionChanged()),
          this,                              SLOT(updateDock()));
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

  setWidget(new EmptyHistory());
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

  auto selection = getSelection(m_context)->segmentations();

  if (selection.size() == 1)
  {
    segmentation = selection.first();
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
        auto delegate = m_delegateFactory->createDelegate(m_segmentation);
        setWidget(delegate->createWidget(m_context));
      }
      catch (...)
      {
        setWidget(new DefaultHistory(m_segmentation));
      }
    }
    else
    {
      reset();
    }
  }
}

