/*
 * Copyright 2015 <copyright holder> <email>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

// ESPINA
#include "PipelineSources.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
PipelineSources::PipelineSources(GUI::View::RepresentationInvalidator &invalidator)
: m_representationInvalidator(invalidator)
{
  connect(&m_representationInvalidator, SIGNAL(representationsInvalidated(ViewItemAdapterList,TimeStamp)),
          this,                         SLOT(onRepresentationsInvalidated(ViewItemAdapterList, TimeStamp)));

  connect(&m_representationInvalidator, SIGNAL(representationColorsInvalidated(ViewItemAdapterList,TimeStamp)),
          this,                         SLOT(onRepresentationColorsInvalidated(ViewItemAdapterList,TimeStamp)));
}

//-----------------------------------------------------------------------------
PipelineSources::~PipelineSources()
{
}

//-----------------------------------------------------------------------------
ViewItemAdapterList PipelineSources::sources() const
{
  return m_sources;
}

//-----------------------------------------------------------------------------
GUI::View::RepresentationInvalidator &PipelineSources::invalidator()
{
  return m_representationInvalidator;
}

//-----------------------------------------------------------------------------
void PipelineSources::insert(ViewItemAdapterList sources)
{
  for (auto source : sources)
  {
    Q_ASSERT(!contains(source));
    m_sources.append(source);
  }
}

//-----------------------------------------------------------------------------
bool PipelineSources::contains(ViewItemAdapterPtr source) const
{
  return m_sources.contains(source);
}

//-----------------------------------------------------------------------------
void PipelineSources::remove(ViewItemAdapterList sources)
{
  for (auto source : sources)
  {
    Q_ASSERT(contains(source));
    m_sources.removeOne(source);
  }
}

//-----------------------------------------------------------------------------
TimeStamp PipelineSources::timeStamp() const
{
  return m_representationInvalidator.timer().timeStamp();
}

//-----------------------------------------------------------------------------
void PipelineSources::onRepresentationsInvalidated(ViewItemAdapterList items, TimeStamp t)
{
  auto invalidatedItems = acceptedItems(items);

  if (!invalidatedItems.isEmpty())
  {
    emit representationsInvalidated(invalidatedItems, t);
  }
}

//-----------------------------------------------------------------------------
void PipelineSources::onRepresentationColorsInvalidated(ViewItemAdapterList items, TimeStamp t)
{
  auto invalidatedItems = acceptedItems(items);

  if (!invalidatedItems.isEmpty())
  {
    emit representationColorsInvalidated(invalidatedItems, t);
  }
}

//-----------------------------------------------------------------------------
ViewItemAdapterList PipelineSources::acceptedItems(const ViewItemAdapterList& items)
{
  ViewItemAdapterList acceptedItems;

  for (auto item : items)
  {
    if (contains(item))
    {
      acceptedItems << item;
    }
  }

  return acceptedItems;
}
