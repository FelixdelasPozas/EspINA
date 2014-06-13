/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Plugin
#include "SASAnalysisDialog.h"

// EspINA
#include <Core/Extensions/AppositionSurfaceExtension.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <Core/Factory/CoreFactory.h>
#include <Support/Settings/EspinaSettings.h>
#include <Extensions/Morphological/MorphologicalInformation.h>
#include <GUI/Widgets/InformationSelector.h>

// Qt
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QHBoxLayout>

using namespace EspINA;
using namespace xlslib_core;

//----------------------------------------------------------------------------
SASAnalysisDialog::SASAnalysisDialog(SegmentationAdapterList segmentations,
                                     ModelAdapterSPtr        model,
                                     QUndoStack             *undoStack,
                                     ModelFactorySPtr        factory,
                                     ViewManagerSPtr         viewManager,
                                     QWidget                *parent)
: QDialog(parent)
, m_model      {model}
, m_factory    {factory}
, m_undoStack  {undoStack}
, m_viewManager{viewManager}
, m_synapses   {segmentations}
{
  setupUi(this);

  m_progressBar->setVisible(false);

  setObjectName("SASAnalysisDialog");
  QHBoxLayout *layout = new QHBoxLayout();

  QPushButton *configureButton = new QPushButton();
  configureButton->setIcon(QIcon(":/espina/settings.png"));
  configureButton->setFlat(false);
  configureButton->setToolTip("Select Analysis Information");
  layout->addWidget(configureButton);

  connect(configureButton, SIGNAL(clicked(bool)),
          this, SLOT(configureInformation()));

  QPushButton *exportButton = new QPushButton();
  QIcon saveIcon = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  exportButton->setIcon(saveIcon);
  exportButton->setFlat(false);
  exportButton->setToolTip("Save All Data");
  layout->addWidget(exportButton);

  connect(exportButton, SIGNAL(clicked(bool)),
          this, SLOT(exportInformation()));

  QWidget *cornerWidget = new QWidget();
  cornerWidget->setLayout(layout);
  cornerWidget->setMinimumSize(80,40);

  connect(m_buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked(bool)),
          this, SLOT(close()));

  m_tabs->setCornerWidget(cornerWidget);

  QMap<QString, SegmentationAdapterList> tabs;
  for(auto segmentation: segmentations)
    tabs[segmentation->category()->classificationName()] << segmentation;

  QSettings settings(CESVIMA, ESPINA);

  settings.beginGroup("Synaptic Apposition Surface Information Analysis");
  resize(settings.value("size", QSize(200, 200)).toSize());
  move(settings.value("pos", QPoint(200, 200)).toPoint());

  m_synapseTags = settings.value("Synapse Tags", QStringList()).toStringList();
  m_sasTags = settings.value("SAS Tags", QStringList()).toStringList();

  Q_ASSERT(m_factory->availableSegmentationExtensions().contains(MorphologicalInformation::TYPE));
  Q_ASSERT(m_factory->availableSegmentationExtensions().contains(AppositionSurfaceExtension::TYPE));

  auto morphologicalExtensionInformations = m_factory->createSegmentationExtension(MorphologicalInformation::TYPE)->availableInformations();
  auto SASInformations = m_factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE)->availableInformations();

  // clean tags in the case some older version has different tags
  SegmentationExtension::InfoTagList wrongTags;
  for(auto tag: m_synapseTags)
  {
    if(tag != tr("Name") && tag != tr("Category") && !morphologicalExtensionInformations.contains(tag))
      wrongTags << tag;
  }

  if(!wrongTags.empty())
    for(auto tag: wrongTags)
    m_synapseTags.removeAll(tag);

  // clean tags in the case some older version has different tags
  wrongTags.clear();
  for(auto tag: m_sasTags)
  {
    if(!SASInformations.contains(tag))
      wrongTags << tag;
  }

  if(!wrongTags.empty())
    for(auto tag: wrongTags)
    m_sasTags.removeAll(tag);

  if (m_synapseTags.isEmpty())
  {
    m_synapseTags << tr("Name") << tr("Category");
  }
  else if (m_synapseTags.first() != tr("Name"))
  {
    m_synapseTags.removeAll(tr("Name"));
    m_synapseTags.removeAll(tr("Category"));
    m_synapseTags.prepend(tr("Category"));
    m_synapseTags.prepend(tr("Name"));
  }

  if (m_sasTags.isEmpty())
  {
    m_sasTags << SASInformations;
  }

  settings.endGroup();

  show();

  createTabs(tabs);
}

