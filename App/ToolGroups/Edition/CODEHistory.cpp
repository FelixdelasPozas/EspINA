/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "CODEHistory.h"

#include "CODEHistoryWidget.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
CODEHistory::CODEHistory(const QString& title, MorphologicalEditionFilterSPtr filter)
: m_filter{filter}
, m_title{title}
{

}

//-----------------------------------------------------------------------------
QWidget *CODEHistory::createWidget(ModelAdapterSPtr model,
                                   ModelFactorySPtr factory,
                                   ViewManagerSPtr  viewManager,
                                   QUndoStack      *undoStack )
{
  auto widget = new CODEHistoryWidget(m_title, m_filter,viewManager, undoStack);

  connect(widget, SIGNAL(radiusChanged(int)),
          this,   SIGNAL(radiusChanged(int)));
  connect(this,   SIGNAL(radiusChanged(int)),
          widget, SLOT(setRadius(int)));

  return widget;
}
