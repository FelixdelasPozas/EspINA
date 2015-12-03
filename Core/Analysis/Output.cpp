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

// ESPINA
#include "Output.h"
#include "Filter.h"
#include "DataProxy.h"
#include "Analysis.h"
#include "Segmentation.h"

// VTK
#include <vtkMath.h>

using namespace ESPINA;

const int ESPINA::Output::INVALID_OUTPUT_ID = -1;

TimeStamp Output::s_tick = 0;

//----------------------------------------------------------------------------
Output::Output(FilterPtr filter, const Output::Id& id, const NmVector3 &spacing)
: m_mutex{QMutex::Recursive}
, m_filter{filter}
, m_id{id}
, m_spacing{spacing}
, m_timeStamp{s_tick++}
{
}

//----------------------------------------------------------------------------
Output::~Output()
{
}

//----------------------------------------------------------------------------
void Output::setSpacing(const NmVector3& spacing)
{
  if (m_spacing != spacing)
  {
    for(auto type : m_data.keys())
    {
      auto data = writeLockData<Data>(type);
      data->setSpacing(spacing);
    }

    // NOTE: spacing change must be set after propagating it to the data
    // so the data can get the old spacing to scale if needed.
    m_spacing = spacing;
    updateModificationTime();
  }
}

//----------------------------------------------------------------------------
NmVector3 Output::spacing() const
{
  return m_spacing;
}

//----------------------------------------------------------------------------
Snapshot Output::snapshot(TemporalStorageSPtr storage,
                          QXmlStreamWriter   &xml,
                          const QString      &path) const
{
  Snapshot snapshot;

  auto saveOutput = isSegmentationOutput();

  for(auto data : m_data)
  {
    auto bounds = data->bounds();

    xml.writeStartElement("Data");
    xml.writeAttribute("type",    data->type());
    xml.writeAttribute("bounds",  bounds.bounds().toString());

    for(int i = 0; i < data->editedRegions().size(); ++i)
    {
      // We need to crop the edited regions bounds in case
      // the data bounds have been reduced to prevent
      // out of bounds data requests
      auto editedBounds = intersection(bounds, data->editedRegions().at(i));
      if (bounds.areValid() && editedBounds.areValid())
      {
        xml.writeStartElement("EditedRegion");
        xml.writeAttribute("id",     QString::number(i));
        xml.writeAttribute("bounds", editedBounds.toString());
        xml.writeEndElement();
      }
    }
    xml.writeEndElement();

    auto snapshotId = QString::number(id());
    if (saveOutput)
    {
      if (!data->isValid())
      {
        data->fetchData();
      }
      snapshot << data->snapshot(storage, path, snapshotId);
    }
    else
    {
      // Similarly to the output data is the case of the edited regions
      // we need to copy the existing ones
      // Alternatively, until the other implementation is available, we
      // just restore them so they are available on save
      if (!data->isValid())
      {
        data->restoreEditedRegions(storage, path, snapshotId);
      }
      snapshot << data->editedRegionsSnapshot(storage, path, snapshotId);
    }
  }

  return snapshot;
}

//----------------------------------------------------------------------------
Bounds Output::bounds() const
{
  Bounds bounds;

  for(auto data : m_data)
  {
    if (data->isValid() && data->bounds().areValid())
    {
      if (bounds.areValid())
      {
        bounds = boundingBox(bounds, data->bounds());
      }
      else
      {
        bounds = data->bounds();
      }
    }
  }

  return bounds;
}

//----------------------------------------------------------------------------
void Output::clearEditedRegions()
{
  for(auto data: m_data)
  {
    data->clearEditedRegions();
  }
}

//----------------------------------------------------------------------------
bool Output::isEdited() const
{
  for(auto data: m_data)
  {
    if (data->isEdited()) return true;
  }

  return false;
}

//----------------------------------------------------------------------------
bool Output::isValid() const
{
  if (m_filter == nullptr) return false;

  if (m_id == INVALID_OUTPUT_ID) return false;

  for(auto data : m_data)
  {
    if (!data->isValid()) return false;
  }

  return !m_data.isEmpty();
}

//----------------------------------------------------------------------------
void Output::updateModificationTime()
{
  m_timeStamp = s_tick++;

  emit modified();;
}

//----------------------------------------------------------------------------
void Output::onDataChanged()
{
  updateModificationTime();
}

//----------------------------------------------------------------------------
void Output::setData(Output::DataSPtr data)
{
  Data::Type type = data->type();

  if (!m_data.contains(type))
  {
    m_data[type] = data->createProxy();
  }

  proxy(type)->set(data);

  updateModificationTime();

  connect(data.get(), SIGNAL(dataChanged()),
          this,       SLOT(onDataChanged()));
}

//----------------------------------------------------------------------------
void Output::removeData(const Data::Type& type)
{
  m_data.remove(type);
}

//----------------------------------------------------------------------------
bool Output::hasData(const Data::Type& type) const
{
  return m_data.contains(type);
}

//----------------------------------------------------------------------------
unsigned int Output::numberOfDatas() const
{
  unsigned int result = 0;
  for(auto data : m_data)
  {
    if (data->isValid())
    {
      ++result;
    }
  }

  return result;
}

//----------------------------------------------------------------------------
void Output::update()
{
  if (m_data.isEmpty())
  {
    m_filter->update();
  }
  else
  {
    for (auto data : m_data)
    {
      if (!data->isValid())
      {
        update(data->type());
      }
    }
  }
}

//----------------------------------------------------------------------------
void Output::update(const Data::Type &type)
{
  auto requestedData = data<Data>(type);

  if (!requestedData->isValid())
  {
    m_mutex.lock();

    if (!requestedData->fetchData())
    {
      auto dependencies = requestedData->dependencies();

      if(!dependencies.empty())
      {
        m_mutex.unlock();

        for (auto dependencyType : dependencies)
        {
          update(dependencyType);
        }

        update(type);

        m_mutex.lock();
      }
      else
      {
        m_filter->update();
      }
    }

    Q_ASSERT(requestedData->isValid());

    m_mutex.unlock();
  }
}

//----------------------------------------------------------------------------
bool Output::isSegmentationOutput() const
{
  auto analysis = m_filter->analysis();
  if (analysis)
  {
    auto content  = analysis->content();
    auto outEdges = content->outEdges(m_filter, QString::number(m_id));

    for (auto edge : outEdges)
    {
      if (std::dynamic_pointer_cast<Segmentation>(edge.target))
      {
        return true;
      }
    }
  }

  return false;
}

//----------------------------------------------------------------------------
DataProxy *Output::proxy(const Data::Type &type)
{
  auto base  = m_data.value(type).get();

  return dynamic_cast<DataProxy *>(base);
}
