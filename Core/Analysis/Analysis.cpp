/*

    Copyright (C) 2014  Jorge Peña Pastor<jpena@cesvima.upm.es>

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
#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Segmentation.h"
#include <Core/Utils/AnalysisUtils.h>

using namespace ESPINA;

//------------------------------------------------------------------------
Analysis::Analysis()
: m_classification{nullptr}
, m_relations{new DirectedGraph()}
, m_content{new DirectedGraph()}
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
}

//------------------------------------------------------------------------
void Analysis::setStorage(TemporalStorageSPtr storage)
{
  m_storage = storage;

  for (auto persistent : m_content->vertices())
  {
    persistent->setStorage(storage);
  }
}

//------------------------------------------------------------------------
void Analysis::setClassification(ClassificationSPtr classification)
{
  m_classification = classification;
}

//------------------------------------------------------------------------
void Analysis::add(SampleSPtr sample) throw (Existing_Item_Exception)
{
  if (m_samples.contains(sample)) throw (Existing_Item_Exception());

  m_samples << sample;

  m_content->add(sample);
  m_relations->add(sample);

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
void Analysis::add(ChannelSPtr channel) throw (Existing_Item_Exception)
{
  if (m_channels.contains(channel)) throw (Existing_Item_Exception());

  m_channels << channel;

  FilterSPtr filter = channel->filter();

  addIfNotExists(filter);

  m_content->add(channel);

  RelationName relation = QString("%1").arg(channel->outputId());

  m_content->addRelation(filter, channel, relation);

  m_relations->add(channel);

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
void Analysis::add(SegmentationSPtr segmentation) throw (Existing_Item_Exception)
{
  if (m_segmentations.contains(segmentation))
    throw (Existing_Item_Exception());

  m_segmentations << segmentation;

  FilterSPtr filter = segmentation->filter();

  addIfNotExists(filter);

  m_content->add(segmentation);

  addFilterContentRelation(filter, segmentation);

  m_relations->add(segmentation);

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
void Analysis::remove(SampleSPtr sample) throw (Item_Not_Found_Exception)
{
  if (!m_samples.contains(sample)) throw (Item_Not_Found_Exception());

  sample->setAnalysis(nullptr);
  m_samples.removeOne(sample);

  m_content->remove(sample);
  m_relations->remove(sample);
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
void Analysis::remove(ChannelSPtr channel) throw (Item_Not_Found_Exception)
{
  if (!m_channels.contains(channel))
  	throw (Item_Not_Found_Exception());

  channel->setAnalysis(nullptr);
  m_channels.removeOne(channel);

  m_content->remove(channel);
  m_relations->remove(channel);

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
void Analysis::remove(SegmentationSPtr segmentation) throw (Item_Not_Found_Exception)
{
  if (!m_segmentations.contains(segmentation))
    throw (Item_Not_Found_Exception());

  segmentation->setAnalysis(nullptr);
  m_segmentations.removeOne(segmentation);

  m_content->remove(segmentation);
  m_relations->remove(segmentation);

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
void Analysis::addRelation(PersistentSPtr    ancestor,
                           PersistentSPtr    succesor,
                           const RelationName& relation)  throw (Item_Not_Found_Exception,Existing_Relation_Exception)
{
  if (!m_relations->contains(ancestor))
    throw (Item_Not_Found_Exception());

  if (!m_relations->contains(succesor))
    throw (Item_Not_Found_Exception());

  if (findRelation(ancestor, succesor, relation))
    throw (Existing_Relation_Exception());

  m_relations->addRelation(ancestor, succesor, relation);
}

//------------------------------------------------------------------------
void Analysis::deleteRelation(PersistentSPtr    ancestor,
                              PersistentSPtr    succesor,
                              const RelationName& relation) throw (Relation_Not_Found_Exception)
{
  if (!findRelation(ancestor, succesor, relation))
    throw (Relation_Not_Found_Exception());

  m_relations->removeRelation(ancestor, succesor, relation);
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

      auto ancestor    = find<Filter>(inputFilter.get(), m_filters);
      Q_ASSERT(ancestor);

      m_content->addRelation(ancestor, filter, QString("%1-%2").arg(i).arg(input->output()->id()));
    }
  }
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
  } else
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
  for(auto edge : m_relations->outEdges(ancestor, relation))
  {
    if (edge.relationship == relation.toStdString() && edge.target == succesor) return true;
  }

  return false;
}
