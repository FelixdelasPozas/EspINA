/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "FilterInspectorDock.h"

#include <Core/Model/Segmentation.h>
#include <GUI/ViewManager.h>

#include <QDebug>
#include <QLayout>
#include <QLabel>

using namespace EspINA;

//----------------------------------------------------------------------------
FilterInspectorDock::FilterInspectorDock(QUndoStack *undoStack,
                                 ViewManager* vm,
                                 QWidget* parent)
: IDockWidget(parent)
, m_undoStack(undoStack)
, m_viewManager(vm)
, m_filter(NULL)
, m_seg   (NULL)
{
  setObjectName("Filter Inspector Panel");

  setWindowTitle(tr("Filter Inspector"));

  connect(m_viewManager, SIGNAL(selectionChanged(ViewManager::Selection,bool)),
          this, SLOT(updatePannel()));
}

//----------------------------------------------------------------------------
FilterInspectorDock::~FilterInspectorDock()
{
}

//----------------------------------------------------------------------------
void FilterInspectorDock::initDockWidget(EspinaModel *model,
                                     QUndoStack  *undoStack,
                                     ViewManager *viewManager)
{
  updatePannel();
}

//----------------------------------------------------------------------------
void FilterInspectorDock::reset()
{

}

//----------------------------------------------------------------------------
void FilterInspectorDock::showEvent(QShowEvent *e)
{
  QWidget::showEvent(e);
  updatePannel();
}

//----------------------------------------------------------------------------
void FilterInspectorDock::updatePannel()
{
  if (!isVisible())
    return;

  SegmentationPtr seg = NULL;
  bool changeWidget = false;

  SegmentationList selectedSegs;
  foreach (PickableItemPtr item, m_viewManager->selection())
  {
    if (EspINA::SEGMENTATION == item->type())
      selectedSegs << segmentationPtr(item);
  }

  if (selectedSegs.size() == 1)
    seg = selectedSegs[0];

  // Update if segmentation are different
  if (seg != m_seg)
  {
    if (m_seg)
    {
      disconnect(m_seg, SIGNAL(outputModified()),
                 this, SLOT(updatePannel()));
    }

    m_seg = seg;

    if (m_seg)
    {
      connect(m_seg, SIGNAL(outputModified()),
              this, SLOT(updatePannel()));
    }
    changeWidget = true;
  }
  else if ((!m_filter.isNull() && m_filter != seg->filter())|| (m_seg == NULL && seg == NULL && widget() == NULL))
  {
    changeWidget = true;
  }

  if (changeWidget)
  {
    QWidget *prevWidget = widget();
    if (prevWidget)
      delete prevWidget;

    if (m_seg)
    {
      m_filter = seg->filter();
      Filter::FilterInspectorPtr inspector = m_filter->filterInspector();
      if (inspector.get())
        setWidget(inspector->createWidget(m_undoStack, m_viewManager));
      else
      {
        QWidget *defaultWidgetInspector = new QWidget();
        defaultWidgetInspector->setLayout(new QVBoxLayout());
        defaultWidgetInspector->layout()->setSpacing(10);

        QLabel *typeLabel = new QLabel(defaultWidgetInspector);
        typeLabel->setText(QString("<b>Segmentation: </b>") + m_seg->data().toString());
        typeLabel->setWordWrap(true);
        typeLabel->setTextInteractionFlags(Qt::NoTextInteraction);
        defaultWidgetInspector->layout()->addWidget(typeLabel);

        QLabel *infoLabel = new QLabel(defaultWidgetInspector);
        infoLabel->setText(QLabel::tr("Segmentation cannot be modified."));
        infoLabel->setWordWrap(true);
        infoLabel->setTextInteractionFlags(Qt::NoTextInteraction);
        defaultWidgetInspector->layout()->addWidget(infoLabel);

        QSpacerItem* spacer = new QSpacerItem(-1, -1, QSizePolicy::Minimum, QSizePolicy::Expanding);
        defaultWidgetInspector->layout()->addItem(spacer);

        setWidget(defaultWidgetInspector);
      }
    }
    else
    {
      m_filter.clear();
      m_seg = NULL;

      QWidget *noWidgetInspector = new QWidget();
      noWidgetInspector->setLayout(new QVBoxLayout());
      noWidgetInspector->layout()->setSpacing(10);

      QLabel *infoLabel = new QLabel(noWidgetInspector);
      infoLabel->setText(QLabel::tr("No segmentation selected."));
      infoLabel->setWordWrap(true);
      infoLabel->setTextInteractionFlags(Qt::NoTextInteraction);
      noWidgetInspector->layout()->addWidget(infoLabel);

      QSpacerItem* spacer = new QSpacerItem(-1, -1, QSizePolicy::Minimum, QSizePolicy::Expanding);
      noWidgetInspector->layout()->addItem(spacer);

      setWidget(noWidgetInspector);
    }
  }
}

