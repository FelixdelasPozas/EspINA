/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "MainToolBar.h"

#include <common/model/EspINA.h>

#include <QAction>
#include <QComboBox>
#include <QIcon>
#include <QTreeView>

//----------------------------------------------------------------------------
MainToolBar::MainToolBar(QSharedPointer<EspINA> model, QWidget* parent)
: QToolBar(parent)
{
  setWindowTitle("EspINA");
  setObjectName("MainToolBar");

  toggleSegVisibility = addAction(//showIcon,
				  tr("Toggle Segmentations Visibility"));

  toggleSegVisibility->setShortcut(QString("Space"));
  toggleSegVisibility->setCheckable(true);
  toggleSegVisibility->setChecked(true);
  setShowSegmentations(true);
  connect(toggleSegVisibility,SIGNAL(triggered(bool)),
	  this,SLOT(setShowSegmentations(bool)));


   // User selected Taxonomy Selection List
  taxonomyView = new QTreeView(this);
  taxonomyView->setHeaderHidden(true);

  taxonomySelector = new QComboBox(this);
  taxonomySelector->setView(taxonomyView); //Brutal!
  taxonomySelector->setModel(model.data());
  taxonomySelector->setRootModelIndex(model->taxonomyRoot());
  connect(model.data(),SIGNAL(dataChanged(QModelIndex,QModelIndex)),
	  this,SLOT(updateTaxonomy(QModelIndex,QModelIndex)));
	  
  taxonomySelector->setMinimumWidth(160);
  taxonomySelector->setToolTip( tr("Type of new segmentation") );
  taxonomySelector->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  addWidget(taxonomySelector);
//   taxonomySelector->setRootModelIndex(m_espina);

//   connect(m_taxonomyView, SIGNAL(entered(QModelIndex)),
//           this, SLOT(setUserDefinedTaxonomy(QModelIndex)));
//   m_taxonomySelector->setCurrentIndex(0);
//   Internals->toolBar->addWidget(m_taxonomySelector);
//   Internals->toolBar->addAction(Internals->actionRemoveSegmentation);
//   connect(Internals->actionRemoveSegmentation,SIGNAL(toggled(bool)),
//           this,SLOT(removeSegmentationClicked(bool)));
}

//----------------------------------------------------------------------------
void MainToolBar::setShowSegmentations(bool visible)
{
  if (visible)
    toggleSegVisibility->setIcon(QIcon(":/espina/show_all.svg"));
  else
    toggleSegVisibility->setIcon(QIcon(":/espina/hide_all.svg"));

  emit showSegmentations(visible);
}

//----------------------------------------------------------------------------
void MainToolBar::updateTaxonomy(QModelIndex left, QModelIndex right)
{
  if (left == taxonomySelector->view()->rootIndex())
    taxonomySelector->setCurrentIndex(0);
  taxonomyView->expandAll();
}