//----------------------------------------------------------------------------
void SASAnalysisDialog::createTabs(QMap<QString, SegmentationAdapterList> tabs)
{
  const QString SYNAPSE = tr("Synapse");
  const QString SAS     = tr("SAS");

  InformationSelector::GroupedInfo tags;
  tags[SYNAPSE] = m_synapseTags;
  tags[SAS] = m_sasTags;

  QApplication::setOverrideCursor(Qt::WaitCursor);
  for(auto category: tabs.keys())
  {
    SASAnalysisEntry *entry = new SASAnalysisEntry(tabs[category], m_model, m_factory, this);
    entry->defineQuery(tags);
    m_tabs->addTab(entry, category);
    m_entries << entry;
  }
  QApplication::restoreOverrideCursor();
}

//----------------------------------------------------------------------------
QStringList SASAnalysisDialog::toStringList(SegmentationExtension::InfoTagList tags) const
{
  QStringList list;

  for(auto tag: tags)
    list << tag;

  return list;
}

//----------------------------------------------------------------------------
void SASAnalysisDialog::closeEvent(QCloseEvent *event)
{
  QSettings settings(CESVIMA, ESPINA);

  settings.beginGroup("Synaptic Apposition Surface Information Analysis");
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.endGroup();

  QDialog::closeEvent(event);
}

//----------------------------------------------------------------------------
void SASAnalysisDialog::configureInformation()
{
  const QString SYNAPSE = tr("Synapse");
  const QString SAS     = tr("SAS");

  Q_ASSERT(m_factory->availableSegmentationExtensions().contains(MorphologicalInformation::TYPE));

  InformationSelector::GroupedInfo tags, selection;
  tags[SYNAPSE]      = m_factory->createSegmentationExtension(MorphologicalInformation::TYPE)->availableInformations();
  tags[SAS]          = m_factory->createSegmentationExtension(AppositionSurfaceExtension::TYPE)->availableInformations();
  selection[SYNAPSE] = m_synapseTags;
  selection[SAS]     = m_sasTags;

  InformationSelector tagSelector{tags, selection, this};

  if (tagSelector.exec() == QDialog::Accepted)
  {
    selection[SYNAPSE].prepend(tr("Category"));
    selection[SYNAPSE].prepend(tr("Name"));

    for(auto entry: m_entries)
      entry->defineQuery(selection);

    m_synapseTags = selection[SYNAPSE];
    m_sasTags     = selection[SAS];

    QSettings settings(CESVIMA, ESPINA);

    settings.beginGroup("Synaptic Apposition Surface Information Analysis");

    settings.setValue("Synapse Tags", toStringList(m_synapseTags));
    settings.setValue("SAS Tags", toStringList(m_sasTags));

    settings.endGroup();

    settings.sync();
  }
}

//----------------------------------------------------------------------------
void SASAnalysisDialog::exportInformation()
{
  QString title   = tr("Export Synaptic Apposition Surface Analysis");
  QString filter = tr("Excel File (*.xls)");
  QString fileName = QFileDialog::getSaveFileName(this,
                                                  title,
                                                  "",
                                                  filter);

  if (fileName.isEmpty())
    return;

  if (fileName.endsWith(".xls"))
    fileName += tr(".xls");

  bool result = !m_entries.isEmpty();

  workbook wb;
  for(auto entry: m_entries)
    result &= entry->exportToXLS(wb);

  wb.Dump(fileName.toStdString());

  if (!result)
    QMessageBox::warning(this, "EspINA", tr("Unable to export %1").arg(fileName));
}
