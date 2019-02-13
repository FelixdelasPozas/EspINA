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
#include "Analysis.h"
#include <Core/Analysis/Sample.h>
#include <Core/Analysis/Filter.h>
#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Segmentation.h>
#include <Core/Utils/AnalysisUtils.h>
#include <Core/Utils/EspinaException.h>

using namespace ESPINA;
using namespace ESPINA::Core;

//------------------------------------------------------------------------
Analysis::Analysis()
: m_classification{nullptr}
, m_relations     {new DirectedGraph()}
, m_content       {new DirectedGraph()}
{
}

//------------------------------------------------------------------------
Analysis::~Analysis()
{
//   qDebug() << "Destroying Analysis";
}

//------------------------------------------------------------------------
void Analysis::clear()
{
  m_classification.reset();
  m_content   = DirectedGraphSPtr{new DirectedGraph()};
  m_relations = DirectedGraphSPtr{new DirectedGraph()};

  m_samples.clear();
  m_channels.clear();
  m_segmentations.clear();
  m_filters.clear();
  m_connections.clear();
}

//------------------------------------------------------------------------
void Analysis::setStorage(TemporalStorageSPtr storage)
{
  m_storage = storage;

  for (auto persistent : m_content->vertices())
  {
    persistent->setStorage(storage);
  }

  m_connections.setStorage(m_storage);
}

//------------------------------------------------------------------------
void Analysis::setClassification(ClassificationSPtr classification)
{
  m_classification = classification;
}

//------------------------------------------------------------------------
void Analysis::add(SampleSPtr sample)
{
  if (m_samples.contains(sample))
  {
    auto what    = QObject::tr("Attempt to add an already existing sample, sample: %1").arg(sample->name());
    auto details = QObject::tr("Analysis::add(sample) -> Attempt to add an already existing sample, sample: %1").arg(sample->name());

    throw Core::Utils::EspinaException(what, details);
  }

  m_samples << sample;

  m_content->add(sample);
  m_relations->add(sample);

  m_itemPointers.insert(sample, sample.get());
  m_itemUUids.insert(sample, sample->uuid());

  sample->setAnalysis(this);
}

//------------------------------------------------------------------------
void Analysis::add(SampleSList samples)
{
  for(auto sample: samples)
  {
    add(sample);
  }
}

//------------------------------------------------------------------------
void Analysis::add(ChannelSPtr channel)
{
  if (m_channels.contains(channel))
  {
    auto what    = QObject::tr("Attempt to add an already existing channel, channel: %1").arg(channel->name());
    auto details = QObject::tr("Analysis::add(channel) -> Attempt to add an already existing channel, channel: %1").arg(channel->name());

    throw Core::Utils::EspinaException(what, details);
  }

  m_channels << channel;

  FilterSPtr filter = channel->filter();

  addIfNotExists(filter);

  m_content->add(channel);

  RelationName relation = QString("%1").arg(channel->outputId());

  m_content->addRelation(filter, channel, relation);

  m_relations->add(channel);

  m_itemPointers.insert(channel, channel.get());
  m_itemUUids.insert(channel, channel->uuid());

  channel->setAnalysis(this);
}

//------------------------------------------------------------------------
void Analysis::add(ChannelSList channels)
{
  for(auto channel: channels)
  {
    add(channel);
  }
}

//------------------------------------------------------------------------
void Analysis::add(SegmentationSPtr segmentation)
{
  if (m_segmentations.contains(segmentation))
  {
    auto what    = QObject::tr("Attempt to add an already existing segmentation, segmentation: %1").arg(segmentation->name());
    auto details = QObject::tr("Analysis::add(segmentation) -> Attempt to add an already existing segmentation, segmentation: %1").arg(segmentation->name());

    throw Core::Utils::EspinaException(what, details);
  }

  m_segmentations << segmentation;

  FilterSPtr filter = segmentation->filter();

  addIfNotExists(filter);

  m_content->add(segmentation);

  addFilterContentRelation(filter, segmentation);

  m_relations->add(segmentation);

  m_itemPointers.insert(segmentation, segmentation.get());
  m_itemUUids.insert(segmentation, segmentation->uuid());

  segmentation->setAnalysis(this);
}

//------------------------------------------------------------------------
void Analysis::add(SegmentationSList segmentations)
{
  for(auto segmentation: segmentations)
  {
    add(segmentation);
  }
}

