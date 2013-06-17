/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Laura Fernandez Soria <laura.fernandez@ctb.upm.es>

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

// EspINA
#include <Core/Model/EspinaModel.h>
#include <Core/EspinaRegion.h>
#include <Core/EspinaTypes.h>
#include <App/Dialogs//TabularReport/TabularReport.h>
#include <Core/Model/Proxies/ConnectomicProxy.h>
#include <Core/Utils/SegmentationCollision.h>
#include <Core/Relations.h>
#include "ConnectomicsDialog.h"

// Qt
#include <QFileDialog>
#include <QtDebug>
#include <QListView>
#include <QAbstractProxyModel>

// C++
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

// itk
#include <itkImage.h>

using namespace EspINA;

//------------------------------------------------------------------------
ConnectomicsDialog::ConnectomicsDialog(EspinaModel *model,
                                       ViewManager *viewManager,
                                       QWidget *parent,
                                       Qt::WindowFlags flags)
: QDialog(parent, flags)
, m_model(model)
, m_viewManager(viewManager)
{
  setObjectName("ConnectomicsInformationDialog");
  setWindowTitle("Connectomics Information");
  setupUi(this);
  listView1->setModel(m_model);
  listView1->setRootIndex(m_model->segmentationRoot());

  generateConectomicGraph(m_model);
  m_listView << listView1;

  connect(listView1, SIGNAL(clicked(QModelIndex)), 
          this, SLOT(showGraphConnectomicsInformation(QModelIndex)));
}

//------------------------------------------------------------------------
void ConnectomicsDialog::generateConectomicGraph(EspinaModel *m_model)
{
  /* FIXME
  foreach(SegmentationSPtr seg, m_model->segmentations())
  {
    ModelItemSList res;
    SegmentationVolume::Pointer segVolume = seg->volume();
    foreach(SegmentationSPtr seg_others, m_model->segmentations())
    {
      if ((seg_others.data() != seg.data()) && (seg->volume()->espinaRegion().intersect(seg_others->volume()->espinaRegion())))
      {
        SegmentationVolume::Pointer otherSegVolume = seg_others->volume();
        if (checkCollision(segVolume, otherSegVolume))
        {
          res = seg->relatedItems(EspINA::RELATION_OUT, CONECTOMICA);
          bool no_existe = true;
          foreach (ModelItemSPtr i_res, res)
          {
            SegmentationPtr seg_i = reinterpret_cast<SegmentationPtr>(i_res.data());
            if (seg_i == seg_others.data())
            {
              no_existe = false;
              break;
            }
          }

          if (no_existe)
            m_model->addRelation(seg, seg_others, CONECTOMICA);
        }
      }
    }
  }
  */
}

//------------------------------------------------------------------------
void ConnectomicsDialog::showGraphConnectomicsInformation(QModelIndex index)
{
  ConnectomicProxy *m_proxy_aux = new ConnectomicProxy();
  QListView *list_sender = dynamic_cast<QListView *>(sender());
  QModelIndex index_proxy = index;
  QListView *listView_aux = new QListView(this);
  if (list_sender == listView1)
  {
    if (!index.isValid())
      return;
    else
      for (int j = (m_listView.size() - 1); j > 0; j--)
      {
        delete (m_listView.takeAt((m_listView.indexOf(list_sender)) + 1));
        selection.removeAt((m_listView.indexOf(list_sender)) + 1);
      }
  }
  else
  {
    const QAbstractProxyModel* p_model = dynamic_cast<const ConnectomicProxy*>(index_proxy.model());
    if (p_model)
      index_proxy = p_model->mapToSource(index);

    if (!index_proxy.isValid())
      return;

    // eliminar las post-view
    int iter = (m_listView.size() - (m_listView.indexOf(list_sender) + 1));
    for (int j = iter; j > 0; j--)
    {
      delete (m_listView.takeAt((m_listView.indexOf(list_sender)) + 1));
      selection.removeAt((m_listView.indexOf(list_sender)) + 1);
    }
  }

  m_proxy_aux->setSourceModel(m_model);
  ModelItemPtr item = indexPtr(index_proxy);
  Segmentation *seg = dynamic_cast<Segmentation *>(item);

  if (EspINA::SEGMENTATION != item->type())
    return;

  m_proxy_aux->setFilterBy(seg);
  listView_aux->setModel(m_proxy_aux);
  listView_aux->setRootIndex(m_proxy_aux->mapFromSource(m_model->segmentationRoot()));
  hor_layout->addWidget(listView_aux);
  m_listView << listView_aux;

  int row = index_proxy.row();
  QModelIndex indexEspinaModel = m_model->index(row, 0, m_model->segmentationRoot());

  ModelItemPtr item2 = indexPtr(indexEspinaModel);
  if (EspINA::SEGMENTATION != item2->type())
    return;
  selection << dynamic_cast<PickableItem *>(item2);
  m_viewManager->setSelection(selection);

  connect(m_listView.at(m_listView.size() - 1), SIGNAL(clicked(QModelIndex)), this, SLOT(showGraphConnectomicsInformation(QModelIndex)));
}
