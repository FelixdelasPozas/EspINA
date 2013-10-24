/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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

#include "Analysis.h"

#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Segmentation.h"
#include "Core/Analysis/Extensions/ExtensionProvider.h"

using namespace EspINA;

//------------------------------------------------------------------------
Analysis::Analysis()
: m_classification{nullptr}
, m_relations{new DirectedGraph()}
, m_content{new DirectedGraph()}
{

}

//------------------------------------------------------------------------
void Analysis::reset()
{
  m_classification.reset();
  m_content   = DirectedGraphSPtr{new DirectedGraph()};
  m_relations = DirectedGraphSPtr{new DirectedGraph()};
  
  m_samples.clear();
  m_channels.clear();
  m_segmentations.clear();
  m_filters.clear();
  m_providers.clear();
}

//------------------------------------------------------------------------
void Analysis::setClassification(ClassificationSPtr classification)
{
  m_classification = classification;
}

//------------------------------------------------------------------------
void Analysis::add(SampleSPtr sample)
{
  if (m_samples.contains(sample)) throw (Existing_Item_Exception());

  m_samples << sample;
  m_content->addItem(sample);
  m_relations->addItem(sample);
}

//------------------------------------------------------------------------
void Analysis::add(SampleSList samples)
{
  foreach(SampleSPtr sample, samples)
  {
    add(sample);
  }
}

//------------------------------------------------------------------------
void Analysis::add(ChannelSPtr channel)
{
  if (m_channels.contains(channel)) throw (Existing_Item_Exception());

  m_channels << channel;

  FilterSPtr filter = channel->filter(); // TODO: What happens when a channel change its output?!

  addIfNotExists(filter);

  m_content->addItem(channel);

  RelationName relation = QString("%1").arg(channel->output()->id());

  m_content->addRelation(filter, channel, relation);
  
  m_relations->addItem(channel);
}

//------------------------------------------------------------------------
void Analysis::add(ChannelSList channels)
{
  foreach(ChannelSPtr channel, channels)
  {
    add(channel);
  }
}

//------------------------------------------------------------------------
void Analysis::add(SegmentationSPtr segmentation)
{
  if (m_segmentations.contains(segmentation)) throw (Existing_Item_Exception());

  m_segmentations << segmentation;

  FilterSPtr filter = segmentation->filter(); // TODO: What happens when a segmentation change its output?!

  addIfNotExists(filter);

  m_content->addItem(segmentation);

  RelationName relation = QString("%1").arg(segmentation->output()->id());

  m_content->addRelation(filter, segmentation, relation);
  
  m_relations->addItem(segmentation);
}

//------------------------------------------------------------------------
void Analysis::add(SegmentationSList segmentations)
{
  foreach(SegmentationSPtr segmentation, segmentations)
  {
    add(segmentation);
  }
}

//------------------------------------------------------------------------
void Analysis::add(ExtensionProviderSPtr provider)
{
  if (m_providers.contains(provider)) throw (Existing_Item_Exception());
  
  m_providers << provider;
  m_content->addItem(provider);
}

//------------------------------------------------------------------------
void Analysis::remove(SampleSPtr sample)
{
  if (!m_samples.contains(sample)) throw (Item_Not_Found_Exception());

  m_samples.removeOne(sample);

  m_content->removeItem(sample);
  m_relations->removeItem(sample);
}

//------------------------------------------------------------------------
void Analysis::remove(SampleSList samples)
{
  foreach(SampleSPtr sample, samples)
  {
    remove(sample);
  }
}


//------------------------------------------------------------------------
void Analysis::remove(ChannelSPtr channel)
{
  if (!m_channels.contains(channel)) throw (Item_Not_Found_Exception());

  m_channels.removeOne(channel);

  m_content->removeItem(channel);
  m_relations->removeItem(channel);

  removeIfIsolated(channel->filter());
}

//------------------------------------------------------------------------
void Analysis::remove(ChannelSList channels)
{
  foreach(ChannelSPtr channel, channels)
  {
    remove(channel);
  }
}

//------------------------------------------------------------------------
void Analysis::remove(SegmentationSPtr segmentation)
{
  if (!m_segmentations.contains(segmentation)) throw (Item_Not_Found_Exception());

  m_segmentations.removeOne(segmentation);

  m_content->removeItem(segmentation);
  m_relations->removeItem(segmentation);

  removeIfIsolated(segmentation->filter());
}

//------------------------------------------------------------------------
void Analysis::remove(SegmentationSList segmentations)
{
  foreach(SegmentationSPtr segmentation, segmentations)
  {
    remove(segmentation);
  }
}

//------------------------------------------------------------------------
void Analysis::remove(ExtensionProviderSPtr provider)
{
  if (!m_providers.contains(provider)) throw (Item_Not_Found_Exception());

  m_providers.removeOne(provider);

  m_content->removeItem(provider);
}

//------------------------------------------------------------------------
void Analysis::addRelation(PersistentSPtr    ancestor,
                           PersistentSPtr    succesor,
                           const RelationName& relation)
{
  if (!m_relations->contains(ancestor)) throw (Item_Not_Found_Exception());

  if (!m_relations->contains(succesor)) throw (Item_Not_Found_Exception());

  if (findRelation(ancestor, succesor, relation))  throw (Existing_Relation_Exception());

  m_relations->addRelation(ancestor, succesor, relation);
}

//------------------------------------------------------------------------
void Analysis::deleteRelation(PersistentSPtr    ancestor,
                              PersistentSPtr    succesor,
                              const RelationName& relation)
{
  if (!findRelation(ancestor, succesor, relation)) throw (Relation_Not_Found_Exception());

  m_relations->removeRelation(ancestor, succesor, relation);
}

//------------------------------------------------------------------------
bool Analysis::removeIfIsolated(DirectedGraphSPtr graph, PersistentSPtr item)
{
  bool removed = false;

  DirectedGraph::Vertex v = graph->vertex(item);

  if (graph->contains(item) && graph->edges(v).isEmpty())
  {
    graph->removeItem(item);
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
    m_filters << filter;
    m_content->addItem(filter);
  }
}


//------------------------------------------------------------------------
void Analysis::removeIfIsolated(FilterSPtr filter)
{
  if (removeIfIsolated(m_content, filter))
  {
    m_filters.removeOne(filter);
  }
}

//------------------------------------------------------------------------
bool Analysis::findRelation(PersistentSPtr    ancestor,
                            PersistentSPtr    succesor,
                            const RelationName& relation)
{
  DirectedGraph::Vertex v = m_relations->vertex(ancestor);
  foreach(DirectedGraph::Edge edge, m_relations->outEdges(v, relation))
  {
   if (edge.relationship == relation.toStdString() && edge.target.item == succesor) return true;
  }

  return false;
}