//------------------------------------------------------------------------
void Analysis::remove(SampleSPtr sample)
{
  if (!m_samples.contains(sample))
  {
    auto what    = QObject::tr("Attempt to delete an unknown sample, sample: %1").arg(sample->name());
    auto details = QObject::tr("Analysis::remove(sample) -> Attempt to delete an unknown sample, sample: %1").arg(sample->name());

    throw Core::Utils::EspinaException(what, details);
  }

  sample->setAnalysis(nullptr);
  m_samples.removeOne(sample);

  m_content->remove(sample);
  m_relations->remove(sample);
  m_itemPointers.remove(sample);
  m_itemUUids.remove(sample);
}

//------------------------------------------------------------------------
void Analysis::remove(SampleSList samples)
{
  for(auto sample: samples)
  {
    remove(sample);
  }
}


//------------------------------------------------------------------------
void Analysis::remove(ChannelSPtr channel)
{
  if (!m_channels.contains(channel))
  {
    auto what    = QObject::tr("Attempt to delete an unknown channel, channel: %1").arg(channel->name());
    auto details = QObject::tr("Analysis::remove(channel) -> Attempt to delete an unknown channel, channel: %1").arg(channel->name());

    throw Core::Utils::EspinaException(what, details);
  }

  channel->setAnalysis(nullptr);
  m_channels.removeOne(channel);

  m_content->remove(channel);
  m_relations->remove(channel);
  m_itemPointers.remove(channel);
  m_itemUUids.remove(channel);

  removeIfIsolated(channel->filter());
}

//------------------------------------------------------------------------
void Analysis::remove(ChannelSList channels)
{
  for(auto channel: channels)
  {
    remove(channel);
  }
}

//------------------------------------------------------------------------
void Analysis::remove(SegmentationSPtr segmentation)
{
  if (!m_segmentations.contains(segmentation))
  {
    auto what    = QObject::tr("Attempt to delete an unknown segmentation, segmentation: %1").arg(segmentation->name());
    auto details = QObject::tr("Analysis::remove(segmentation) -> Attempt to delete an unknown segmentation, segmentation: %1").arg(segmentation->name());

    throw Core::Utils::EspinaException(what, details);
  }

  segmentation->setAnalysis(nullptr);
  m_segmentations.removeOne(segmentation);

  m_content->remove(segmentation);
  m_relations->remove(segmentation);
  m_connections.removeSegmentation(segmentation);
  m_itemPointers.remove(segmentation);
  m_itemUUids.remove(segmentation);

  removeIfIsolated(segmentation->filter());
}

//------------------------------------------------------------------------
void Analysis::remove(SegmentationSList segmentations)
{
  for(auto segmentation: segmentations)
  {
    remove(segmentation);
  }
}

//------------------------------------------------------------------------
void Analysis::changeSpacing(ChannelSPtr channel, const NmVector3 &spacing)
{
  auto origin = channel->position();
  for (auto filter : downStreamPipeline(channel->filter()))
  {
    filter->changeSpacing(origin, spacing);
  }
}

//------------------------------------------------------------------------
void Analysis::addRelation(PersistentSPtr    ancestor,
                           PersistentSPtr    successor,
                           const RelationName& relation)
{
  if (!m_relations->contains(ancestor))
  {
    auto what    = QObject::tr("Attempt to add a relation to an unknown ancestor, item: %1").arg(ancestor->name());
    auto details = QObject::tr("Analysis::addRelation() -> Attempt add a relation to an unknown ancestor, item: %1").arg(ancestor->name());

    throw Core::Utils::EspinaException(what, details);
  }

  if (!m_relations->contains(successor))
  {
    auto what    = QObject::tr("Attempt to add a relation to an unknown successor, item: %1").arg(successor->name());
    auto details = QObject::tr("Analysis::addRelation() -> Attempt add a relation to an unknown successor, item: %1").arg(successor->name());

    throw Core::Utils::EspinaException(what, details);
  }

  if (findRelation(ancestor, successor, relation))
  {
    auto what    = QObject::tr("Attempt to add an existing relation, ancestor: %1, successor: %2, description: %3").arg(ancestor->name()).arg(successor->name()).arg(relation);
    auto details = QObject::tr("Analysis::addRelation() -> Attempt add an existing relation, ancestor: %1, successor: %2, description: %3").arg(ancestor->name()).arg(successor->name()).arg(relation);

    throw Core::Utils::EspinaException(what, details);
  }

  m_relations->addRelation(ancestor, successor, relation);
}

