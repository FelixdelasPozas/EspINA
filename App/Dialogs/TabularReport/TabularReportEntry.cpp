/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "TabularReportEntry.h"
#include <GUI/Analysis/xlsUtils.h>
#include <GUI/Analysis/InformationSelector.h>
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMessageBox>

using namespace EspINA;
using namespace xlslib_core;

//------------------------------------------------------------------------
TabularReport::Entry::Entry(QString taxonomy, EspinaFactory *factory)
: QWidget()
, m_factory(factory)
, m_taxonomy(taxonomy)
{
  setupUi(this);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  writeDataToFile->setIcon(iconSave);
  connect(writeDataToFile, SIGNAL(clicked(bool)),
          this, SLOT(extractInformation()));
  connect(selectInformation, SIGNAL(clicked(bool)),
          this, SLOT(changeDisplayedInformation()));

  tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
}

//------------------------------------------------------------------------
TabularReport::Entry::~Entry()
{
  if (Proxy) delete Proxy;
}

//------------------------------------------------------------------------
int TabularReport::Entry::rowCount() const
{
  return tableView->model()->rowCount(tableView->rootIndex()) + 1;
}

//------------------------------------------------------------------------
int TabularReport::Entry::columnCount() const
{
  return Proxy->informationTags().size();
}

//------------------------------------------------------------------------
QVariant TabularReport::Entry::value(int row, int column) const
{
  QVariant result;

  if (row < rowCount() && column < columnCount())
  {
    if (row == 0)
      result = Proxy->informationTags()[column];
    else
      result = tableView->model()->index(row - 1, column, tableView->rootIndex()).data();
  }

  return result;
}


//------------------------------------------------------------------------
void TabularReport::Entry::changeDisplayedInformation()
{
  InformationSelector::TaxonomyInformation tags;
  tags[m_taxonomy] = Proxy->informationTags();

  InformationSelector tagSelector(tags, m_factory, this);

  if (tagSelector.exec() == QDialog::Accepted)
  {
    m_tags.clear();
    m_tags << tr("Name") << tr("Taxonomy") << tags[m_taxonomy];

    Proxy->setInformationTags(m_tags);

    QStandardItemModel *header = new QStandardItemModel(1, m_tags.size(), this);
    header->setHorizontalHeaderLabels(m_tags);
    tableView->horizontalHeader()->setModel(header);
  }
}

//------------------------------------------------------------------------
void TabularReport::Entry::extractInformation()
{
  QString filter = tr("Excel File (*.xls)") + ";;" + tr("CSV Text File (*.csv)");
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  tr("Export %1 Data").arg(m_taxonomy),
                                                  QString("%1.xls").arg(m_taxonomy.replace("/","-")),
                                                  filter);

  if (fileName.isEmpty())
    return;

  bool result = false;

  if (fileName.endsWith(".csv"))
  {
    result = exportToCSV(fileName);
  }
  else if (fileName.endsWith(".xls"))
  {
    result = exportToXLS(fileName);
  }

  if (!result)
    QMessageBox::warning(this, "EspINA", tr("Couldn't export %1").arg(fileName));
}

//------------------------------------------------------------------------
bool TabularReport::Entry::exportToCSV(const QString &filename)
{
  QFile file(filename);

  file.open(QIODevice::WriteOnly |  QIODevice::Text);

  QTextStream out(&file);

  for (int r = 0; r < rowCount(); r++)
  {
    for (int c = 0; c < columnCount(); c++)
    {
      if (c)
        out << ",";
      out << value(r, c).toString();
    }
    out << "\n";
  }
  file.close();

  return true;
}

//------------------------------------------------------------------------
bool TabularReport::Entry::exportToXLS(const QString &filename)
{
  workbook wb;

  worksheet *ws = wb.sheet(m_taxonomy.toStdString());

  for (int r = 0; r < rowCount(); ++r)
  {
    for (int c = 0; c < columnCount(); ++c)
    {
      createCell(ws, r, c, value(r,c));
    }
  }

  wb.Dump(filename.toStdString());

  return true;
}