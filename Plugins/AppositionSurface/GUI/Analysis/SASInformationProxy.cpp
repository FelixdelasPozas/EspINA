/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

// Plugin
#include "SASInformationProxy.h"

// EspINA
#include <Core/Analysis/Extension.h>
#include <GUI/Model/Proxies/InformationProxy.h>

// Qt
#include <QString>

using namespace EspINA;

const QString SAS = QObject::tr("SAS");

class SASInformationProxy::SASInformationFetcher
: public InformationProxy::InformationFetcher
{
public:
  SASInformationFetcher(SegmentationAdapterPtr segmentation,
                        SegmentationAdapterPtr sas,
                        const SegmentationExtension::InfoTagList &tags,
                        SchedulerSPtr scheduler)
  : InformationProxy::InformationFetcher(segmentation, tags, scheduler)
  , m_sas         {sas}
  {
    auto id = Segmentation->data(Qt::DisplayRole).toString();
    setDescription(tr("%1 information").arg(id));
    setHidden(true);

    m_tags.removeOne(NAME_TAG);
    m_tags.removeOne(CATEGORY_TAG);

    bool ready = true;
    for (auto tag : m_tags)
    {
      if(tag.startsWith("SAS "))
      {
        auto sasTag = QString(tag).remove(0,4);
        ready &= m_sas->isInformationReady(sasTag);
      }
      else
        ready &= Segmentation->isInformationReady(tag);

      if (!ready) break;
    }

    setFinished(ready);
  }

  SegmentationAdapterPtr m_sas;

protected:
  virtual void run()
  {
    for (int i = 0; i < m_tags.size(); ++i)
    {
      if (!canExecute()) break;

      auto tag = m_tags[i];
      if (tag != NAME_TAG && tag != CATEGORY_TAG)
      {
        if(tag.startsWith(tr("SAS ")))
        {
          auto sasTag = QString(tag).remove(0,4);

          if(!m_sas->isInformationReady(sasTag))
          {
            setWaiting(true);
            m_sas->information(sasTag);
            setWaiting(false);
            if (!canExecute()) break;
          }
        }
        else
        {
          if (!Segmentation->isInformationReady(tag))
          {
            setWaiting(true);
            Segmentation->information(tag);
            setWaiting(false);
            if (!canExecute()) break;
          }
        }
      }

      m_progress = (100.0*i)/m_tags.size();
      emit progress(m_progress);
    }
  }
};

//----------------------------------------------------------------------------
QVariant SASInformationProxy::data(const QModelIndex& proxyIndex, int role) const
{
  if (!proxyIndex.isValid())
    return QVariant();

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
    if (!m_pendingInformation.contains(segmentation) ||!m_pendingInformation[segmentation]->hasFinished())
    {
      return Qt::lightGray;
    } else
    {
      return QAbstractProxyModel::data(proxyIndex, role);
    }
  }

  if (role == Qt::DisplayRole && !m_tags.isEmpty())
  {
    auto tag = m_tags[proxyIndex.column()];

    if (NAME_TAG == tag)
    {
      return segmentation->data(role);
    }

    if (CATEGORY_TAG == tag)
    {
      return segmentation->category()->data(role);
    }

    if (segmentation->informationTags().contains(tag))
    {
      if (!m_pendingInformation.contains(segmentation) || m_pendingInformation[segmentation]->isAborted())
      {
        auto sasItem = m_model->relatedItems(segmentation, RelationType::RELATION_OUT, SAS).first().get();
        auto sas = segmentationPtr(sasItem);

        InformationFetcherSPtr task{new SASInformationFetcher(segmentation, sas, m_tags, m_scheduler)};
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
          return segmentation->information(tag);
        }
      } else if (m_pendingInformation[segmentation]->hasFinished())
      {
        return segmentation->information(tag);
      }

      return "";
    }
    else if(tag.startsWith(tr("SAS ")))
    {
      auto sasItem = m_model->relatedItems(segmentation, RelationType::RELATION_OUT, SAS).first().get();
      auto sas = segmentationPtr(sasItem);
      auto sasTag = QString(tag).remove(0,4);

      if(sas->informationTags().contains(sasTag))
      {
        if (!m_pendingInformation.contains(segmentation) || m_pendingInformation[segmentation]->isAborted())
        {
          InformationFetcherSPtr task{new SASInformationFetcher(segmentation, sas, m_tags, m_scheduler)};
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
            return sas->information(sasTag);
          }
        } else if (m_pendingInformation[segmentation]->hasFinished())
        {
          return sas->information(sasTag);
        }

        return "";
      }
    }
    else
    {
      return tr("Unavailable");
    }
  } else if (proxyIndex.column() > 0)
    return QVariant();//To avoid checkrole or other roles

    return QAbstractProxyModel::data(proxyIndex, role);
}