//------------------------------------------------------------------------
void Analysis::deleteRelation(PersistentSPtr    ancestor,
                              PersistentSPtr    successor,
                              const RelationName& relation)
{
  if (!findRelation(ancestor, successor, relation))
  {
    auto what    = QObject::tr("Attempt to delete an unknown relation, ancestor: %1, successor: %2, description: %3").arg(ancestor->name()).arg(successor->name()).arg(relation);
    auto details = QObject::tr("Analysis::deleteRelation() -> Attempt to remove an unknown relation, ancestor: %1, successor: %2, description: %3").arg(ancestor->name()).arg(successor->name()).arg(relation);

    throw Core::Utils::EspinaException(what, details);
  }

  m_relations->removeRelation(ancestor, successor, relation);
}

//------------------------------------------------------------------------
void Analysis::removeIfIsolated(FilterSPtr filter)
{
  if (removeIfIsolated(m_content, filter))
  {
    filter->setAnalysis(nullptr);
    m_filters.removeOne(filter);
  }
}

//------------------------------------------------------------------------
bool Analysis::removeIfIsolated(DirectedGraphSPtr graph, PersistentSPtr item)
{
  bool removed = false;

  if (graph->contains(item) && graph->outEdges(item).isEmpty())
  {
    graph->remove(item);
    removed = true;
  }

  return removed;
}

//------------------------------------------------------------------------
void Analysis::addIfNotExists(FilterSPtr filter)
{
  // NOTE: We could use m_filters instead to check if there is a copy in the content
  if (!m_content->contains(filter))
  {
    filter->setAnalysis(this);
    m_filters << filter;
    m_content->add(filter);

    for(int i = 0; i < filter->inputs().size(); ++i)
    {
      auto input       = filter->inputs()[i];
      auto inputFilter = input->filter();

      addIfNotExists(inputFilter);

      auto ancestor = find<Filter>(inputFilter.get(), m_filters);
      Q_ASSERT(ancestor);

      m_content->addRelation(ancestor, filter, QString("%1-%2").arg(i).arg(input->output()->id()));
    }
  }
}

//------------------------------------------------------------------------
FilterSList Analysis::downStreamPipeline(FilterSPtr filter)
{
  FilterSList inFilters;
  FilterSList outFilters;

  inFilters  << filter;
  outFilters << filter;

  while (!inFilters.isEmpty())
  {
    auto ancestor = inFilters.takeFirst();

    for(auto edge : m_content->outEdges(ancestor))
    {
      auto succesor = std::dynamic_pointer_cast<Filter>(edge.target);

      if (succesor)
      {
        inFilters  << succesor;
        outFilters << succesor;
      }
    }
  }

  return outFilters;
}

//------------------------------------------------------------------------
FilterSList Analysis::upStreamPipeline(FilterSPtr filter)
{
  FilterSList inFilters;
  FilterSList outFilters;

  inFilters  << filter;
  outFilters << filter;

  while (!inFilters.isEmpty())
  {
    auto ancestor = inFilters.takeFirst();

    for(auto edge : m_content->inEdges(ancestor))
    {
      auto predecessor = std::dynamic_pointer_cast<Filter>(edge.target);

      if (predecessor)
      {
        inFilters  << predecessor;
        outFilters << predecessor;
      }
    }
  }

  return outFilters;
}

//------------------------------------------------------------------------
void Analysis::addFilterContentRelation(FilterSPtr filter, ViewItem* item)
{
  ViewItemSPtr succesor;

  auto segmentation = dynamic_cast<Segmentation *>(item);
  if (segmentation)
  {
    succesor = find<Segmentation>(segmentation, m_segmentations);
  } else
  {
    auto channel = dynamic_cast<Channel *>(item);
    if (channel)
    {
      succesor = find<Channel>(channel, m_channels);
    }
  }
  addFilterContentRelation(filter, succesor);
}

//------------------------------------------------------------------------
void Analysis::addFilterContentRelation(FilterSPtr filter, ViewItemSPtr item)
{
  RelationName relation = QString("%1").arg(item->outputId());

  m_content->addRelation(filter, item, relation);
}

//------------------------------------------------------------------------
void Analysis::removeFilterContentRelation(FilterSPtr filter, ViewItem* item)
{
  ViewItemSPtr succesor;

  auto segmentation = dynamic_cast<Segmentation *>(item);
  if (segmentation)
  {
    succesor = find<Segmentation>(segmentation, m_segmentations);
  }
  else
  {
    auto channel = dynamic_cast<Channel *>(item);
    if (channel)
    {
      succesor = find<Channel>(channel, m_channels);
    }
  }

  removeFilterContentRelation(filter, succesor);
}

//------------------------------------------------------------------------
void Analysis::removeFilterContentRelation(FilterSPtr filter, ViewItemSPtr item)
{
  RelationName relation = QString("%1").arg(item->outputId());
  m_content->removeRelation(filter, item, relation);
}

