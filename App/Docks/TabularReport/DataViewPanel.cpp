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

#include "TabularReport.h"
#include <Core/Model/Proxies/TaxonomyProxy.h>
#include <QTreeView>

#ifdef TEST_ESPINA_MODELS
#include <Core/Model/ModelTest.h>
#endif

using namespace EspINA;

//----------------------------------------------------------------------------
DataViewPanel::DataViewPanel(EspinaModel *model,
                             ViewManager *viewManager,
                             QWidget     *parent)
  : IDockWidget(parent)
  , m_informationProxy(new TaxonomicalInformationProxy())
{
  setObjectName("Data View Panel");

  setWindowTitle(tr("Segmentation Information"));

  m_informationProxy->setSourceModel(model);

  #ifdef TEST_ESPINA_MODELS
  m_modelTester = QSharedPointer<ModelTest>(new ModelTest(m_informationProxy.data()));
  #endif

  TabularReport *report = new TabularReport(model->factory(), viewManager);
  report->setModel(m_informationProxy.data());
  setWidget(report);
// //   QTreeView *tv = new QTreeView();
// //   tv->setModel(m_informationProxy.data());
// //   setWidget(tv);
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
