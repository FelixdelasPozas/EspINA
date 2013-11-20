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
#include <QXmlStreamWriter>

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
Snapshot Output::snapshot() const
{
  Snapshot snapshot;

  QByteArray xml;
  QXmlStreamWriter stream(&xml);
  stream.setAutoFormatting(true);

  stream.writeStartDocument();
  for(auto dataProxy : m_data) 
  {
    DataSPtr data = dataProxy->get();

    stream.writeStartElement(data->type());
    stream.writeAttribute("Bounds", data->bounds().toString());
    stream.writeStartElement("Edited Regions");
    for(int i = 0; i < data->editedRegions().size(); ++i)
    {
      auto region = data->editedRegions()[i];
      stream.writeStartElement(QString::number(i));
      stream.writeAttribute("Bounds", region.toString());
      stream.writeEndElement();
    }
    stream.writeEndElement();
    stream.writeEndElement();

    if (m_hasToBeSaved)
    {
      snapshot << data->snapshot();
    }
    else
    {
      snapshot << data->editedRegionsSnapshot();
    }
  }
  stream.writeEndDocument();

  QString file = QString("Outputs/%1_%2.xml").arg(m_filter->uuid()).arg(m_id);
  snapshot << SnapshotData(file, xml);

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
  foreach(DataProxySPtr data, m_data)
  {
    data->get()->clearEditedRegions();
  }
}

//----------------------------------------------------------------------------
bool Output::isEdited() const
{
  foreach(DataProxySPtr data, m_data) 
  {
    if (!data->get()->isEdited()) return true;
  }

  return false;
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
void Output::onDataChanged()
{

}

//----------------------------------------------------------------------------
void Output::setData(Output::DataSPtr data)
{
  Data::Type type = data->type();

  if (!m_data.contains(type))
  {
    m_data[type] = data->createProxy();
  }

  m_data[type]->set(data);
  data->setOutput(this);
}

//----------------------------------------------------------------------------
void Output::removeData(const Data::Type& type)
{
  m_data.remove(type);
}

//----------------------------------------------------------------------------
Output::DataSPtr Output::data(const Data::Type& type) const
{
  DataSPtr result;

  if (m_data.contains(type))
    result = m_data.value(type)->get();

  return result;
}

//----------------------------------------------------------------------------
void Output::update()
{
  m_filter->update(m_id);
}