//------------------------------------------------------------------------
bool Analysis::findRelation(PersistentSPtr    ancestor,
                            PersistentSPtr    succesor,
                            const RelationName& relation)
{
  const auto edges = m_relations->outEdges(ancestor, relation);

  auto booleanOp = [relation, succesor](const DirectedGraph::Edge &edge) {return (edge.relationship == relation.toStdString() && edge.target == succesor); };
  return std::any_of(edges.constBegin(), edges.constEnd(), booleanOp);
}

//------------------------------------------------------------------------
void Analysis::addConnection(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2, const NmVector3 &point)
{
  if(!m_connections.addConnection(segmentation1, segmentation2, point))
  {
    auto message = QObject::tr("Tried to add an existing connection between %1 and %2 at the point %3.").arg(segmentation1->name()).arg(segmentation2->name()).arg(point.toString());
    auto details = QObject::tr("Analysis::addConnection() -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }

  // the user is allowed to add multiple connections between the same segmentations, only add the relation once.
  if(m_connections.connections(segmentation1, segmentation2).size() < 2)
  {
    m_relations->addRelation(segmentation1, segmentation2, Connection::CONNECTS);
    m_relations->addRelation(segmentation2, segmentation1, Connection::CONNECTS);
  }
}

//------------------------------------------------------------------------
void Analysis::removeConnection(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2, const NmVector3 &point)
{
  if(!m_connections.removeConnection(segmentation1, segmentation2, point))
  {
    auto message = QObject::tr("Tried to remove a non existing connection between %1 and %2 at the point %3.").arg(segmentation1->name()).arg(segmentation2->name()).arg(point.toString());
    auto details = QObject::tr("Analysis::removeConnection() -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }

  // the user is allowed to add multiple connections between the same segmentations, only remove the relation when
  // there are no more connection points.
  if(m_connections.connections(segmentation1, segmentation2).isEmpty())
  {
    m_relations->removeRelation(segmentation1, segmentation2, Connection::CONNECTS);
    m_relations->removeRelation(segmentation2, segmentation1, Connection::CONNECTS);
  }
}

//------------------------------------------------------------------------
void Analysis::removeConnections(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2)
{
  if(!m_connections.removeConnections(segmentation1, segmentation2))
  {
    auto message = QObject::tr("Tried to remove non existing connections between %1 and %2.").arg(segmentation1->name()).arg(segmentation2->name());
    auto details = QObject::tr("Analysis::removeConnections(segmentation1, segmentation2) -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }
}

//------------------------------------------------------------------------
void Analysis::removeConnections(const PersistentSPtr segmentation)
{
  if(!m_connections.removeSegmentation(segmentation))
  {
    auto message = QObject::tr("Tried to remove non existing connections of segmentation %1.").arg(segmentation->name());
    auto details = QObject::tr("Analysis::removeConnections(segmentation) -> ") + message;

    throw Core::Utils::EspinaException(message, details);
  }
}

//------------------------------------------------------------------------
Core::Connections Analysis::connections(const PersistentSPtr segmentation1, const PersistentSPtr segmentation2) const
{
  Core::Connections result;

  for(auto connection: m_connections.connections(segmentation1, segmentation2))
  {
    Core::Connection coreConnection;
    coreConnection.segmentation1 = segmentation1;
    coreConnection.segmentation2 = segmentation2;
    coreConnection.point         = connection.point;

    result << coreConnection;
  }

  return result;
}

//------------------------------------------------------------------------
Core::Connections Analysis::connections(const PersistentSPtr segmentation) const
{
  Core::Connections result;

  for(auto connection: m_connections.connections(segmentation))
  {
    Core::Connection coreConnection;
    coreConnection.segmentation1 = segmentation;
    coreConnection.segmentation2 = m_itemUUids.key(connection.segmentation2);
    coreConnection.point         = connection.point;
    Q_ASSERT(coreConnection.segmentation2);

    result << coreConnection;
  }

  return result;
}

//------------------------------------------------------------------------
bool Analysis::saveConnections() const
{
  return m_connections.save();
}

//------------------------------------------------------------------------
bool Analysis::loadConnections()
{
  return m_connections.load();
}

//------------------------------------------------------------------------
Core::Connections ESPINA::Analysis::connections(const PersistentPtr segmentation) const
{
  auto segSPtr = m_itemPointers.key(segmentation);
  Q_ASSERT(segSPtr);
  return connections(segSPtr);
}

//------------------------------------------------------------------------
PersistentSPtr ESPINA::Analysis::smartPointer(PersistentPtr item)
{
  auto itemSPtr = m_itemPointers.key(item);
  Q_ASSERT(itemSPtr);

  return itemSPtr;
}
