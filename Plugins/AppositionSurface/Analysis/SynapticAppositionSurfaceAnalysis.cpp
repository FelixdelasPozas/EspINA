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
#include <QUndoStack>
#include <QFileDialog>
#include <QScrollBar>

using namespace EspINA;

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

  tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

  connect(tableView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
          analysis->horizontalScrollBar(), SLOT(setValue(int)));

  connect(analysis->horizontalScrollBar(), SIGNAL(valueChanged(int)), 
          tableView->horizontalScrollBar(), SLOT(setValue(int)));

  connect(tableView, SIGNAL(cellChanged(int,int)),
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

  tableView->setRowCount(rowCount);
  tableView->setColumnCount(synapseColumns + sasColumns);

  analysis->setRowCount(1);
  analysis->setColumnCount(synapseColumns + sasColumns);

  QStringList headers;
  headers << m_synapseTags;
  foreach(QString sasHeader, m_sasTags)
  {
    headers << tr("SAS %1").arg(sasHeader);
  }
  tableView->setHorizontalHeaderLabels(headers);
  analysis->setHorizontalHeaderLabels(headers);

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
      tableView->setItem(r, c, item);

      if (r == 0)
      {
        analysis->setItem(0, c, new QTableWidgetItem("0"));
      }

      bool   isNumber;
      double cellValue = cell.toDouble(&isNumber);
      if (isNumber)
      {
        item->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
        double sum = analysis->item(0, c)->data(Qt::DisplayRole).toDouble() + cellValue;
        analysis->item(0, c)->setData(Qt::DisplayRole, sum);
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

    SegmentationPtr sas = segmentationPtr(relatedItems.first().data());

    for (int c = 0; c < sasColumns; ++c)
    {
      QVariant cell = sas->information(m_sasTags[c]);

      QTableWidgetItem *item = new QTableWidgetItem();
      item->setData(Qt::DisplayRole, cell);
      tableView->setItem(r, synapseColumns + c, item);

      int ac = c + synapseColumns;
      if (r == 0)
      {
        analysis->setItem(0, ac, new QTableWidgetItem("0"));
      }

      bool   isNumber;
      double cellValue = cell.toDouble(&isNumber);
      if (isNumber)
      {
        item->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
        double sum = analysis->item(0, ac)->data(Qt::DisplayRole).toDouble() + cellValue;
        analysis->item(0, ac)->setData(Qt::DisplayRole, sum);
      }
    }

    tableView->item(r, 0)->setData(Qt::DecorationRole, segmentation->data(Qt::DecorationRole));

    progressBar->setValue(r);
  }

  for (int c = 0; c < synapseColumns + sasColumns; ++c)
  {
    bool   isNumber;
    if (analysis->item(0, c))
    {
      double cellValue = analysis->item(0, c)->data(Qt::DisplayRole).toDouble(&isNumber);
      if (isNumber)
      {
        analysis->item(0, c)->setData(Qt::DisplayRole, cellValue / rowCount);
        analysis->item(0, c)->setData(Qt::TextAlignmentRole, Qt::AlignCenter);
      }
    }
  }

  if (createSAS)
  {
    m_model->emitSegmentationAdded(createdSegmentations);
    m_undoStack->endMacro();
  }

  tableView->resizeColumnsToContents();
  tableView->setSortingEnabled(true);
  tableView->horizontalHeader()->setMovable(true);
  analysis->horizontalHeader()->setMovable(true);

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
  QString fileExt = tr("CSV Text File (*.csv)");
  QString fileName = QFileDialog::getSaveFileName(this, title, "", fileExt);

  if (fileName.isEmpty())
    return;

  QFile file(fileName);
  file.open(QIODevice::WriteOnly |  QIODevice::Text);
  QTextStream out(&file);
  for (int c = 0; c < analysis->columnCount(); ++c)
  {
    if (c)
      out << ",";
    out << analysis->horizontalHeaderItem(c)->data(Qt::DisplayRole).toString();
  }
  out << "\n";

  for (int r = 0; r < analysis->rowCount(); r++)
  {
    for (int c = 0; c < analysis->columnCount(); c++)
    {
      if (c)
        out << ",";
      out << analysis->item(r,c)->data(Qt::DisplayRole).toString();
    }
    out << "\n";
  }
  file.close();
}

//----------------------------------------------------------------------------
void SynapticAppositionSurfaceAnalysis::syncGeometry()
{
  for (int c = 0; c < analysis->columnCount(); ++c)
  {
    analysis->setColumnWidth(c, tableView->columnWidth(c));
  }
}
