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
#include "DistanceInformationDialog.h"
#include "DistanceInformationTabularReport.h"
#include <GUI/Dialogs/DefaultDialogs.h>
#include <Support/Settings/Settings.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/Spatial.h>
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/MultiTasking/Scheduler.h>
#include <Extensions/Morphological/MorphologicalInformation.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QSpacerItem>
#include <QProgressBar>
#include <QApplication>

// C++
#include <cmath>

// VTK
#include <vtkSmartPointer.h>
#include <vtkDistancePolyDataFilter.h>
#include <vtkPointData.h>

using ESPINA::Core::SegmentationExtension;
using ESPINA::Extensions::MorphologicalInformation;

using namespace ESPINA;
using namespace ESPINA::GUI;

const QString SETTINGS_GROUP = "Distance Information Report";

//----------------------------------------------------------------------------
DistanceInformationDialog::DistanceInformationDialog(const SegmentationAdapterList selectedSegmentations,
                                                     const DistanceInformationOptionsDialog::Options options,
                                                     Support::Context &context)
: QDialog             (DefaultDialogs::defaultParentWidget(), Qt::WindowStaysOnTopHint)
, Support::WithContext(context)
, m_segmentations     (selectedSegmentations)
, m_options           (options)
, m_i                 {0}
, m_j                 {0}
, m_iMax              {m_segmentations.isEmpty() ? context.model()->segmentations().size() : m_segmentations.size()}
, m_jMax              {context.model()->segmentations().size()}
, m_finished          {false}
{
  setWindowTitle(tr("Distance Information Report"));
  setWindowIconText(":/espina/Espina.svg");
  setLayout(new QVBoxLayout());

  layout()->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

  auto label = new QLabel("Computing distances.", this, 0);
  label->setAlignment(Qt::AlignHCenter);
  layout()->addWidget(label);

  m_progress = new QProgressBar(this);
  m_progress->setValue(0);

  layout()->addWidget(m_progress);

  layout()->addItem(new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum));

  auto cancelButton = new QDialogButtonBox(QDialogButtonBox::Cancel);

  connect(cancelButton, SIGNAL(rejected()),
          this,         SLOT(cancelComputations()));

  layout()->addWidget(cancelButton);

  window()->resize(layout()->sizeHint());
  window()->adjustSize();

  auto numSegs = context.model()->segmentations().size();

  int numComputations;

  if(m_segmentations.isEmpty())
  {
    numComputations = (numSegs * (numSegs-1))/2;
  }
  else
  {
    auto givenSegsNum = m_segmentations.size();
    numComputations = (givenSegsNum * (numSegs - 1)) - ((givenSegsNum * (givenSegsNum - 1))/2);
  }

  m_progress->setMaximum(numComputations);

  for(unsigned int i = 0; i < Scheduler::maxRunningTasks(); ++i)
  {
    computeNextDistance();
  }
}

//----------------------------------------------------------------------------
void DistanceInformationDialog::computeNextDistance()
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

  auto segmentations            = getModel()->segmentations();
  SegmentationAdapterPtr first  = nullptr;
  SegmentationAdapterPtr second = nullptr;

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

    auto thread = new DistanceComputationThread(first, second, m_options.distanceInformationType, getContext());
    connect(thread, SIGNAL(finished()), this, SLOT(finishedComputation()));

    m_threads << thread;

    thread->start();
  }
}

//----------------------------------------------------------------------------
void DistanceInformationDialog::cancelComputations()
{
  QWriteLocker lock(&m_lock);

  for(auto thread: m_threads)
  {
    disconnect(thread, SIGNAL(finished()), this, SLOT(finishedComputation()));

    delete thread;
  }

  m_threads.clear();

  close();
}

//----------------------------------------------------------------------------
void DistanceInformationDialog::finishedComputation()
{
  if(m_threads.isEmpty()) return;

  QApplication::processEvents();

  QWriteLocker lock(&m_lock);

  auto thread = qobject_cast<DistanceComputationThread *>(sender());

  if(thread && m_threads.contains(thread))
  {
    m_distances[thread->first()][thread->second()] = thread->distance();
    m_distances[thread->second()][thread->first()] = thread->distance();

    m_threads.removeOne(thread);

    delete thread;

    m_progress->setValue(m_progress->value() + 1);

    computeNextDistance();
  }
  else
  {
    qWarning("DistanceInformationDialog::finishedComputation() -> Unknown thread");
  }

  if(m_threads.isEmpty())
  {
    while(layout()->count() != 0)
    {
      layout()->removeItem(layout()->itemAt(0));
    }

    auto report = new DistanceInformationTabularReport(getContext(), m_segmentations, m_options, m_distances);

    layout()->addWidget(report);

    auto acceptButton = new QDialogButtonBox(QDialogButtonBox::Ok);

    connect(acceptButton, SIGNAL(accepted()),
            this,         SLOT(accept()));

    layout()->addWidget(acceptButton);

    window()->resize(layout()->sizeHint());
    window()->adjustSize();
  }
}

//----------------------------------------------------------------------------
const bool DistanceInformationDialog::isDistanceCached(SegmentationAdapterPtr first,
                                                       SegmentationAdapterPtr second) const

{
  // NOTE: if m_distances[first][second] exists then m_distances[second][first] exists too. If there is a value then
  // has been computed or is being computed.
  if(m_distances.keys().contains(first) && m_distances[first].keys().contains(second))
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
DistanceComputationThread::DistanceComputationThread(SegmentationAdapterPtr first,
                                                     SegmentationAdapterPtr second,
                                                     const DistanceInformationOptionsDialog::DistanceType type,
                                                     Support::Context      &context)
: QThread()
, m_context           (context)
, m_distance          {0}
, m_first             {first}
, m_second            {second}
, m_type              {type}
{
}

// TODO solve interlocking problem.

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
    auto mesh1 = readLockMesh(m_first->output())->mesh();
    auto mesh2 = readLockMesh(m_second->output())->mesh();

    auto distanceFilter = vtkSmartPointer<vtkDistancePolyDataFilter>::New();
    distanceFilter->SignedDistanceOff();
    distanceFilter->SetInputData(0, mesh1);
    distanceFilter->SetInputData(1, mesh2);
    distanceFilter->Update();

    m_distance = distanceFilter->GetOutput()->GetPointData()->GetScalars()->GetRange()[0];
  }
}

//----------------------------------------------------------------------------
const NmVector3 DistanceComputationThread::getCentroid(SegmentationAdapterPtr seg)
{
  auto extensions = seg->extensions();

  if(!extensions->hasExtension(MorphologicalInformation::TYPE))
  {
    auto extension = m_context.factory()->createSegmentationExtension(MorphologicalInformation::TYPE);
    extensions->add(extension);
  }

  auto morphologicalExtension = extensions->get<MorphologicalInformation>();

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
