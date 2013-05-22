/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2012  Laura Fernandez Soria <laura.fernandez@ctb.upm.es>
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
 */

// EspINA
#include <Core/Model/Segmentation.h>
#include <Core/Model/RelationshipGraph.h>
#include <Core/Model/EspinaModel.h>
#include "ConnectomicProxy.h"

// Qt
#include <QSortFilterProxyModel>

using namespace EspINA;

//------------------------------------------------------------------------
ConnectomicProxy::ConnectomicProxy(QObject* parent)
: QSortFilterProxyModel(parent)
, m_seg(NULL)
{
}

//------------------------------------------------------------------------
bool ConnectomicProxy::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const
{
  if (!m_seg)
    return true;

  QAbstractItemModel* source = this->sourceModel();
  EspinaModel *model = dynamic_cast<EspinaModel *>(source);
  QModelIndex index = model->index(sourceRow, 0, sourceParent);

  if (!index.isValid()
    || index == model->sampleRoot()
    || index == model->channelRoot()
    || index == model->filterRoot()
    || index == model->taxonomyRoot())
    return false;

  if (index == model->segmentationRoot())
    return true;

  ModelItemPtr item = indexPtr(index);

  if (EspINA::SEGMENTATION != item->type())
    return false;

  //   std::cout << "Check Segmentation " << item->data().toString().toStdString() << std::endl;
    SegmentationPtr seg = segmentationPtr(item);
    ModelItemSList res = m_seg->relatedItems(EspINA::OUT, CONECTOMICA); // elementos conectados a la seg de la vista

    foreach (ModelItemSPtr i_res, res)
    {
      SegmentationPtr seg_i = segmentationPtr(i_res.get());
      if (seg_i == seg)
        return true;
    }

    return false;
}
