/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Factory/CoreFactory.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Data/VolumetricData.hxx>
#include <Extensions/Histogram/StackHistogram.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>

// Qt
#include <QFile>
#include <QDataStream>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const QString StackHistogram::TYPE = "StackHistogram";

//--------------------------------------------------------------------
State StackHistogram::state() const
{
  return State();
}

//--------------------------------------------------------------------
Snapshot StackHistogram::snapshot() const
{
  Snapshot snapshot;

  if(!m_histogram.isEmpty())
  {
    auto name = snapshotName(tr("Histogram.txt"));
    QByteArray data;
    QDataStream dataStream(&data, QIODevice::WriteOnly);
    dataStream.setVersion(QDataStream::Qt_4_0);

    for(int i = 0; i < 256; ++i)
    {
      dataStream << m_histogram.values(i);
    }

    snapshot << SnapshotData(name, data);
    data.clear();
  }

  return snapshot;
}

//--------------------------------------------------------------------
const Core::StackExtension::TypeList StackHistogram::dependencies() const
{
  TypeList list;

  list << ChannelEdges::TYPE;

  return list;
}

//--------------------------------------------------------------------
StackHistogram::StackHistogram(CoreFactory* factory)
: Core::StackExtension{InfoCache()}
, m_factory{factory}
{
}

//--------------------------------------------------------------------
const QString StackHistogram::toolTipText() const
{
  return tr("Histogram");
}

//--------------------------------------------------------------------
void StackHistogram::checkHistogramValidity()
{
  QMutexLocker lock(&m_lock);

  if(m_histogram.isEmpty())
  {
    auto volume = readLockVolume(m_extendedItem->output());
    const auto region = volume->itkRegion();
    const auto origin = volume->itkOriginalOrigin();
    const auto spacing = volume->itkSpacing();
    const auto vOrigin = NmVector3{origin[0], origin[1], origin[2]};
    const auto vSpacing = NmVector3{spacing[0], spacing[1], spacing[2]};
    int progressValue = 0;

    auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(m_extendedItem, m_factory);

    emit progress(0);

    for(auto z = region.GetIndex(2); z < region.GetIndex(2) + static_cast<long int>(region.GetSize(2)); ++z)
    {
      auto sliceRegion = edgesExtension->sliceRegion(z - region.GetIndex(2));
      auto sliceImage = volume->itkImage(equivalentBounds<itkVolumeType>(vOrigin, vSpacing, sliceRegion));
      itk::ImageRegionConstIterator<itkVolumeType> it(sliceImage, sliceRegion);
      it.GoToBegin();
      while(!it.IsAtEnd())
      {
        m_histogram.addValue(it.Value());
        ++it;
      }

      int newProgress = (100*(z - region.GetIndex(2)))/region.GetSize(2);
      if(newProgress != progressValue)
      {
        progressValue = newProgress;
        emit progress(progressValue);
      }
    }

    m_histogram.update();
    emit progress(100);
  }
}

//--------------------------------------------------------------------
void ESPINA::Extensions::StackHistogram::loadSnapshot()
{
  auto name = snapshotName(tr("Histogram.txt"));

  if(m_extendedItem->storage()->exists(name))
  {
    auto file = m_extendedItem->storage()->absoluteFilePath(name);
    QFile handle{file};
    if(handle.exists() && handle.open(QIODevice::ReadOnly))
    {
      auto data = handle.readAll();

      QDataStream dataStream(&data, QIODevice::ReadOnly);
      dataStream.setVersion(QDataStream::Qt_4_0);
      unsigned long long count;

      for(int i = 0; i < 256; ++i)
      {
        dataStream >> count;

        for(unsigned long long j = 0; j < count; ++j) m_histogram.addValue(static_cast<unsigned char>(i));
      }

      m_histogram.update();
      handle.close();
    }
  }
}
