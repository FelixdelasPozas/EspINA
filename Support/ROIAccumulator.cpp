/*
 * Copyright 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "ROIAccumulator.h"

using namespace ESPINA;
using namespace ESPINA::Support;

//-----------------------------------------------------------------------------
ROIAccumulator::ROIAccumulator()
: m_provider(nullptr)
{

}

//-----------------------------------------------------------------------------
void ROIAccumulator::setProvider(ROIProviderPtr provider)
{
  m_provider = provider;
}

//-----------------------------------------------------------------------------
ROISPtr ROIAccumulator::currentROI()
{
  return m_provider?m_provider->currentROI():ROISPtr();
}

//-----------------------------------------------------------------------------
void ROIAccumulator::clear()
{
  if (m_provider)
  {
    m_provider->consumeROI();
  }
}
