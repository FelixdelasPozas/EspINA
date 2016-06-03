/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

// ESPINA
#include "EdgesAnalyzer.h"
#include "ChannelEdges.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>

// ITK
#include <itkImageRegionIterator.h>

// Qt
#include <QDebug>

// C++
#include <algorithm>
#include <cstdlib>

using namespace ESPINA;

//------------------------------------------------------------------------
EdgesAnalyzer::EdgesAnalyzer(ChannelEdges *extension,
                             SchedulerSPtr  scheduler)
: Task                 {scheduler}
, m_useDistanceToBounds{0}
, m_bgIntensity        {0}
, m_bgThreshold        {0}
, m_extension          {extension}
{
}

//------------------------------------------------------------------------
EdgesAnalyzer::~EdgesAnalyzer()
{
}

//------------------------------------------------------------------------
void EdgesAnalyzer::run()
{
//  qDebug() << "Analyzing Adaptive Edges" << m_extension->m_extendedItem->name();
  auto volume = readLockVolume(m_extension->m_extendedItem->output());

  m_useDistanceToBounds = 0;
  m_bgIntensity         = 0;
  m_bgThreshold         = 0;

  analyzeEdge(volume, leftSliceBounds(volume));
  reportProgress(25);
  analyzeEdge(volume, rightSliceBounds(volume));
  reportProgress(50);
  analyzeEdge(volume, topSliceBounds(volume));
  reportProgress(75);
  analyzeEdge(volume, bottomSliceBounds(volume));
  reportProgress(99);

  if (!isAborted())
  {
    reportProgress(100);

    const int NUM_EDGES = 4;
    m_extension->m_useDistanceToBounds = (m_useDistanceToBounds == NUM_EDGES);

    if (!m_extension->m_useDistanceToBounds)
    {
      m_extension->m_backgroundColor = m_bgIntensity / (NUM_EDGES - m_useDistanceToBounds);
      m_extension->m_threshold       = (m_bgThreshold < 10 ? 10 : m_bgThreshold);
    }
  }

  m_extension->m_hasAnalizedChannel = !isAborted();
  m_extension->m_analisysWait.wakeAll();
//  qDebug() << "Adaptive Edges Analyzed" << m_extension->m_extendedItem->name();
}

//------------------------------------------------------------------------
void EdgesAnalyzer::analyzeEdge(const Output::ReadLockData<DefaultVolumetricData> &volume, const Bounds& edgeBounds)
{
  using Intensity = int;
  using Frequency = long long int;

  auto image  = volume->itkImage(edgeBounds);

  auto it = itk::ImageRegionConstIterator<itkVolumeType>(image, image->GetLargestPossibleRegion());
  it.GoToBegin();

  QMap<Intensity, Frequency> borderIntensityFrequency;

  Frequency numVoxels = 0;
  while(canExecute() && !it.IsAtEnd())
  {
    ++borderIntensityFrequency[it.Value()];

    ++numVoxels;
    ++it;
  }

  int       bgIntensity   = 0;
  Frequency quarterVoxels = numVoxels/4;

  QList<Intensity> frequentIntensities;
  for (auto intensity : borderIntensityFrequency.keys())
  {
    if (!canExecute()) break;

    if (borderIntensityFrequency[intensity] > quarterVoxels)
    {
      bgIntensity += intensity;
      frequentIntensities << intensity;
    }
  }

  if (!frequentIntensities.isEmpty())
  {
    for(auto intensity: frequentIntensities)
    {
      m_bgThreshold = std::max(m_bgThreshold, std::abs(intensity - frequentIntensities.first()));
    }

    m_bgIntensity += (bgIntensity / frequentIntensities.size());
  }
  else
  {
    ++m_useDistanceToBounds;
  }
}
