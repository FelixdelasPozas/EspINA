/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef QTREECOMBOBOX_H
#define QTREECOMBOBOX_H

#include <QComboBox>
#include <QTreeView>

class QTreeComboBox : public QComboBox
{
public:
  QTreeComboBox(QWidget* parent = 0) : QComboBox(parent), skipNextHide(false)
  {
    QTreeView *treeview = new QTreeView(this);
    treeview->setHeaderHidden(true);
    setView(treeview);
    view()->viewport()->installEventFilter(this);
  }

  bool eventFilter(QObject* object, QEvent* event);

  virtual void showPopup()
  {
    //setRootModelIndex(rootModelIndex());
    view()->setMinimumWidth(50);
    QComboBox::showPopup();
  }

  virtual void hidePopup()
  {
    //setRootModelIndex(view()->currentIndex().parent());
    setCurrentIndex(view()->currentIndex().row());
    if (skipNextHide)
      skipNextHide = false;
    else
      QComboBox::hidePopup();
  }

private:
  bool skipNextHide;
};

#endif // QTREECOMBOBOX_H
