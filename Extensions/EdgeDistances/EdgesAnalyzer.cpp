/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "EdgesAnalyzer.h"

#include "ChannelEdges.h"
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/VolumetricData.h>
#include <Core/Analysis/Data/VolumetricDataUtils.h>
#include <itkImageRegionIterator.h>


#include <QDebug>

using namespace EspINA;

//------------------------------------------------------------------------
EdgesAnalyzer::EdgesAnalyzer(ChannelEdges *extension,
                             SchedulerSPtr  scheduler)
: Task(scheduler)
, m_extension(extension)
, m_useDistanceToBounds(true)
, m_bgIntensity(0)
{
}

//------------------------------------------------------------------------
EdgesAnalyzer::~EdgesAnalyzer()
{
}

//------------------------------------------------------------------------
void EdgesAnalyzer::run()
{
  qDebug() << "Analyzing Adaptive Edges" << m_extension->m_extendedItem->name();
  auto volume  = volumetricData(m_extension->m_extendedItem->output());
  auto bounds  = volume->bounds();
  auto spacing = volume->spacing();

  m_useDistanceToBounds = 0;

  analyzeEdge(volume, leftSliceBounds(volume));
  emit progress(25);
  analyzeEdge(volume, rightSliceBounds(volume));
  emit progress(50);
  analyzeEdge(volume, topSliceBounds(volume));
  emit progress(75);
  analyzeEdge(volume, bottomSliceBounds(volume));
  emit progress(99);

  if (!isAborted())
  {
    emit progress(100);

    const int NUM_EDGES = 4;
    m_extension->m_useDistanceToBounds = m_useDistanceToBounds == NUM_EDGES;

    if (!m_extension->m_useDistanceToBounds)
    {
      m_extension->m_backgroundColor = m_bgIntensity / (NUM_EDGES - m_useDistanceToBounds);
      m_extension->m_threshold       = 10;
    }
  }

  m_extension->m_analyzeEdgesMutex.unlock();
}

//------------------------------------------------------------------------
void EdgesAnalyzer::analyzeEdge(DefaultVolumetricDataSPtr volume, const Bounds& edgeBounds)
{
  using Intensity = int;
  using Frequency = long long int;

  auto image  = volume->itkImage(edgeBounds);

  auto it = itk::ImageRegionIterator<itkVolumeType>(image, image->GetLargestPossibleRegion());
  it.GoToBegin();

  QMap<Intensity, Frequency> borderIntensityFrequency;

  Frequency numVoxels = 0;
  while(canExecute() && !it.IsAtEnd())
  {
    borderIntensityFrequency[it.Value()]++;
    numVoxels++;

    ++it;
  }

  int       bgIntensity = 0;
  Frequency halfVoxels  = numVoxels/2;

  QList<Intensity> frequentIntensities;
  for (auto intensity : borderIntensityFrequency.keys())
  {
    if (!canExecute()) break;

    if (borderIntensityFrequency[intensity] > halfVoxels)
    {
      bgIntensity += intensity;
      frequentIntensities << intensity;
    }
  }

  if (!frequentIntensities.isEmpty())
  {
    m_bgIntensity = bgIntensity / frequentIntensities.size();
  } else
  {
    ++m_useDistanceToBounds;
  }
}
