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

#include <QIcon>
#include <QComboBox>
#include <QTreeView>

MainToolBar::MainToolBar(QSharedPointer<EspINA> model, QWidget* parent)
: QToolBar(parent)
{
  setWindowTitle("EspINA");

  addAction(QIcon(":/espina/show_all.svg"),tr("Toggle Segmentations Visibility"));

   // User selected Taxonomy Selection List
  QTreeView *taxonomyView = new QTreeView(this);
  taxonomyView->setHeaderHidden(true);

  QComboBox *taxonomySelector = new QComboBox(this);
  taxonomySelector->setView(taxonomyView); //Brutal!
  taxonomySelector->setModel(model.data());
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
