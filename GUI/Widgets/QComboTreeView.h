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


#ifndef QCOMBOTREEVIEW_H
#define QCOMBOTREEVIEW_H

#include "EspinaGUI_Export.h"

#include <QComboBox>
#include <QTreeView>

class QTreeView;

class EspinaGUI_EXPORT QComboTreeView
: public QComboBox
{
  Q_OBJECT
public:
  explicit QComboTreeView(QWidget* parent = 0);

  void setModel(QAbstractItemModel *model);

  void setRootModelIndex( const QModelIndex &index);

  void setCurrentModelIndex(const QModelIndex &index);

  QModelIndex currentModelIndex() const {return m_currentModelIndex;}

  virtual void mousePressEvent(QMouseEvent* e);

protected:
  virtual void showPopup();

  //virtual void keyPressEvent(QKeyEvent* e){}
  //virtual void keyReleaseEvent(QKeyEvent* e){}
  //virtual void wheelEvent(QWheelEvent* e){}

private slots:
  void indexEntered(const QModelIndex &index);
  void indexActivated();

signals:
  void activated(const QModelIndex &index);

private:
  QModelIndex m_rootModelIndex;
  QModelIndex m_currentModelIndex;
  QTreeView   m_treeView;
};


#endif // QCOMBOTREEVIEW_H
