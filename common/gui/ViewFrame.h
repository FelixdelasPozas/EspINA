/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


//----------------------------------------------------------------------------
// File:    ViewFrame.h
// Purpose: Contain various views and re-arrange them.
//----------------------------------------------------------------------------
#ifndef MULTIVIEWFRAME_H
#define MULTIVIEWFRAME_H

#include <QSplitter>

class pqView;
class SliceView;
class MultiViewFrame : public QWidget
{
public:
  explicit MultiViewFrame(QWidget *mainView, QWidget *parent=NULL);
  ~MultiViewFrame();

private:
  QList<pqView*> m_views;
};

#endif // MULTIVIEWFRAME_H
