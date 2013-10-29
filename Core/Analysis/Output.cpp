/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "Output.h"

#include "Core/Analysis/Filter.h"
#include "Core/Analysis/DataProxy.h"

#include <vtkMath.h>

using namespace EspINA;

const int EspINA::Output::INVALID_OUTPUT_ID = -1;

TimeStamp Output::s_tick = 0;

//----------------------------------------------------------------------------
Output::Output(FilterPtr filter, const Output::Id& id)
: m_filter{filter}
, m_id{id}
, m_timeStamp{s_tick++}
, m_hasToBeSaved{false}
{

}

//----------------------------------------------------------------------------
Snapshot Output::snapshot()
{
  Snapshot snapshot;

  std::ostringstream stream;

  foreach(DataProxySPtr dataProxy, m_data) 
  {
    DataSPtr data = dataProxy->get();

    stream << data->type().toStdString() << " " << data->bounds() << "\n";
    foreach(Bounds region, data->editedRegions())
    {
      stream << region << "\n";
    }
    stream << "\n";

    if (m_hasToBeSaved)
    {
      snapshot << data->snapshot();
    }
    else
    {
      snapshot << data->editedRegionsSnapshot();
    }
  }

  QString file = QString("Outputs/%1_%2.trc").arg(m_filter->uuid()).arg(m_id);
  snapshot << SnapshotData(file, stream.str().c_str());

  return snapshot;
}

//----------------------------------------------------------------------------
Bounds Output::bounds() const
{

}

//----------------------------------------------------------------------------
void Output::clearEditedRegions()
{
  foreach(DataProxySPtr data, m_data)
  {
    data->get()->clearEditedRegions();
  }
}

// void Output::dumpEditedRegions(const QString& prefix, Snapshot& snapshot)
// {
// 
// }

// bool Output::dumpSnapshot(const QString& prefix, Snapshot& snapshot, bool saveEditedRegions)
// {
// 
// }

// Output::EditedRegionSList Output::editedRegions() const
// {
// 
// }

bool Output::isEdited() const
{

}

bool Output::isValid() const
{
  if (m_filter == nullptr) return false;

  if (m_id == INVALID_OUTPUT_ID) return false;

  foreach(DataProxySPtr data, m_data) 
  {
    if (!data->get()->isValid()) return false;
  }

  return !m_data.isEmpty();
}

void Output::onDataChanged()
{

}

// void Output::restoreEditedRegions(const QDir& cacheDir, const QString& ouptutId)
// {
// 
// }

void Output::setData(Output::DataSPtr data)
{
  Data::Type type = data->type();

  if (data.get())
  {
    m_data.remove(type);
  } else
  {
    if (!m_data.contains(type))
    {
      m_data[type] = data->createProxy();
    }

    m_data[type]->set(data);
    data->setOutput(this);
  }
}

Output::DataSPtr Output::data(const Data::Type& type) const
{
  DataSPtr result;

  if (m_data.contains(type))
    result = m_data.value(type)->get();

  return result;
}


// void Output::setEditedRegions(Output::EditedRegionSList regions)
// {
// 
// }

void Output::update()
{
  m_filter->update(m_id);
}


