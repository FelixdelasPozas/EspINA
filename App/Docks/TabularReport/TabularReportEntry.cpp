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
#include <GUI/Analysis/InformationSelector.h>
#include <QFileDialog>
#include <QStandardItemModel>

using namespace EspINA;

//------------------------------------------------------------------------
TabularReport::Entry::Entry(QString taxonomy, EspinaFactory *factory)
: QWidget()
, m_factory(factory)
, m_taxonomy(taxonomy)
{
  setupUi(this);
  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  writeDataToFile->setIcon(iconSave);
  connect(writeDataToFile, SIGNAL(clicked(bool)),
          this, SLOT(extractInformation()));
  connect(selectInformation, SIGNAL(clicked(bool)),
          this, SLOT(changeDisplayedInformation()));
}

//------------------------------------------------------------------------
void TabularReport::Entry::changeDisplayedInformation()
{
  QModelIndex rootIndex = tableView->rootIndex();

  InformationSelector::TaxonomyInformation tags;
  tags[m_taxonomy] = rootIndex.data(InformationTagsRole).toStringList();

  InformationSelector tagSelector(tags, m_factory, this);

  if (tagSelector.exec() == QDialog::Accepted)
  {
    m_tags.clear();
    m_tags << tr("Name") << tr("Taxonomy") << tags[m_taxonomy];

    tableView->model()->setData(rootIndex, m_tags, InformationTagsRole);

    QStandardItemModel *header = new QStandardItemModel(1, m_tags.size(), this);
    header->setHorizontalHeaderLabels(m_tags);
    tableView->horizontalHeader()->setModel(header);
  }
}

//------------------------------------------------------------------------
void TabularReport::Entry::extractInformation()
{
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  tr("Export %1 Data").arg(m_taxonomy),
                                                  QString("%1.csv").arg(m_taxonomy.replace("/","-")),
                                                  tr("CSV Text File (*.csv)"));

  if (fileName.isEmpty())
    return;

  QFile file(fileName);
  file.open(QIODevice::WriteOnly |  QIODevice::Text);
  QTextStream out(&file);

  QModelIndex rootIndex = tableView->rootIndex();

  out << rootIndex.data(InformationTagsRole).toStringList().join(",") << "\n";

  for (int r = 0; r < tableView->model()->rowCount(rootIndex); r++)
  {
    for (int c = 0; c < tableView->model()->columnCount(rootIndex); c++)
    {
      if (c)
        out << ",";
      out << tableView->model()->index(r,c, rootIndex).data().toString();
    }
    out << "\n";
  }
  file.close();
}
