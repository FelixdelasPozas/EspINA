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
#include <Core/Analysis/Extensions.h>
#include <GUI/Model/Proxies/InformationProxy.h>
#include <GUI/Model/Utils/SegmentationUtils.h>

// Qt
#include <QString>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::GUI::Model::Utils;


//----------------------------------------------------------------------------
bool isSASInformation(SegmentationExtension::InformationKey& key)
{
  return key.extension() == AppositionSurfaceExtension::TYPE;
}


//----------------------------------------------------------------------------
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
      auto id = m_segmentation->data(Qt::DisplayRole).toString();
      setDescription(tr("%1 information").arg(id));
      setHidden(true);

      m_keys.removeOne(NameKey());
      m_keys.removeOne(CategoryKey());

      bool ready = true;
      for (auto key : m_keys)
      {
        if (m_sas && isSASInformation(key))
        {
          ready &= m_sas->isReady(key);
        }
        else
        {
          if(m_segmentation)
          {
            ready &= m_segmentation->isReady(key);
          }
        }

        if (!ready) break;
      }

      setFinished(ready);
    }

  protected:
    virtual void run()
    {
      for (int i = 0; i < m_keys.size(); ++i)
      {
        if (!canExecute()) break;

        auto key = m_keys[i];

        if (key != NameKey() && key != CategoryKey())
        {
          if (isSASInformation(key))
          {
            if (m_sas)
            {
              updateInformation(m_sas, key);
              if (!canExecute()) break;
            }
          }
          else
          {
            updateInformation(m_segmentation, key);
            if (!canExecute()) break;
          }
        }

        reportProgress((100.0*i)/m_keys.size());
      }
    }

    void updateInformation(SegmentationAdapterPtr segmentation, SegmentationExtension::InformationKey &key)
    {
      if(!segmentation->isReady(key))
      {
        segmentation->information(key);
      }
    }

  private:
    SegmentationAdapterPtr m_sas;
};

//----------------------------------------------------------------------------
QVariant SASInformationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid()) return QVariant();

  auto proxyItem = itemAdapter(proxyIndex);

  if (!isSegmentation(proxyItem)) return QVariant();

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

      progress = task->hasFinished() ? HIDE_PROGRESS : task->currentProgress();
    }

    return progress;
  }

  if(role == Qt::ForegroundRole || role == Qt::BackgroundRole)
  {
    if(!m_pendingInformation.contains(segmentation) || !m_pendingInformation[segmentation]->hasFinished())
    {
      return Qt::lightGray;
    }

    auto info = data(proxyIndex, Qt::DisplayRole);
    if(info.canConvert(QVariant::String) && (info.toString().contains("Fail", Qt::CaseInsensitive) || info.toString().contains("Error", Qt::CaseInsensitive)))
    {
      return role == Qt::ForegroundRole ? Qt::white : Qt::red;
    }

    return QAbstractProxyModel::data(proxyIndex, role);
  }

  if(role == Qt::ToolTipRole)
  {
    auto key = m_keys[proxyIndex.column()];
    auto sas = AppositionSurfacePlugin::segmentationSAS(segmentation);
    if(!sas && isSASInformation(key))
    {
      return tr("No SAS has been created for %1.").arg(segmentation->data().toString());
    }

    return QString();
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

    if (segmentation->hasInformation(key) || isSASInformation(key))
    {
      auto sas =  AppositionSurfacePlugin::segmentationSAS(segmentation);

      if (!m_pendingInformation.contains(segmentation) || m_pendingInformation[segmentation]->isAborted())
      {
        auto task = std::make_shared<SASInformationFetcher>(segmentation, sas, m_keys, m_scheduler);
        m_pendingInformation[segmentation] = task;

        if (!task->hasFinished()) // If all information is available on constructor, it is set as finished
        {
          connect(task.get(), SIGNAL(progress(int)),
                  this,       SLOT(onProgressReported(int)));
          connect(task.get(), SIGNAL(finished()),
                  this,       SLOT(onTaskFininished()));

          Task::submit(task);
        }
        else // we avoid overloading the scheduler
        {
          return information(segmentation, sas, key);
        }
      }
      else
      {
        if (m_pendingInformation[segmentation]->hasFinished())
        {
          return information(segmentation, sas, key);
        }
        else
        {
          return "";
        }
      }
    }
    else
    {
      return tr("Unavailable");
    }
  }
  else if (proxyIndex.column() > 0)
  {
    return QVariant(); //To avoid checkrole or other roles
  }

  return QAbstractProxyModel::data(proxyIndex, role);
}

//----------------------------------------------------------------------------
QVariant SASInformationProxy::information(SegmentationAdapterPtr segmentation,
                                          SegmentationAdapterPtr sas,
                                          SegmentationExtension::InformationKey& key) const
{
  if (isSASInformation(key))
  {
    if (sas)
    {
      return sas->information(key);
    }
    else
    {
      return tr("Unavailable SAS");
    }
  }
  else
  {
    return segmentation->information(key);
  }
}