// //----------------------------------------------------------------------------
// void FilterOutput::onRepresentationChanged()
// {
//   emit modified();
// }
// 
// 
// //----------------------------------------------------------------------------
// ChannelOutput::ChannelOutput(Filter *filter, const FilterOutputId &id)
// : FilterOutput(filter, id)
// {
// }
// 
// //----------------------------------------------------------------------------
// EspinaRegion ChannelOutput::region() const
// {
//   EspinaRegion bb;
// 
//   bool first = true;
//   foreach (ChannelRepresentationSPtr representation, m_representations)
//   {
//     if (first)
//     {
//       bb    = representation->representationBounds();
//       first = false;
//     } else
//     {
//       bb = BoundingBox(bb, representation->representationBounds());
//     }
//   }
// 
//   return bb;
// }
// 
// 
// //----------------------------------------------------------------------------
// SegmentationOutput::SegmentationOutput(Filter *filter, const FilterOutputId &id)
// : FilterOutput(filter, id)
// {
// }
// 
// //----------------------------------------------------------------------------
// bool SegmentationOutput::dumpSnapshot(const QString &prefix, Snapshot &snapshot, bool saveEditedRegions)
// {
//   bool dumped = false;
// 
//   int oldSize = m_editerRegions.size();
// 
//   std::ostringstream outputInfo;
//   foreach(SegmentationRepresentationSPtr rep, m_representations)
//   {
//     outputInfo << rep->type().toStdString() << std::endl;
//     if (isCached())
//     {
//       dumped |= rep->dumpSnapshot(prefix, snapshot);
//     }
// 
//     if (saveEditedRegions)
//     {
//       // We don't need to make a copy of latest modifications of cached representations data
//       // becasue they are implictly stored by dumpSnapshot. We just save the edited regions
//       // in case the output is not referenced by a segmentation in a future
//       // NOTE: Verify that the order of the regions doesn't matter
//       rep->commitEditedRegions(!isCached());
//     }
//   }
// 
//   if (saveEditedRegions)
//   {
//     outputInfo << std::endl; // Empty line separates representation types from region info
// 
// 
//     int regionId = 0;
//     foreach(EditedRegionSPtr editedRegion, m_editerRegions)
//     {
//       outputInfo << editedRegion->Name.toStdString() << " ";
//       for (int i = 0; i < 6; ++i)
//       {
//         outputInfo << editedRegion->Region[i] << " ";
//       }
//       outputInfo << std::endl;
// 
//       dumped |= editedRegion->dump(m_filter->cacheDir(), QString("%1_%2").arg(prefix).arg(regionId++) , snapshot);
//     }
//   }
// 
//   // Because current representations can be modified and saved again,
//   // we don't commit them definetely
//   while (m_editerRegions.size() > oldSize)
//   {
//     m_editerRegions.pop_back();
//   }
// 
//   QString outputInfoFile = QString("Outputs/%1.trc").arg(prefix);
//   snapshot << SnapshotEntry(outputInfoFile, outputInfo.str().c_str());
// 
//   return dumped;
// }
// 
// //----------------------------------------------------------------------------
// EspinaRegion SegmentationOutput::region() const
// {
//   EspinaRegion bb;
// 
//   bool first = true;
//   foreach (SegmentationRepresentationSPtr representation, m_representations)
//   {
//     if (first)
//     {
//       bb    = representation->representationBounds();
//       first = false;
//     } else
//     {
//       bb = BoundingBox(bb, representation->representationBounds());
//     }
//   }
// 
//   return bb;
// }
// 
// 
// //----------------------------------------------------------------------------
// bool SegmentationOutput::isEdited() const
// {
//   bool editedData = false;
// 
//   int i = 0;
//   while (!editedData && i < m_representations.size())
//   {
//     editedData = m_representations[m_representations.keys()[i]]->isEdited();
//     ++i;
//   }
// 
//   return editedData;
// }
// 
// //----------------------------------------------------------------------------
// void SegmentationOutput::push(FilterOutput::EditedRegionSList editedRegions)
// {
//   m_editerRegions << editedRegions;
// }
// 
// //----------------------------------------------------------------------------
// void SegmentationOutput::clearEditedRegions()
// {
//   m_editerRegions.clear();
//   foreach(SegmentationRepresentationSPtr rep, m_representations)
//   {
//     rep->clearEditedRegions();
//   }
// }
// 
// //----------------------------------------------------------------------------
// void SegmentationOutput::dumpEditedRegions(const QString &prefix, Snapshot &snapshot)
// {
//   //FIXME
//   std::ostringstream regions;
// 
//   int oldSize = m_editerRegions.size();
//   // Verify that the order of the regions doesn't matter
//   // We need to add regions that haven't been
//   foreach(SegmentationRepresentationSPtr rep, m_representations)
//   {
//     rep->commitEditedRegions(!isCached());
//   }
// 
//   foreach(EditedRegionSPtr editedRegion, m_editerRegions)
//   {
//     regions << editedRegion->Name.toStdString() << " ";
//     for (int i = 0; i < 6; ++i)
//     {
//       regions << editedRegion->Region[i] << " ";
//     }
//     regions << std::endl;
// 
//     editedRegion->dump(m_filter->cacheDir(), prefix, snapshot);
//   }
// 
//   snapshot << SnapshotEntry(prefix + ".trc", regions.str().c_str());
// 
//   while (m_editerRegions.size() > oldSize)
//   {
//     m_editerRegions.pop_back();
//   }
// }
// 
// //----------------------------------------------------------------------------
// SegmentationOutput::EditedRegionSList SegmentationOutput::editedRegions() const
// {
//   return m_editerRegions;
// }
// 
// //----------------------------------------------------------------------------
// void SegmentationOutput::restoreEditedRegions(const QDir &cacheDir, const QString &ouptutId)
// {
//   foreach(SegmentationRepresentationSPtr rep, m_representations)
//   {
//     rep->restoreEditedRegions(cacheDir, ouptutId);
//   }
// }
// 
// //----------------------------------------------------------------------------
// void SegmentationOutput::setEditedRegions(FilterOutput::EditedRegionSList regions)
// {
//   clearEditedRegions();
//   //FIXME set regions
// }