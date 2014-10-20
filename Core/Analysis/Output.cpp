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
Output::Output(FilterPtr filter, const Output::Id& id)
: m_filter{filter}
, m_id{id}
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
    m_spacing = spacing;

    for(auto data : m_data)
    {
      if (data->get()->isValid())
      {
        data->get()->setSpacing(spacing);
      }
    }

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

  bool saveOutput = hasToBeSaved();

  for(auto dataProxy : m_data)
  {
    DataSPtr data = dataProxy->get();

    xml.writeStartElement("Data");
    xml.writeAttribute("type",    data->type());
    xml.writeAttribute("bounds",  data->bounds().toString());

    for(int i = 0; i < data->editedRegions().size(); ++i)
    {
      auto region = data->editedRegions()[i];
      xml.writeStartElement("Edited Region");
      xml.writeAttribute("id",     QString::number(i));
      xml.writeAttribute("bounds", region.toString());
      xml.writeEndElement();
    }
    xml.writeEndElement();

    auto snapshotId = QString::number(id());
    if (saveOutput)
    {
      snapshot << data->snapshot(storage, path, snapshotId);
    }
    else
    {
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
    if (bounds.areValid())
    {
      bounds = boundingBox(bounds, data->get()->bounds());
    } else
    {
      bounds = data->get()->bounds();
    }
  }

  return bounds;
}

//----------------------------------------------------------------------------
void Output::clearEditedRegions()
{
  for(auto data: m_data)
  {
    data->get()->clearEditedRegions();
  }
}

//----------------------------------------------------------------------------
bool Output::isEdited() const
{
  for(auto data: m_data)
  {
    if (data->get()->isEdited()) return true;
  }

  return false;
}

//----------------------------------------------------------------------------
bool Output::isValid() const
{
  if (m_filter == nullptr) return false;

  if (m_id == INVALID_OUTPUT_ID) return false;

  for(DataProxySPtr data : m_data)
  {
    if (!data->get()->isValid()) return false;
  }

  return !m_data.isEmpty();
}

//----------------------------------------------------------------------------
void Output::onDataChanged()
{
  emit modified();
}

//----------------------------------------------------------------------------
void Output::setData(Output::DataSPtr data)
{
  Data::Type type = data->type();

  if (m_data.contains(type))
  {
    auto oldData = m_data[type]->get();
    disconnect(oldData.get(), SIGNAL(dataChanged()),
               this, SLOT(onDataChanged()));
  }
  else
  {
    m_data[type] = data->createProxy();
  }

  m_data[type]->set(data);
  data->setOutput(this);

  updateModificationTime();
  emit modified();

  connect(data.get(), SIGNAL(dataChanged()),
          this, SLOT(onDataChanged()));
}

//----------------------------------------------------------------------------
void Output::removeData(const Data::Type& type)
{
  m_data.remove(type);
}

//----------------------------------------------------------------------------
Output::DataSPtr Output::data(const Data::Type& type) const
{
  m_filter->update(m_id);

  DataSPtr result;

  if (m_data.contains(type))
    result = m_data.value(type)->get();

  return result;
}

//----------------------------------------------------------------------------
bool Output::hasData(const Data::Type& type) const
{
  return m_data.contains(type);
}


//----------------------------------------------------------------------------
void Output::update()
{
  m_filter->update(m_id);
}

//----------------------------------------------------------------------------
bool Output::hasToBeSaved() const
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
