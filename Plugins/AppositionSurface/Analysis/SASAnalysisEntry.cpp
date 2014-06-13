/*
    Copyright (c) 2013, <copyright holder> <email>
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

    THIS SOFTWARE IS PROVIDED BY <copyright holder> <email> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL <copyright holder> <email> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// Plugin
#include "SASAnalysisEntry.h"

// EspINA
#include <Filter/AppositionSurfaceFilter.h>
#include <Core/Extensions/AppositionSurfaceExtension.h>
#include <Core/Factory/CoreFactory.h>
#include <Core/Analysis/Extension.h>
#include <Extensions/Morphological/MorphologicalInformation.h>
#include <Support/Utils/xlsUtils.h>

// Qt
#include <QFileDialog>
#include <QScrollBar>
#include <QMessageBox>
#include <QFile>

using namespace EspINA;
using namespace xlslib_core;

//class SegmentationItem
//: public QTableWidgetItem
//{
//public:
//  SegmentationItem(int number)
//  : m_number(number) {}
//
//  virtual bool operator<(const QTableWidgetItem& other) const
//  {
//    const SegmentationItem *base = dynamic_cast<const SegmentationItem *>(&other);
//    return m_number < base->m_number;
//  }
//
//private:
//  int m_number;
//};

//----------------------------------------------------------------------------
SASAnalysisEntry::SASAnalysisEntry(SegmentationAdapterList segmentations,
                                   ModelAdapterSPtr        model,
                                   ModelFactorySPtr        factory,
                                   QWidget                *parent)
: QWidget(parent)
, m_model      {model}
, m_factory    {factory}
, m_synapses   {segmentations}
{
  setupUi(this);

  if (!segmentations.isEmpty())
  {
    m_title = segmentations.first()->category()->classificationName();
  }

  dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);

  connect(dataTable->horizontalScrollBar(), SIGNAL(valueChanged(int)),
          analysisTable->horizontalScrollBar(), SLOT(setValue(int)));

  connect(analysisTable->horizontalScrollBar(), SIGNAL(valueChanged(int)), 
          dataTable->horizontalScrollBar(), SLOT(setValue(int)));

  connect(dataTable, SIGNAL(cellChanged(int,int)),
          this, SLOT(syncGeometry()));

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  writeDataToFile->setIcon(iconSave);
  connect(writeDataToFile, SIGNAL(clicked(bool)),
          this, SLOT(saveAnalysis()));

  connect(changeQuery, SIGNAL(clicked(bool)),
          this, SLOT(defineQuery()));
}

//----------------------------------------------------------------------------
void SASAnalysisEntry::displayInformation()
{
  //progressBar->setVisible(true);
  int synapseColumns = m_synapseTags.size();
  int sasColumns     = m_sasTags.size();
  int rowCount       = m_synapses.size();

  dataTable->clearContents();
  dataTable->setRowCount(rowCount);
  dataTable->setColumnCount(synapseColumns + sasColumns);

  analysisTable->clearContents();
  analysisTable->setRowCount(1);
  analysisTable->setColumnCount(synapseColumns + sasColumns);

  QStringList headers;
  headers << m_synapseTags;
  for(auto sasHeader: m_sasTags)
  {
    headers << tr("SAS %1").arg(sasHeader);
  }
  dataTable->setHorizontalHeaderLabels(headers);
  analysisTable->setHorizontalHeaderLabels(headers);

  analysisTable->setVisible(false);

  //progressBar->setMinimum(-1);
  //progressBar->setMaximum(rowCount - 1);

  for (int r = 0; r < rowCount; ++r)
  {
    SegmentationAdapterPtr segmentation = m_synapses[r];

    // create morphological information extension if not present
    bool found = false;
    for(auto extension: segmentation->extensions())
      if(extension->type() == MorphologicalInformation::TYPE)
      {
        found = true;
      }

    if(!found)
      segmentation->addExtension(m_factory->createSegmentationExtension(MorphologicalInformation::TYPE));

    for(int c = 0; c < synapseColumns; ++c)
    {
      SegmentationExtension::InfoTag tag = m_synapseTags[c];
      QVariant cell;
      if(tag == tr("Name"))
        cell = segmentation->data().toString();
      else if (tag == tr("Category"))
        cell = segmentation->category()->name();
      else
        cell = segmentation->information(tag);

      QTableWidgetItem *item = new QTableWidgetItem();
      item->setData(Qt::DisplayRole, cell);
      dataTable->setItem(r, c, item);

      if (r == 0)
      {
        analysisTable->setItem(0, c, new QTableWidgetItem(""));
      }

      if (QVariant::Double == cell.type() || QVariant::Int == cell.type())
      {
        item->setData(Qt::TextAlignmentRole, Qt::AlignCenter);

        double total     = analysisTable->item(0, c)->data(Qt::DisplayRole).toDouble();
        double cellValue = cell.toDouble();

        analysisTable->item(0, c)->setData(Qt::DisplayRole, total + cellValue);
      }
    }

    auto relatedItems = m_model->relatedItems(segmentation, RelationType::RELATION_OUT, SAS);
    Q_ASSERT(relatedItems.size() >= 1);

    if (relatedItems.size() > 1)
    {
      qWarning() << segmentation->data().toString() <<  "has several SAS segmentations associated";
    }

    SegmentationAdapterSPtr sas = std::dynamic_pointer_cast<SegmentationAdapter>(relatedItems.first());

    for (int c = 0; c < sasColumns; ++c)
    {
      QVariant cell = sas->information(m_sasTags[c]);
      QTableWidgetItem *item = new QTableWidgetItem();
      item->setData(Qt::DisplayRole, cell);
      dataTable->setItem(r, synapseColumns + c, item);

      int ac = c + synapseColumns;
      if (r == 0)
      {
        analysisTable->setItem(0, ac, new QTableWidgetItem("0"));
      }

      if (QVariant::Double == cell.type() || QVariant::Int == cell.type())
      {
        item->setData(Qt::TextAlignmentRole, Qt::AlignCenter);

        double total     = analysisTable->item(0, ac)->data(Qt::DisplayRole).toDouble();
        double cellValue = cell.toDouble();

        analysisTable->item(0, ac)->setData(Qt::DisplayRole, total + cellValue);
      }
    }

    dataTable->item(r, 0)->setData(Qt::DecorationRole, segmentation->data(Qt::DecorationRole));

    //progressBar->setValue(r);
  }

  for (int c = 0; c < synapseColumns + sasColumns; ++c)
  {
    if (analysisTable->item(0, c))
    {
      QVariant cell = analysisTable->item(0, c)->data(Qt::DisplayRole);

      if (QVariant::Double == cell.type() || QVariant::Int == cell.type())
      {
        analysisTable->item(0, c)->setData(Qt::DisplayRole, cell.toDouble() / rowCount);
        analysisTable->item(0, c)->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
      }
    }
  }

  dataTable->resizeColumnsToContents();
  dataTable->setSortingEnabled(true);
  dataTable->horizontalHeader()->setMovable(true);
  analysisTable->horizontalHeader()->setMovable(true);

  syncGeometry();

  //progressBar->setVisible(false);
  dataTable->sortByColumn(0, Qt::AscendingOrder);
}

//----------------------------------------------------------------------------
bool SASAnalysisEntry::exportToXLS(xlslib_core::workbook& wb)
{
  worksheet *sheet = wb.sheet(m_title.replace("/",">").toStdString());

  int sheetRow = 0;
  // Headers
  for (int c = 0; c < dataTable->columnCount(); ++c)
  {
    createCell(sheet, sheetRow, c, dataTable->horizontalHeaderItem(c)->data(Qt::DisplayRole));
  }

  ++sheetRow;

  for (int r = 0; r < dataTable->rowCount(); ++r, ++sheetRow)
  {
    for (int c = 0; c < dataTable->columnCount(); ++c)
    {
      createCell(sheet, sheetRow, c, dataTable->item(r,c)->data(Qt::DisplayRole));
    }
  }

  ++sheetRow;

//   for (int r = 0; r < analysisTable->rowCount(); ++r, ++sheetRow)
//   {
//     for (int c = 0; c < analysisTable->columnCount(); ++c)
//     {
//       createCell(sheet, sheetRow, c, analysisTable->item(r,c)->data(Qt::DisplayRole));
//     }
//   }

  return true;
}

//----------------------------------------------------------------------------
void SASAnalysisEntry::defineQuery(InformationSelector::GroupedInfo tags)
{
  const QString SYNAPSE = tr("Synapse");
  const QString SAS     = tr("SAS");

  m_synapseTags.clear();
  m_synapseTags << tags[SYNAPSE];

  m_sasTags.clear();
  m_sasTags << tags[SAS];

  displayInformation();
}

//----------------------------------------------------------------------------
void SASAnalysisEntry::defineQuery()
{
  const QString SYNAPSE = tr("Synapse");
  const QString SAS     = tr("SAS");

  Q_ASSERT(m_factory->availableSegmentationExtensions().contains(MorphologicalInformation::TYPE));

  InformationSelector::GroupedInfo tags, selection;
  tags[SYNAPSE]      = m_factory->createSegmentationExtension(MorphologicalInformation::TYPE)->availableInformations();
  tags[SAS]          = m_factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE)->availableInformations();
  selection[SYNAPSE] = m_synapseTags;
  selection[SAS]     = m_sasTags;

  InformationSelector tagSelector(tags, selection, this);

  if (tagSelector.exec() == QDialog::Accepted)
  {
    selection[SYNAPSE].prepend(tr("Category"));
    selection[SYNAPSE].prepend(tr("Name"));

    defineQuery(selection);
  }
}

//----------------------------------------------------------------------------
void SASAnalysisEntry::saveAnalysis()
{
  QString title   = tr("Export Synaptic Apposition Surface Analysis");
  QString filter = tr("Excel File (*.xls)") + ";;" + tr("CSV Text File (*.csv)");
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  title,
                                                  "",
                                                  filter);

  if (fileName.isEmpty())
    return;

  // some users are used to omit the extension and expect a xls file by default
  if(!fileName.toLower().endsWith(".csv") && !fileName.toLower().endsWith(".xls"))
    fileName += tr(".xls");

  bool result = false;
  if (fileName.endsWith(".csv"))
  {
    result = exportToCSV(fileName);
  } 
  else
  {
    result = exportToXLS(fileName);
  }

  if (!result)
    QMessageBox::warning(this, "EspINA", tr("Unable to export %1").arg(fileName));
}

//----------------------------------------------------------------------------
void SASAnalysisEntry::syncGeometry()
{
  for (int c = 0; c < analysisTable->columnCount(); ++c)
  {
    analysisTable->setColumnWidth(c, dataTable->columnWidth(c));
  }
}

//----------------------------------------------------------------------------
bool SASAnalysisEntry::exportToCSV(const QString &filename)
{
  QFile file(filename);

  file.open(QIODevice::WriteOnly |  QIODevice::Text);

  QTextStream out(&file);
  for (int c = 0; c < dataTable->columnCount(); ++c)
  {
    if (c)
      out << ",";
    out << dataTable->horizontalHeaderItem(c)->data(Qt::DisplayRole).toString();
  }
  out << "\n";

  for (int r = 0; r < dataTable->rowCount(); ++r)
  {
    for (int c = 0; c < dataTable->columnCount(); ++c)
    {
      if (c)
        out << ",";
      out << dataTable->item(r,c)->data(Qt::DisplayRole).toString();
    }
    out << "\n";
  }

  out << "\n";

//   for (int r = 0; r < analysisTable->rowCount(); ++r)
//   {
//     for (int c = 0; c < analysisTable->columnCount(); ++c)
//     {
//       if (c)
//         out << ",";
//       out << analysisTable->item(r,c)->data(Qt::DisplayRole).toString();
//     }
//     out << "\n";
//   }

  file.close();

  return true;
}

//----------------------------------------------------------------------------
bool SASAnalysisEntry::exportToXLS(const QString &filename)
{
  workbook wb;

  exportToXLS(wb);

  wb.Dump(filename.toStdString());

  return true;
}
