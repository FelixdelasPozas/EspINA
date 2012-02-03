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


#include "SegmentAction.h"

#include "../common/gui/ActionSelectorWidget.h"

#include <QDebug>

//------------------------------------------------------------------------
SegmentAction::SegmentAction(QObject* parent)
: QWidgetAction(parent)
{
}

//------------------------------------------------------------------------
QWidget* SegmentAction::createWidget(QWidget* parent)
{
  qDebug() << "Create Segment Widget";
  // Segmentation Button
  ActionSelectorWidget *segmentateButton = new ActionSelectorWidget(parent);
  segmentateButton->setIconSize(QSize(22,22));

  foreach(QAction *action, m_actions)
  {
    segmentateButton->addAction(action);
  }

  connect(segmentateButton, SIGNAL(actionTriggered(QAction*)),
	  this, SLOT(actionTriggered(QAction*)));
  connect(this, SIGNAL(cancelAction()),
	  segmentateButton, SLOT(cancelAction()));
  connect(segmentateButton, SIGNAL(actionCanceled()),
	  this, SLOT(onActionCanceled()));

  return segmentateButton;
}

//------------------------------------------------------------------------
void SegmentAction::addSelector(QAction* action)
{
  m_actions.append(action);
}


//------------------------------------------------------------------------
void SegmentAction::actionTriggered(QAction* action)
{
  qDebug() << action->text() << "has been triggered";
  emit triggered(action);
}

//------------------------------------------------------------------------
void SegmentAction::onActionCanceled()
{
  emit actionCanceled();
}
