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

#include "PipelineSources.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
PipelineSources::PipelineSources(): QObject()
{

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