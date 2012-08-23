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


#ifndef MODIFYFILTERPANEL_H
#define MODIFYFILTERPANEL_H

#include "common/gui/EspinaDockWidget.h"

#include "common/model/EspinaModel.h"

class Segmentation;
class ModifyFilterPanel
: public EspinaDockWidget
{
  Q_OBJECT
public:
  explicit ModifyFilterPanel(QWidget* parent = 0);
  virtual ~ModifyFilterPanel();

  virtual void showEvent(QShowEvent* e);

protected slots:
  void updatePannel();

private:
  Filter       *m_filter;
  Segmentation *m_seg;
};

#endif // MODIFYFILTERPANEL_H
