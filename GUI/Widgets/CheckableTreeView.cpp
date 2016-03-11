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

// ESPINA
#include "CheckableTreeView.h"

// Qt
#include <QHeaderView>
#include <QMouseEvent>
#include <QDebug>

//------------------------------------------------------------------------
CheckableTreeView::CheckableTreeView(QWidget* parent)
: QTreeView{parent}
{
  setUniformRowHeights(true);
}

//------------------------------------------------------------------------
CheckableTreeView::~CheckableTreeView()
{
}

//------------------------------------------------------------------------
void CheckableTreeView::mouseReleaseEvent(QMouseEvent* event)
{
  QTreeView::mouseReleaseEvent(event);

  QModelIndex indexClicked = indexAt(event->pos());
  if(indexClicked.isValid())
  {
    QRect vrect = visualRect(indexClicked);
    int itemIndentation = vrect.x() - visualRect(rootIndex()).x();
    QRect rect = QRect(header()->sectionViewportPosition(0) + itemIndentation, vrect.y(), style()->pixelMetric(QStyle::PM_IndicatorWidth), vrect.height());
    if(rect.contains(event->pos()))
    {
      emit itemStateChanged(indexClicked);
    }
  }
}
