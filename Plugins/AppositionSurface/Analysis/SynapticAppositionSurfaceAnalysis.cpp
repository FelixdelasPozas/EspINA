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


#include "SynapticAppositionSurfaceAnalysis.h"
#include <Filter/AppositionSurfaceFilter.h>
#include <Core/Extensions/AppositionSurfaceExtension.h>
#include <Undo/AppositionSurfaceCommand.h>

#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/Extensions/Morphological/MorphologicalInformation.h>
#include <GUI/Analysis/InformationSelector.h>
#include <GUI/Analysis/xlsUtils.h>
#include <xlslib/workbook.h>
#include <QUndoStack>
#include <QFileDialog>
#include <QScrollBar>
#include <QMessageBox>

using namespace EspINA;
using namespace xlslib_core;

//----------------------------------------------------------------------------
SynapticAppositionSurfaceAnalysis::SynapticAppositionSurfaceAnalysis(SegmentationList segmentations,
                                                                     EspinaModel      *model,
                                                                     QUndoStack       *undoStack,
                                                                     ViewManager      *viewManager,
                                                                     QWidget          *parent)
: QDialog(parent)
, m_model(model)
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_synapses(segmentations)
{
  setupUi(this);

  setObjectName("SynapticAppositionSurfaceAnalysis");

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

  m_synapseTags << tr("Name") << tr("Taxonomy");
  m_synapseTags << model->factory()->segmentationExtension(MorphologicalInformationID)->availableInformations();

  m_sasTags << AppositionSurfaceExtension::AREA;
  m_sasTags << AppositionSurfaceExtension::PERIMETER;
  m_sasTags << AppositionSurfaceExtension::TORTUOSITY;

  show();

  displayInformation();
}

//----------------------------------------------------------------------------
void SynapticAppositionSurfaceAnalysis::displayInformation()
{
  progressBar->setVisible(true);

  int synapseColumns = m_synapseTags.size();
  int sasColumns     = m_sasTags.size();
  int rowCount       = m_synapses.size();

  dataTable->setRowCount(rowCount);
  dataTable->setColumnCount(synapseColumns + sasColumns);

  analysisTable->setRowCount(1);
  analysisTable->setColumnCount(synapseColumns + sasColumns);

  QStringList headers;
  headers << m_synapseTags;
  foreach(QString sasHeader, m_sasTags)
  {
    headers << tr("SAS %1").arg(sasHeader);
  }
  dataTable->setHorizontalHeaderLabels(headers);
  analysisTable->setHorizontalHeaderLabels(headers);

  progressBar->setMinimum(-1);
  progressBar->setMaximum(rowCount - 1);

  bool createSAS = false;
  SegmentationSList createdSegmentations;

  for (int r = 0; r < rowCount; ++r)
  {
    SegmentationPtr segmentation = m_synapses[r];

    for(int c = 0; c < synapseColumns; ++c)
    {
      QVariant cell = segmentation->information(m_synapseTags[c]);

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

    ModelItemSList relatedItems = segmentation->relatedItems(EspINA::OUT, AppositionSurfaceFilter::SAS);
    if (relatedItems.isEmpty())
    {
      if (!createSAS)
      {
        m_undoStack->beginMacro("Create Synaptic Apposition Surface");
      }

      SegmentationList noSASSegmentations;
      noSASSegmentations << segmentation;

      m_undoStack->push(new AppositionSurfaceCommand(noSASSegmentations, m_model, m_viewManager, createdSegmentations));

      relatedItems = segmentation->relatedItems(EspINA::OUT, AppositionSurfaceFilter::SAS);
      createSAS = true;
    }

    if (relatedItems.size() > 1)
    {
      qWarning() << segmentation->data().toString() <<  "has several SAS segmentations associated";
    }

    SegmentationPtr sas = segmentationPtr(relatedItems.first().get());

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

    progressBar->setValue(r);
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

  if (createSAS)
  {
    m_model->emitSegmentationAdded(createdSegmentations);
    m_undoStack->endMacro();
  }

  dataTable->resizeColumnsToContents();
  dataTable->setSortingEnabled(true);
  dataTable->horizontalHeader()->setMovable(true);
  analysisTable->horizontalHeader()->setMovable(true);

  syncGeometry();

  progressBar->setVisible(false);

}

//----------------------------------------------------------------------------
void SynapticAppositionSurfaceAnalysis::defineQuery()
{
  const QString SYNAPSE = tr("Synapse");
  const QString SAS     = tr("SAS");

  InformationSelector::TaxonomyInformation tags;
  tags[SYNAPSE] = m_synapseTags;
  tags[SAS]     = m_sasTags;

  InformationSelector tagSelector(tags,
                                  m_model->factory(),
                                  this);

  if (tagSelector.exec() == QDialog::Accepted)
  {
    m_synapseTags.clear();
    m_synapseTags << tr("Name") << tr("Taxonomy") << tags[SYNAPSE];

    m_sasTags.clear();
    m_sasTags << tags[SAS];

    displayInformation();
  }
}

//----------------------------------------------------------------------------
void SynapticAppositionSurfaceAnalysis::saveAnalysis()
{
  QString title   = tr("Export Synaptic Apposition Surface Analysis");
  QString filter = tr("Excel File (*.xls)") + ";;" + tr("CSV Text File (*.csv)");
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  title,
                                                  "",
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

//----------------------------------------------------------------------------
void SynapticAppositionSurfaceAnalysis::syncGeometry()
{
  for (int c = 0; c < analysisTable->columnCount(); ++c)
  {
    analysisTable->setColumnWidth(c, dataTable->columnWidth(c));
  }
}

//----------------------------------------------------------------------------
bool SynapticAppositionSurfaceAnalysis::exportToCSV(const QString &filename)
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

  for (int r = 0; r < analysisTable->rowCount(); ++r)
  {
    for (int c = 0; c < analysisTable->columnCount(); ++c)
    {
      if (c)
        out << ",";
      out << analysisTable->item(r,c)->data(Qt::DisplayRole).toString();
    }
    out << "\n";
  }

  file.close();

  return true;
}

//----------------------------------------------------------------------------
bool SynapticAppositionSurfaceAnalysis::exportToXLS(const QString &filename)
{
  workbook wb;
  worksheet *sheet = wb.sheet("Synaptic Apposition Surface Analysis");

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

  for (int r = 0; r < analysisTable->rowCount(); ++r, ++sheetRow)
  {
    for (int c = 0; c < analysisTable->columnCount(); ++c)
    {
      createCell(sheet, sheetRow, c, analysisTable->item(r,c)->data(Qt::DisplayRole));
    }
  }

  wb.Dump(filename.toStdString());

  return true;
}
