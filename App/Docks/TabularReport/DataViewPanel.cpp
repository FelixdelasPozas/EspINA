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


#include "DataViewPanel.h"

#include "DataView.h"

using namespace EspINA;

//----------------------------------------------------------------------------
DataViewPanel::DataViewPanel(EspinaModel *model,
                             ViewManager *viewManager,
                             QWidget     *parent)
  : IDockWidget(parent)
{
  setObjectName("Data View Panel");

  setWindowTitle(tr("Segmentation Information"));

  setWidget(new DataView(model, viewManager));
}

//----------------------------------------------------------------------------
DataViewPanel::~DataViewPanel()
{

}

//----------------------------------------------------------------------------
void DataViewPanel::initDockWidget(EspinaModel *model,
                                   QUndoStack     *undoStack,
                                   ViewManager    *viewManager)
{

}

//----------------------------------------------------------------------------
void DataViewPanel::reset()
{

}
