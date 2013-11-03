/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#include "QComboTreeView.h"

#include <GUI/Utils/QtModelUtils.h>

//----------------------------------------------------------------------------
QComboTreeView::QComboTreeView(QWidget* parent)
: QComboBox(parent)
{
  setMinimumWidth(160);
  setSizeAdjustPolicy(QComboBox::AdjustToContents);

  m_treeView.setHeaderHidden(true);
  setView(&m_treeView); // Brutal!

  connect(&m_treeView, SIGNAL(entered(QModelIndex)),
          this, SLOT(indexEntered(QModelIndex)));

  connect(this, SIGNAL(currentIndexChanged(int)),
          this, SLOT(indexActivated()));
}

//----------------------------------------------------------------------------
void QComboTreeView::setModel(QAbstractItemModel* model)
{
  QComboBox::setModel(model);

  if (count() > 0)
  {
    setCurrentIndex(0);
    m_currentModelIndex = rootModelIndex().child(0,0);
    indexActivated();
  }
}


//----------------------------------------------------------------------------
void QComboTreeView::setRootModelIndex(const QModelIndex& index)
{
  QComboBox::setRootModelIndex(index);

  if (count() > 0)
  {
    setCurrentIndex(0);
    m_currentModelIndex = index.child(0,0);
    indexActivated();
  }
}

//----------------------------------------------------------------------------
void QComboTreeView::showPopup()
{
  m_treeView.expandAll();
  QComboBox::showPopup();
}

//----------------------------------------------------------------------------
void QComboTreeView::indexEntered(const QModelIndex& index)
{
  m_currentModelIndex = index;
}

//----------------------------------------------------------------------------
void QComboTreeView::indexActivated()
{
  QModelIndex index;
  if (count())
  {
    index = QtModelUtils::findChildIndex(rootModelIndex(), currentText());
    m_currentModelIndex = index;
  }

  if (index.isValid())
  {
    emit activated(index);
  }
}