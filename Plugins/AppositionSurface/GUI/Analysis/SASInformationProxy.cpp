/*
 
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// Plugin
#include "SASInformationProxy.h"

#include "Core/Extensions/AppositionSurfaceExtension.h"
#include <AppositionSurfacePlugin.h>

// ESPINA
#include <Core/Analysis/Extension.h>
#include <GUI/Model/Proxies/InformationProxy.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// Qt
#include <QString>

using namespace ESPINA;
using namespace ESPINA::GUI::Model::Utils;

class SASInformationProxy::SASInformationFetcher
: public InformationProxy::InformationFetcher
{
public:
  SASInformationFetcher(SegmentationAdapterPtr segmentation,
                        SegmentationAdapterPtr sas,
                        const SegmentationExtension::InformationKeyList &keys,
                        SchedulerSPtr scheduler)
  : InformationProxy::InformationFetcher(segmentation, keys, scheduler)
  , m_sas{sas}
  {
    auto id = Segmentation->data(Qt::DisplayRole).toString();
    setDescription(tr("%1 information").arg(id));
    setHidden(true);

    m_keys.removeOne(NameKey());
    m_keys.removeOne(CategoryKey());

    bool ready = true;
    for (auto key : m_keys)
    {
      ready &= m_sas->isReady(key);

      if (!ready) break;
    }

    setFinished(ready);
  }

  SegmentationAdapterPtr m_sas;

protected:
  virtual void run()
  {
    for (int i = 0; i < m_keys.size(); ++i)
    {
      if (!canExecute()) break;

      auto key = m_keys[i];
      if (key != NameKey() && key != CategoryKey())
      {
        if(!m_sas->isReady(key))
        {
          m_sas->information(key);
          if (!canExecute()) break;
        }
      }

      m_progress = (100.0*i)/m_keys.size();
      reportProgress(m_progress);
    }
  }
};

//----------------------------------------------------------------------------
QVariant SASInformationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid()) return QVariant();

  auto proxyItem = itemAdapter(proxyIndex);
  if (!isSegmentation(proxyItem))
    return QVariant();

  auto segmentation = segmentationPtr(proxyItem);

  if (role == Qt::TextAlignmentRole)
  {
    return Qt::AlignRight;
  }

  if (role == Qt::UserRole && proxyIndex.column() == 0)
  {
    const int HIDE_PROGRESS = -1;
    int progress = HIDE_PROGRESS;

    if (m_pendingInformation.contains(segmentation))
    {
      InformationFetcherSPtr task = m_pendingInformation[segmentation];

      progress = task->hasFinished()?HIDE_PROGRESS:task->currentProgress();
    }

    return progress;
  }

  if (role == Qt::BackgroundRole)
  {
    auto disabled = !m_pendingInformation.contains(segmentation) ||!m_pendingInformation[segmentation]->hasFinished();

    return disabled?Qt::lightGray:QAbstractProxyModel::data(proxyIndex, role);
  }

  if (role == Qt::DisplayRole && !m_keys.isEmpty())
  {
    auto key = m_keys[proxyIndex.column()];

    if (NameKey() == key)
    {
      return segmentation->data(role);
    }

    if (CategoryKey() == key)
    {
      return segmentation->category()->data(role);
    }

    if (segmentation->hasInformation(key))
    {
      if (!m_pendingInformation.contains(segmentation) || m_pendingInformation[segmentation]->isAborted())
      {
        auto sas =  AppositionSurfacePlugin::segmentationSAS(segmentation);

        auto task = std::make_shared<SASInformationFetcher>(segmentation, sas, m_keys, m_scheduler);
        m_pendingInformation[segmentation] = task;

        if (!task->hasFinished()) // If all information is available on constructor, it is set as finished
        {
          connect(task.get(), SIGNAL(progress(int)),
                  this, SLOT(onProgessReported(int)));
          connect(task.get(), SIGNAL(finished()),
                  this, SLOT(onTaskFininished()));
          //qDebug() << "Launching Task";
          Task::submit(task);
        } else // we avoid overloading the scheduler
        {
          return segmentation->information(key);
        }
      } else if (m_pendingInformation[segmentation]->hasFinished())
      {
        return segmentation->information(key);
      }

      return "";
    }
    else if(key.extension() == AppositionSurfaceExtension::TYPE)
    {
      auto sas = AppositionSurfacePlugin::segmentationSAS(segmentation);

      if(sas->hasInformation(key))
      {
        if (!m_pendingInformation.contains(segmentation) || m_pendingInformation[segmentation]->isAborted())
        {
          auto task = std::make_shared<SASInformationFetcher>(segmentation, sas, m_keys, m_scheduler);
          m_pendingInformation[segmentation] = task;

          if (!task->hasFinished()) // If all information is available on constructor, it is set as finished
          {
            connect(task.get(), SIGNAL(progress(int)),
                    this,       SLOT(onProgessReported(int)));
            connect(task.get(), SIGNAL(finished()),
                    this,       SLOT(onTaskFininished()));
            //qDebug() << "Launching Task";
            Task::submit(task);
          }
          else // we avoid overloading the scheduler
          {
            return sas->information(key);
          }
        }
        else
          if (m_pendingInformation[segmentation]->hasFinished())
          {
            return sas->information(key);
          }

        return "";
      }
    }
    else
    {
      return tr("Unavailable");
    }
  }
  else if (proxyIndex.column() > 0)
  {
      return QVariant();//To avoid checkrole or other roles
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}