/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Core/Analysis/Segmentation.h>
#include <Core/Analysis/Category.h>
#include <Core/Analysis/Analysis.h>
#include <Extensions/BasicInformation/BasicSegmentationInformation.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;

const SegmentationExtension::Type BasicSegmentationInformationExtension::TYPE = "Segmentation";

const SegmentationExtension::Key CATEGORY        = "Category";
const SegmentationExtension::Key NUM_CONNECTIONS = "Num of connections";

//--------------------------------------------------------------------
BasicSegmentationInformationExtension::BasicSegmentationInformationExtension(const InfoCache &infoCache)
: Core::SegmentationExtension{infoCache}
{
}

//--------------------------------------------------------------------
const SegmentationExtension::InformationKeyList BasicSegmentationInformationExtension::availableInformation() const
{
  InformationKeyList keys;
  
  keys << createKey(CATEGORY)
       << createKey(NUM_CONNECTIONS);

  return keys;
}

//--------------------------------------------------------------------
QVariant BasicSegmentationInformationExtension::cacheFail(const InformationKey& key) const
{
  QVariant result;

  result = m_extendedItem->category()->name();
  updateInfoCache(CATEGORY, result);

  auto analysis = m_extendedItem->analysis();
  result = analysis->connections(analysis->smartPointer(m_extendedItem)).size();
  updateInfoCache(NUM_CONNECTIONS, result);

  result = information(key);

  return result;
}
