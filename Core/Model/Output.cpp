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
#include "OutputType.h"
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
bool FilterOutput::dumpSnapshot(const QString &prefix, Snapshot &snapshot)
{
  bool dumped = true;

  foreach(OutputTypeSPtr data, m_data)
  {
    dumped |= data->dumpSnapshot(prefix, snapshot);
  }

  return dumped;
}

//----------------------------------------------------------------------------
bool FilterOutput::fetchSnapshot(const QString &prefix)
{
  bool fetched = true;

  foreach(OutputTypeSPtr outputData, m_data)
  {
    fetched &= outputData->fetchSnapshot(prefix);
  }

  return fetched;
}

//----------------------------------------------------------------------------
bool FilterOutput::isValid() const
{
  bool validData = !m_data.isEmpty();

  int i = 0;
  while (validData && i < m_data.size())
  {
    validData = m_data[m_data.keys()[i]]->isValid();
    ++i;
  }

  return NULL != m_filter
      && INVALID_OUTPUT_ID < m_id
      && validData;
}

//----------------------------------------------------------------------------
bool FilterOutput::isEdited() const
{
  bool editedData = false;

  int i = 0;
  while (!editedData && i < m_data.size())
  {
    editedData = m_data[m_data.keys()[i]]->isEdited();
    ++i;
  }

  return editedData;
}

//----------------------------------------------------------------------------
void FilterOutput::clearEditedRegions()
{
  foreach(OutputTypeSPtr outputData, m_data)
  {
    outputData->clearEditedRegions();
  }
}

//----------------------------------------------------------------------------
void FilterOutput::dumpEditedRegions(const QString &prefix, Snapshot &snapshot)
{
  //FIXME

//   std::ostringstream regions;
// 
//   for (int r = 0; r < m_data; ++r)
//   {
//     Output::NamedRegion editedRegion = output->editedRegions[r];
//     
//     regions << editedRegion.first << " ";
//     for (int i = 0; i < 6; ++i)
//     {
//       regions << editedRegion.second[i] << " ";
//     }
//     regions << std::endl;
//   }
// 
//   snapshot << SnapshotEntry(prefix + ".trc", regions.str().c_str());
}

//----------------------------------------------------------------------------
FilterOutput::NamedRegionList FilterOutput::editedRegions() const
{
  NamedRegionList regions;

  foreach (OutputTypeSPtr data, m_data)
  {
    regions << data->editedRegions();
  }

  return regions;
}

//----------------------------------------------------------------------------
void FilterOutput::restoreEditedRegions(const QString &prefix)
{
  //FIXME
}

//----------------------------------------------------------------------------
void FilterOutput::setEditedRegions(const FilterOutput::NamedRegionList &regions)
{
  clearEditedRegions();
  //FIXME set regions
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
