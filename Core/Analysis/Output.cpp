/*

 Copyright (C) 2014 Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "Output.h"
#include "Filter.h"
#include "DataProxy.h"
#include "Analysis.h"
#include "Segmentation.h"
#include <Core/Utils/EspinaException.h>

// VTK
#include <vtkMath.h>

// Qt
#include <QString>

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
const NmVector3 Output::spacing() const
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
    const auto bounds = data->bounds();
    auto dependencies = QStringList{data->dependencies()};

    xml.writeStartElement("Data");
    xml.writeAttribute("type",    data->type());
    xml.writeAttribute("bounds",  bounds.bounds().toString());
    xml.writeAttribute("depends", dependencies.join(";"));

    if(bounds.areValid())
    {
      for(int i = 0; i < data->editedRegions().size(); ++i)
      {
        // We need to crop the edited regions bounds in case
        // the data bounds have been reduced to prevent
        // out of bounds data requests
        if(!intersect(bounds, data->editedRegions().at(i))) continue;

        auto editedBounds = intersection(bounds, data->editedRegions().at(i));
        if (editedBounds.areValid())
        {
          xml.writeStartElement("EditedRegion");
          xml.writeAttribute("id",     QString::number(i));
          xml.writeAttribute("bounds", editedBounds.toString());
          xml.writeEndElement();
        }
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

  emit modified();
}

//----------------------------------------------------------------------------
void Output::onDataChanged()
{
  updateModificationTime();
}

//----------------------------------------------------------------------------
void Output::setData(Output::DataSPtr data)
{
  const auto type = data->type();

  if (!m_data.contains(type))
  {
    m_data[type] = data->createProxy();
  }

  proxy(type)->set(data);

  connect(data.get(), SIGNAL(dataChanged()),
          this,       SLOT(onDataChanged()));

  updateModificationTime();
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

  m_mutex.lock();

  if (!requestedData->isValid())
  {
    if (!requestedData->fetchData())
    {
      auto dependencies = requestedData->dependencies();

      if(!dependencies.empty())
      {
        // TODO: this can make another thread to enter and request an
        // update to the same data. FIX.
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

    if(!requestedData->isValid())
    {
      m_mutex.unlock();

      auto message = tr("Invalid %1 data updating output from filter %2 (%3)").arg(type).arg(this->filter()->name()).arg(filter()->uuid().toString());
      auto details = tr("Output::update() -> ") + message;

      throw Core::Utils::EspinaException(message, details);
    }
  }

  m_mutex.unlock();
}

//----------------------------------------------------------------------------
bool Output::isSegmentationOutput() const
{
  auto analysis = m_filter->analysis();
  if (analysis)
  {
    auto content  = analysis->content();
    auto outEdges = content->outEdges(m_filter, QString::number(m_id));

    auto booleanOp = [](const DirectedGraph::Edge &edge) { return (nullptr != std::dynamic_pointer_cast<Segmentation>(edge.target)); };
    auto exists = std::any_of(outEdges.begin(), outEdges.end(), booleanOp);

    return exists;
  }

  return false;
}

//----------------------------------------------------------------------------
DataProxy *Output::proxy(const Data::Type &type)
{
  auto base  = m_data.value(type).get();

  return dynamic_cast<DataProxy *>(base);
}
