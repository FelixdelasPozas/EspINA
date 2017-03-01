/*
 * Copyright (C) 2016, Rafael Juan Vicente Garcia <rafaelj.vicente@gmail.com>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "DistanceInformationReport.h"
#include <Dialogs/DistanceInformation/DistanceInformationDialog.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/MultiTasking/Scheduler.h>
#include <Extensions/Morphological/MorphologicalInformation.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/QueryAdapter.h>

// C++
#include <cmath>

// VTK
#include <vtkSmartPointer.h>
#include <vtkImplicitPolyDataDistance.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>

// Qt
#include <QMutex>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI;

//----------------------------------------------------------------------------
DistanceInformationReport::DistanceInformationReport(Support::Context& context)
: WithContext(context)
{
}

//----------------------------------------------------------------------------
QString DistanceInformationReport::name() const
{
  return tr("Distance Information");
}

//----------------------------------------------------------------------------
QString DistanceInformationReport::description() const
{
  return tr("Computes and reports the distance between segmentations in the stack.\n\n" \
            "The distance can be computed from centroid to centroid or from surface to surface. The report can have a single table or an individual table for each segmentation." \
            "If there aren't any selected segmentations the distances will be computed for all segmentations in the stack.");
}

//----------------------------------------------------------------------------
SegmentationAdapterList DistanceInformationReport::acceptedInput(SegmentationAdapterList segmentations) const
{
  return segmentations;
}

//----------------------------------------------------------------------------
QString DistanceInformationReport::requiredInputDescription() const
{
  return QString();
}

//----------------------------------------------------------------------------
void DistanceInformationReport::show(SegmentationAdapterList segmentations) const
{
  if(getModel()->segmentations().size() < 2)
  {
    DefaultDialogs::ErrorMessage(tr("There should be at least two segmentations to compute distances."));
  }
  else
  {
    DistanceInformationOptionsDialog optionsDialog(getContext());

    if (optionsDialog.exec() == QDialog::Accepted)
    {
      auto options = optionsDialog.getOptions();

      if(options.category != CategoryAdapterSPtr())
      {
        auto segmentations = QueryAdapter::segmentationsOfCategory(getModel(), options.category);
        if(segmentations.isEmpty())
        {
          DefaultDialogs::ErrorMessage(tr("There aren't any segmentations in the selected category '%1'.").arg(options.category->name()));
          return;
        }
      }

      auto task = std::make_shared<DistanceComputationManagerThread>(segmentations, optionsDialog.getOptions(), getContext());
      task->setDescription(tr("Segmentation's distances computation."));

      connect(task.get(), SIGNAL(finished()), this, SLOT(onComputationFinished()));

      {
        QMutexLocker lock(&m_mutex);

        m_tasks << task;
      }

      Task::submit(task);
    }
  }
}

//--------------------------------------------------------------------
void DistanceInformationReport::onComputationFinished()
{
  auto task = dynamic_cast<DistanceComputationManagerThread *>(sender());

  if(!task)
  {
    qWarning("DistanceInformationReport::onComputationFinished() -> Unknown thread");
    return; // this shouldn't happen.
  }

  if(!task->isAborted())
  {
    auto dialog = new DistanceInformationDialog(task->getSegmentations(), task->getDistances(), task->getOptions(), getContext());
    dialog->setAttribute(Qt::WA_DeleteOnClose, true);
    dialog->show();
  }

  QMutexLocker lock(&m_mutex);

  for(auto storedTask: m_tasks)
  {
    if(storedTask.get() == task)
    {
      m_tasks.removeOne(storedTask);
      return;
    }
  }
}

//----------------------------------------------------------------------------
DistanceComputationThread::DistanceComputationThread(SegmentationAdapterPtr first,
                                                     SegmentationAdapterPtr second,
                                                     const DistanceInformationOptionsDialog::DistanceType type,
                                                     Support::Context &context,
                                                     DistanceComputationManagerThread *manager)
: Task      {context.scheduler()}
, m_context (context)
, m_distance{VTK_DOUBLE_MAX}
, m_first   {first}
, m_second  {second}
, m_type    {type}
, m_manager {manager}
{
  Q_ASSERT(first != nullptr && second != nullptr);
  setDescription(QObject::tr("Compute distance from %1 to %2.").arg(first->data().toString()).arg(second->data().toString()));
  setHidden(true);
}

//----------------------------------------------------------------------------
void DistanceComputationThread::run()
{
  if(m_type == DistanceInformationOptionsDialog::DistanceType::CENTROID)
  {
    auto centroid1 = getCentroid(m_first);
    auto c1x = centroid1[0];
    auto c1y = centroid1[1];
    auto c1z = centroid1[2];

    auto centroid2 = getCentroid(m_second);
    auto c2x = centroid2[0];
    auto c2y = centroid2[1];
    auto c2z = centroid2[2];

    m_distance = std::sqrt(std::pow(c1x - c2x, 2) + std::pow(c1y - c2y, 2) + std::pow(c1z - c2z, 2));
  }
  else
  {
    auto from = readLockMesh(m_first->output())->mesh();
    auto to   = readLockMesh(m_second->output())->mesh();

    if ((from->GetNumberOfPolys() == 0) || (from->GetNumberOfPoints() == 0) ||
        (to->GetNumberOfPolys() == 0)   || (to->GetNumberOfPoints() == 0))
    {
      m_distance = -1;
      return;
    }

    auto distanceFilter = vtkSmartPointer<vtkImplicitPolyDataDistance>::New();
    distanceFilter->SetInput(to);

    int numPts = from->GetNumberOfPoints();

    for (vtkIdType ptId = 0; ptId < numPts; ++ptId)
    {
      double pt[3];
      from->GetPoint(ptId, pt);
      try
      {
        auto dist = std::fabs(distanceFilter->EvaluateFunction(pt));

        if(m_distance > dist)
        {
          m_distance = dist;
        }
      }
      catch(...)
      {
        m_distance = -1;
        break;
      }
    }
  }

  if(canExecute())
  {
    m_manager->m_waiter.wakeAll();
  }
}

//----------------------------------------------------------------------------
const NmVector3 DistanceComputationThread::getCentroid(SegmentationAdapterPtr seg)
{
  auto morphologicalExtension = retrieveOrCreateSegmentationExtension<MorphologicalInformation>(seg, m_context.factory());

  bool isOk;
  const QStringList tagList = {"Centroid X","Centroid Y","Centroid Z"};

  NmVector3 centroid;

  for(int i = 0; i < 3; i++)
  {
    auto key    = SegmentationExtension::InformationKey(MorphologicalInformation::TYPE, tagList.at(i));
    auto qValue = morphologicalExtension->information(key);
    auto value  = qValue.toDouble(&isOk);

    if(!isOk)
    {
      qWarning() << "DistanceComputationThread::getCentroid() -> Unexpected variant value.";
    }

    centroid[i] = value;
  }

  return centroid;
}

//--------------------------------------------------------------------
DistanceComputationManagerThread::DistanceComputationManagerThread(const SegmentationAdapterList selectedSegmentations,
                                                                   const DistanceInformationOptionsDialog::Options options,
                                                                   Support::Context &context)
: Task             {context.scheduler()}
, m_segmentations  (selectedSegmentations)
, m_options        (options)
, m_i              {0}
, m_j              {0}
, m_iMax           {m_segmentations.isEmpty() ? context.model()->segmentations().size() : m_segmentations.size()}
, m_jMax           {options.category == CategoryAdapterSPtr() ? context.model()->segmentations().size() : QueryAdapter::segmentationsOfCategory(context.model(), options.category).size()}
, m_numComputations{0}
, m_computations   {0}
, m_finished       {false}
, m_context        (context)
{
}

//--------------------------------------------------------------------
DistanceComputationManagerThread::~DistanceComputationManagerThread()
{
  QMutexLocker lock(&m_lock);

  for(auto task: m_tasks)
  {
    disconnect(task.get(), SIGNAL(finished()), this, SLOT(onComputationFinished()));
    task->abort();
  }

  m_tasks.clear();
}

//--------------------------------------------------------------------
void DistanceComputationManagerThread::run()
{
  if(m_segmentations.isEmpty())
  {
    m_numComputations = (m_iMax * (m_iMax-1))/2;
  }
  else
  {
    m_numComputations = (m_iMax * (m_jMax - 1)) - ((m_iMax * (m_iMax - 1))/2);
  }

  for(unsigned int i = 0; i < Scheduler::maxRunningTasks(); ++i)
  {
    computeNextDistance();
  }

  // wait until finished or cancelled
  while((!m_finished || !m_tasks.isEmpty()) && canExecute())
  {
    m_waitMutex.lock();
    m_waiter.wait(&m_waitMutex);
    m_waitMutex.unlock();

    QApplication::processEvents();
  }
}

//--------------------------------------------------------------------
void DistanceComputationManagerThread::onComputationFinished()
{
  if(!canExecute())
  {
    m_finished = true;

    for(auto task: m_tasks)
    {
      disconnect(task.get(), SIGNAL(finished()), this, SLOT(onComputationFinished()));
      task->abort();
    }

    m_tasks.clear();

    return;
  }

  QMutexLocker lock(&m_lock);

  auto thread = qobject_cast<DistanceComputationThread *>(sender());

  if(thread)
  {
    for(auto task: m_tasks)
    {
      if(task.get() == thread)
      {
        m_distances[thread->first()][thread->second()] = thread->distance();
        m_distances[thread->second()][thread->first()] = thread->distance();

        m_tasks.removeOne(task);
        ++m_computations;

        reportProgress(static_cast<int>((100 * m_computations)/m_numComputations));

        computeNextDistance();
        break;
      }
    }
  }
  else
  {
    qWarning("DistanceComputationManagerThread::onComputationFinished() -> Unknown thread");
  }
}

//--------------------------------------------------------------------
void DistanceComputationManagerThread::computeNextDistance()
{
  auto incrementCheck = [this] ()
  {
    if(this->m_j == this->m_jMax)
    {
      this->m_i = this->m_i + 1;
      this->m_j = 0;
    }

    if (this->m_i == this->m_iMax)
    {
      this->m_finished = true;
    }
  };

  SegmentationAdapterSList segmentations;
  SegmentationAdapterPtr   first  = nullptr;
  SegmentationAdapterPtr   second = nullptr;

  if(m_options.category == CategoryAdapterSPtr())
  {
    segmentations = m_context.model()->segmentations();
  }
  else
  {
    segmentations = QueryAdapter::segmentationsOfCategory(m_context.model(), m_options.category);
  }

  while((first == second) && !m_finished)
  {
    incrementCheck();

    if(!m_finished)
    {
      first = m_segmentations.isEmpty() ? segmentations.at(m_i).get() : m_segmentations.at(m_i);
      second = segmentations.at(m_j).get();

      while((first == second) || isDistanceCached(first, second))
      {
        ++m_j;

        incrementCheck();

        if(m_finished) break;

        first = m_segmentations.isEmpty() ? segmentations.at(m_i).get() : m_segmentations.at(m_i);
        second = segmentations.at(m_j).get();
      }
    }
  }

  if(!m_finished)
  {
    m_distances[first][second] = m_distances[second][first] = VTK_DOUBLE_MAX;

    auto task = std::make_shared<DistanceComputationThread>(first, second, m_options.distanceType, m_context, this);

    connect(task.get(), SIGNAL(finished()), this, SLOT(onComputationFinished()));

    m_tasks << task;

    Task::submit(task);
  }
}

//--------------------------------------------------------------------
DistanceInformationDialog::DistancesMap DistanceComputationManagerThread::getDistances() const
{
  QMutexLocker lock(&m_lock);

  return m_distances;
}

//--------------------------------------------------------------------
const bool DistanceComputationManagerThread::isDistanceCached(SegmentationAdapterPtr first, SegmentationAdapterPtr second) const
{
  // NOTE: if m_distances[first][second] exists then m_distances[second][first] exists too. If there is a value then
  // has been computed or is being computed.
  if(m_distances.keys().contains(first) && m_distances[first].keys().contains(second))
  {
    return true;
  }

  return false;
}
