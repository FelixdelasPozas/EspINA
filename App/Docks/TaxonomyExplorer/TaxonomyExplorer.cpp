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


#include "TaxonomyExplorer.h"
#include <ui_TaxonomyExplorer.h>

// EspINA
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Taxonomy.h>
#include <Core/Model/Segmentation.h>
#include <GUI/ViewManager.h>

// Qt
#include <QColorDialog>
#include <QMessageBox>

using namespace EspINA;

//------------------------------------------------------------------------
class TaxonomyExplorer::GUI
: public QWidget
, public Ui::TaxonomyExplorer
{
public:
  GUI(){setupUi(this);}
};

//------------------------------------------------------------------------
TaxonomyExplorer::TaxonomyExplorer(EspinaModel *model,
                                   ViewManager *vm,
                                   TaxonomyColorEnginePtr engine,
                                   QWidget* parent)
: QDockWidget(parent)
, m_baseModel(model)
, m_viewManager(vm)
, m_engine(engine)
, m_gui(new GUI())
, m_sort(new QSortFilterProxyModel())
{
  setObjectName("TaxonomyExplorer");

  setWindowTitle(tr("Taxonomy Explorer"));

  m_sort->setSourceModel(m_baseModel);
  m_sort->setDynamicSortFilter(true);
  m_gui->treeView->setModel(m_sort.data());
  m_gui->treeView->setRootIndex(m_sort->mapFromSource(m_baseModel->taxonomyRoot()));
  m_gui->treeView->sortByColumn(0, Qt::AscendingOrder);

  connect(m_gui->addTaxonomy, SIGNAL(clicked(bool)),
          this, SLOT(addSameLevelTaxonomy()));
  connect(m_gui->addSubTaxonomy, SIGNAL(clicked(bool)),
          this, SLOT(addSubTaxonomy()));
  connect(m_gui->colorSelector, SIGNAL(clicked(bool)),
          this, SLOT(changeColor()));
  connect(m_gui->removeTaxonomy, SIGNAL(clicked(bool)),
          this, SLOT(removeSelectedTaxonomy()));

  setWidget(m_gui);
}

//------------------------------------------------------------------------
TaxonomyExplorer::~TaxonomyExplorer()
{
  qDebug() << "Destroy TaxExplorer";
  delete m_gui;
}

//------------------------------------------------------------------------
void TaxonomyExplorer::addSameLevelTaxonomy()
{
  QModelIndex currentIndex = m_gui->treeView->currentIndex();
  QModelIndex parent = currentIndex.parent();

  if (!parent.isValid())
    parent = m_gui->treeView->rootIndex();

  QModelIndex index = m_sort->mapToSource(parent);
  QModelIndex tax = m_baseModel->addTaxonomyElement(index,"Undefined");
  m_gui->treeView->setCurrentIndex(tax);
}

//------------------------------------------------------------------------
void TaxonomyExplorer::addSubTaxonomy()
{
  QModelIndex currentIndex = m_gui->treeView->currentIndex();
  if (!currentIndex.isValid())
    currentIndex = m_gui->treeView->rootIndex();

  QModelIndex index = m_sort->mapToSource(currentIndex);
  QModelIndex tax = m_baseModel->addTaxonomyElement(index,"Undefined");
  m_gui->treeView->setCurrentIndex(tax);
}

//------------------------------------------------------------------------
void TaxonomyExplorer::changeColor()
{
  QColorDialog colorSelector;
  if( colorSelector.exec() == QDialog::Accepted)
  {
    QModelIndex index = m_sort->mapToSource(m_gui->treeView->currentIndex());
    m_baseModel->setData(index,
                         colorSelector.selectedColor(),
                         Qt::DecorationRole);
    ModelItemPtr item = indexPtr(index);
    Q_ASSERT(EspINA::TAXONOMY == item->type());
    TaxonomyElementPtr tax = taxonomyElementPtr(item);
    m_engine->updateTaxonomyColor(tax);
    m_viewManager->updateSegmentationRepresentations();
    m_viewManager->updateViews();
  }
}

//------------------------------------------------------------------------
void TaxonomyExplorer::removeSelectedTaxonomy()
{
  if (m_gui->treeView->currentIndex().isValid())
  {
    QModelIndex index = m_sort->mapToSource(m_gui->treeView->currentIndex());
    ModelItemPtr item = indexPtr(index);
    TaxonomyElementPtr tax = taxonomyElementPtr(item);

    if (tax->subElements().isEmpty())
    {
      bool inUse = false;
      int i = 0;
      while (!inUse && i < m_baseModel->segmentations().size())
        inUse = m_baseModel->segmentations()[i++]->taxonomy() == tax;

      if (!inUse)
        m_baseModel->removeTaxonomyElement(index);
      else
        QMessageBox::warning(this,
                             tr("Couldn't Remove Taxonomy's Element"),
                             tr("Selected taxonomical element is assigned to a segmentation."));
    }
    else
      QMessageBox::warning(this,
                           tr("Couldn't Remove Taxonomy's Element"),
                           tr("Other taxonomical elements depend on this taxonomy's element.\n"
                           "If you want to remove it, remove dependent taxonomies first."));
  }
}
