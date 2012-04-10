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


#include "TaxonomyInspector.h"
#include <ui_TaxonomyInspector.h>

#include "common/model/EspinaModel.h"
#include "common/model/Taxonomy.h"

#include <QColorDialog>
#include <QSortFilterProxyModel>


//------------------------------------------------------------------------
class TaxonomyInspector::GUI
: public QWidget
, public Ui::TaxonomyInspector
{
public:
  GUI(){setupUi(this);}
};

//------------------------------------------------------------------------
TaxonomyInspector::TaxonomyInspector(QSharedPointer<EspinaModel> model, QWidget* parent)
: EspinaDockWidget(parent)
, m_gui(new GUI())
, m_baseModel(model)
, m_sort(new QSortFilterProxyModel())
{
  setWindowTitle(tr("Taxonomy Inspector"));
  setObjectName("TaxonomyInspector");
  m_sort->setSourceModel(m_baseModel.data());
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
TaxonomyInspector::~TaxonomyInspector()
{

}

//------------------------------------------------------------------------
void TaxonomyInspector::addSameLevelTaxonomy()
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
void TaxonomyInspector::addSubTaxonomy()
{
  QModelIndex currentIndex = m_gui->treeView->currentIndex();
  if (!currentIndex.isValid())
    currentIndex = m_gui->treeView->rootIndex();

  QModelIndex index = m_sort->mapToSource(currentIndex);
  QModelIndex tax = m_baseModel->addTaxonomyElement(index,"Undefined");
  m_gui->treeView->setCurrentIndex(tax);
}

//------------------------------------------------------------------------
void TaxonomyInspector::changeColor()
{
  QColorDialog colorSelector;
  if( colorSelector.exec() == QDialog::Accepted)
  {
    QModelIndex index = m_sort->mapToSource(m_gui->treeView->currentIndex());
    m_baseModel->setData(index,
			 colorSelector.selectedColor(),
			 Qt::DecorationRole);
  }
}

//------------------------------------------------------------------------
void TaxonomyInspector::removeSelectedTaxonomy()
{
  if (m_gui->treeView->currentIndex().isValid())
  {
    QModelIndex index = m_sort->mapToSource(m_gui->treeView->currentIndex());
    m_baseModel->removeTaxonomyElement(index);
  }
}
