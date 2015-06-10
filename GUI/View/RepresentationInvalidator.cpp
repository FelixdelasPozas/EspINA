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

#include "RepresentationInvalidator.h"

using namespace ESPINA;
using namespace ESPINA::GUI::View;

//------------------------------------------------------------------------
RepresentationInvalidator::RepresentationInvalidator(Timer &timer)
: m_timer(timer)
{
}

//------------------------------------------------------------------------
void RepresentationInvalidator::invalidateRepresentations(ViewItemAdapterList items)
{
  auto t = m_timer.increment();

  emit representationsInvalidated(items, t);
}

//------------------------------------------------------------------------
void RepresentationInvalidator::invalidateDependentRepresentations(ViewItemAdapterList items)
{
  auto invalidatedItems = items;
  // TODO 2015-04-20: search dependent items on relationship graph

  invalidateRepresentations(invalidatedItems);
}

//------------------------------------------------------------------------
Timer &RepresentationInvalidator::timer() const
{
  return m_timer;
}

//------------------------------------------------------------------------
void RepresentationInvalidator::invalidateRepresentations(ViewItemAdapterPtr item)
{
  invalidateRepresentations(toViewItemList(item));
}