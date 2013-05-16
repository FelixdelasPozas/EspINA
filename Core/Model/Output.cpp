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
#include "OutputRepresentation.h"
#include "Filter.h"
#include <vtkMath.h>

using namespace EspINA;

const int EspINA::FilterOutput::INVALID_OUTPUT_ID = -1;

//----------------------------------------------------------------------------
FilterOutput::FilterOutput(Filter *filter, const FilterOutputId &id)
: m_id(id)
, m_isCached(false)
, m_filter(filter)
, m_region(0, -1, 0, -1, 0, -1)
{
  // FIXME
  //if (isValid()) this->volume->toITK()->DisconnectPipeline();
}

//----------------------------------------------------------------------------
void FilterOutput::update()
{
  m_filter->update(m_id);
}

//----------------------------------------------------------------------------
void FilterOutput::setRegion(const EspinaRegion &region)
{
  m_region = region;
}

//----------------------------------------------------------------------------
void FilterOutput::onRepresentationChanged()
{
  emit modified();
}


//----------------------------------------------------------------------------
ChannelOutput::ChannelOutput(Filter *filter, const FilterOutputId &id)
: FilterOutput(filter, id)
{
}

//----------------------------------------------------------------------------
bool ChannelOutput::isValid() const
{
  bool validData = !m_representations.isEmpty();

  int i = 0;
  while (validData && i < m_representations.size())
  {
    validData = m_representations[m_representations.keys()[i]]->isValid();
    ++i;
  }

  return NULL != m_filter
      && INVALID_OUTPUT_ID < m_id
      && validData;
}


//----------------------------------------------------------------------------
SegmentationOutput::SegmentationOutput(Filter *filter, const FilterOutputId &id)
: FilterOutput(filter, id)
{
}

//----------------------------------------------------------------------------
bool SegmentationOutput::dumpSnapshot(const QString &prefix, Snapshot &snapshot, bool saveEditedRegions)
{
  bool dumped = false;

  int oldSize = m_editerRegions.size();

  std::ostringstream outputInfo;
  foreach(SegmentationRepresentationSPtr rep, m_representations)
  {
    outputInfo << rep->type().toStdString() << std::endl;
    if (isCached())
    {
      dumped |= rep->dumpSnapshot(prefix, snapshot);
    }

    if (saveEditedRegions)
    {
      // We don't need to make a copy of latest modifications of cached representations data
      // becasue they are implictly stored by dumpSnapshot. We just save the edited regions
      // in case the output is not referenced by a segmentation in a future
      // NOTE: Verify that the order of the regions doesn't matter
      rep->commitEditedRegions(!isCached());
    }
  }

  if (saveEditedRegions)
  {
    outputInfo << std::endl; // Empty line separates representation types from region info


    int regionId = 0;
    foreach(EditedRegionSPtr editedRegion, m_editerRegions)
    {
      outputInfo << editedRegion->Name.toStdString() << " ";
      for (int i = 0; i < 6; ++i)
      {
        outputInfo << editedRegion->Region[i] << " ";
      }
      outputInfo << std::endl;

      dumped |= editedRegion->dump(m_filter->cacheDir(), QString("%1_%2").arg(prefix).arg(regionId++) , snapshot);
    }
  }

  // Because current representations can be modified and saved again,
  // we don't commit them definetely
  while (m_editerRegions.size() > oldSize)
  {
    m_editerRegions.pop_back();
  }

  QString outputInfoFile = QString("Outputs/%1.trc").arg(prefix);
  snapshot << SnapshotEntry(outputInfoFile, outputInfo.str().c_str());

  return dumped;
}

// //----------------------------------------------------------------------------
// bool SegmentationOutput::fetchSnapshot(const QString &prefix)
// {
//   bool fetched = true;
// 
//   foreach(SegmentationRepresentationSPtr rep, m_representations)
//   {
//     fetched &= rep->fetchSnapshot(m_filter, prefix);
//   }
// 
//   return fetched;
// }

//----------------------------------------------------------------------------
bool SegmentationOutput::isValid() const
{
  bool validData = !m_representations.isEmpty();

  int i = 0;
  while (validData && i < m_representations.size())
  {
    validData = m_representations[m_representations.keys()[i]]->isValid();
    ++i;
  }

  return NULL != m_filter
      && INVALID_OUTPUT_ID < m_id
      && validData;
}

//----------------------------------------------------------------------------
bool SegmentationOutput::isEdited() const
{
  bool editedData = false;

  int i = 0;
  while (!editedData && i < m_representations.size())
  {
    editedData = m_representations[m_representations.keys()[i]]->isEdited();
    ++i;
  }

  return editedData;
}

//----------------------------------------------------------------------------
void SegmentationOutput::push(FilterOutput::EditedRegionSList editedRegions)
{
  m_editerRegions << editedRegions;
}

//----------------------------------------------------------------------------
void SegmentationOutput::clearEditedRegions()
{
  m_editerRegions.clear();
  foreach(SegmentationRepresentationSPtr rep, m_representations)
  {
    rep->clearEditedRegions();
  }
}

//----------------------------------------------------------------------------
void SegmentationOutput::dumpEditedRegions(const QString &prefix, Snapshot &snapshot)
{
  //FIXME
  std::ostringstream regions;

  int oldSize = m_editerRegions.size();
  // Verify that the order of the regions doesn't matter
  // We need to add regions that haven't been
  foreach(SegmentationRepresentationSPtr rep, m_representations)
  {
    rep->commitEditedRegions(!isCached());
  }

  foreach(EditedRegionSPtr editedRegion, m_editerRegions)
  {
    regions << editedRegion->Name.toStdString() << " ";
    for (int i = 0; i < 6; ++i)
    {
      regions << editedRegion->Region[i] << " ";
    }
    regions << std::endl;

    editedRegion->dump(m_filter->cacheDir(), prefix, snapshot);
  }

  snapshot << SnapshotEntry(prefix + ".trc", regions.str().c_str());

  while (m_editerRegions.size() > oldSize)
  {
    m_editerRegions.pop_back();
  }
}

//----------------------------------------------------------------------------
SegmentationOutput::EditedRegionSList SegmentationOutput::editedRegions() const
{
  return m_editerRegions;
}

//----------------------------------------------------------------------------
void SegmentationOutput::restoreEditedRegions(const QDir &cacheDir, const QString &ouptutId)
{
  foreach(SegmentationRepresentationSPtr rep, m_representations)
  {
    rep->restoreEditedRegions(cacheDir, ouptutId);
  }
}

//----------------------------------------------------------------------------
void SegmentationOutput::setEditedRegions(FilterOutput::EditedRegionSList regions)
{
  clearEditedRegions();
  //FIXME set regions
}

//----------------------------------------------------------------------------
void SegmentationOutput::setRepresentation(const FilterOutput::OutputRepresentationName &name,
                                           SegmentationRepresentationSPtr representation)
{
  if (m_representations.contains(name))
  {
    disconnect(m_representations[name].get(), SIGNAL(representationChanged()),
               this, SLOT(onRepresentationChanged()));
  }

  if (representation.get()) {
    m_representations[name] = representation;

    connect(m_representations[name].get(), SIGNAL(representationChanged()),
            this, SLOT(onRepresentationChanged()));
  } else 
  {
    m_representations.remove(name);
  }
}
