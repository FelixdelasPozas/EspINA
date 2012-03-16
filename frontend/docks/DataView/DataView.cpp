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


#include "DataView.h"

#include <EspinaCore.h>
#include <QFileDialog>
#include "QueryView.h"

//------------------------------------------------------------------------
DataView::DataView(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, m_model(new InformationProxy())
{
  setupUi(this);

  EspinaModel *model = EspinaCore::instance()->model().data();
  m_model->setSourceModel(model);
  QStringList query;
  query << "Name" << "Size" << "Centroid X";
  m_model->setQuery(query);
  tableView->setModel(m_model.data());
//   tableView->setRootIndex(model->segmentationRoot());

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  writeDataToFile->setIcon(iconSave);
  connect(writeDataToFile, SIGNAL(clicked()),
	  this,SLOT(extractInformation()));
  connect(changeQuery, SIGNAL(clicked()),
	  this,SLOT(defineQuery()));
}

//------------------------------------------------------------------------
DataView::~DataView()
{

}

//------------------------------------------------------------------------
void DataView::defineQuery()
{
  QStringList query;
  query << "Name" << "Size" << "Centroid X";
  QueryView *querySelector = new QueryView(query, this);
  querySelector->exec();
  qDebug() << "New Query" << query;
  m_model->setQuery(query);
}

//------------------------------------------------------------------------
void DataView::extractInformation()
{
  QString title   = tr("Export Segmentation Data");
  QString fileExt = tr("CSV Text File (*.csv)");
  QString fileName = QFileDialog::getSaveFileName(this, title, "", fileExt);

  if (fileName.isEmpty())
    return;

  QFile file(fileName);
  file.open(QIODevice::WriteOnly |  QIODevice::Text);
  QTextStream out(&file);
//   out << EspINAFactory::instance()->segmentationAvailableInformations().join(",") << "\n";
//   for (int r = 0; r < m_espina->rowCount(m_espina->segmentationRoot()); r++)
//   {
//     for (int c = 0; c < m_espina->columnCount(m_espina->segmentationRoot()); c++)
//     {
//       if (c)
// 	out << ",";
//       out << m_espina->index(r,c,m_espina->segmentationRoot()).data().toString();
//     }
//     out << "\n";
//   }
  file.close();
}

