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
#include "QComboTreeView.h"
#include <GUI/Utils/QtModelUtils.h>

// Qt
#include <QLineEdit>

//----------------------------------------------------------------------------
QComboTreeView::QComboTreeView(QWidget* parent)
: QComboBox        {parent}
, m_treeView       {new QTreeView{this}}
, m_usePressedIndex{false}
{
  setMinimumWidth(160);
  setSizeAdjustPolicy(QComboBox::AdjustToContents);

  m_treeView->setHeaderHidden(true);

  connect(m_treeView, SIGNAL(entered(QModelIndex)),
          this,       SLOT(indexEntered(QModelIndex)));

  connect(this, SIGNAL(currentIndexChanged(int)),
          this, SLOT(indexActivated()));

  QComboBox::setView(m_treeView);
}

//----------------------------------------------------------------------------
QComboTreeView::~QComboTreeView()
{
}

//----------------------------------------------------------------------------
void QComboTreeView::mousePressEvent(QMouseEvent* e)
{
  setRootModelIndex(m_rootModelIndex);
  QComboBox::mousePressEvent(e);
}

//----------------------------------------------------------------------------
void QComboTreeView::setModel(QAbstractItemModel* model)
{
  if(model)
  {
    QComboBox::setModel(model);

    setRootModelIndex(rootModelIndex());
  }
  else
  {
    QComboBox::clear();
  }
}

//----------------------------------------------------------------------------
void QComboTreeView::setRootModelIndex(const QModelIndex& index)
{
  QComboBox::setRootModelIndex(index);

  m_rootModelIndex    = index;
  m_currentModelIndex = index;

  if (count() > 0)
  {
    setCurrentIndex(0);
    m_currentModelIndex = index.child(0,0);
    indexActivated();
  }
}

//----------------------------------------------------------------------------
void QComboTreeView::setCurrentModelIndex(const QModelIndex& index)
{
  setRootModelIndex(index.parent());
  setCurrentIndex(index.row());

  m_currentModelIndex = index;
}

//----------------------------------------------------------------------------
void QComboTreeView::showPopup()
{
  m_treeView->expandAll();
  adjustSize();
  QComboBox::showPopup();
}

//----------------------------------------------------------------------------
void QComboTreeView::indexEntered(const QModelIndex& index)
{
  m_currentModelIndex = index;
  m_usePressedIndex   = true;
}

//----------------------------------------------------------------------------
void QComboTreeView::indexActivated()
{
  if (!m_usePressedIndex)
  {
    m_currentModelIndex = QtModelUtils::findChildIndex(rootModelIndex(), currentText(), false);
  }

  m_usePressedIndex = false;

  if (m_currentModelIndex.isValid() && m_currentModelIndex != m_rootModelIndex)
  {
    emit activated(m_currentModelIndex);
  }
}
