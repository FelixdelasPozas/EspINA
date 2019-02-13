/*
 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>


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
#include <Core/Utils/EspinaException.h>
#include "LibraryExtensionFactory.h"
#include "EdgeDistances/ChannelEdges.h"
#include "EdgeDistances/ChannelEdgesFactory.h"
#include "EdgeDistances/EdgeDistance.h"
#include "EdgeDistances/EdgeDistanceFactory.h"
#include "Issues/SegmentationIssues.h"
#include "Issues/SegmentationIssuesFactory.h"
#include "Morphological/MorphologicalInformation.h"
#include "Morphological/MorphologicalInformationFactory.h"
#include "Notes/SegmentationNotes.h"
#include "Notes/SegmentationNotesFactory.h"
#include "Tags/SegmentationTags.h"
#include "Tags/SegmentationTagsFactory.h"
#include "Extensions/SkeletonInformation/DendriteInformation.h"
#include "Extensions/SkeletonInformation/AxonInformation.h"
#include "Extensions/SkeletonInformation/SynapseInformation.h"
#include "SkeletonInformation/SkeletonInformationFactory.h"
#include "SLIC/StackSLIC.h"
#include "SLIC/StackSLICFactory.h"
#include "BasicInformation/BasicSegmentationInformation.h"
#include "BasicInformation/BasicSegmentationInformationFactory.h"

// C++
#include <memory>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;

//-----------------------------------------------------------------------
LibraryStackExtensionFactory::LibraryStackExtensionFactory(CoreFactory* factory)
: StackExtensionFactory{factory}
{
  m_factories.insert(ChannelEdges::TYPE, std::make_shared<ChannelEdgesFactory>(factory));
  m_factories.insert(StackSLIC::TYPE, std::make_shared<StackSLICFactory>(factory));
}

//-----------------------------------------------------------------------
StackExtensionSPtr LibraryStackExtensionFactory::createExtension(const Core::StackExtension::Type &type,
                                                                 const Core::StackExtension::InfoCache &cache,
                                                                 const State& state) const
{
  StackExtensionSPtr extension = nullptr;

  if (m_factories.keys().contains(type))
  {
    try
    {
      extension = m_factories[type]->createExtension(type, cache, state);
    }
    catch(...)
    {
      // empty
    }
  }

  if(!extension || !extension.get())
  {
    auto message = QObject::tr("Unknown stack extension type %1").arg(type);
    auto details = QObject::tr("LibraryStackExtensionFactory::createExtension() -> ") + message;

    throw EspinaException(message, details);
  }

  return extension;
}

//-----------------------------------------------------------------------
StackExtension::TypeList ESPINA::LibraryStackExtensionFactory::providedExtensions() const
{
  StackExtension::TypeList list;
  list << m_factories.keys();

  return list;
}

//-----------------------------------------------------------------------
LibrarySegmentationExtensionFactory::LibrarySegmentationExtensionFactory(CoreFactory* factory)
: SegmentationExtensionFactory{factory}
{
  m_factories.insert(BasicSegmentationInformationExtension::TYPE, std::make_shared<BasicSegmentationInformationExtensionFactory>());
  m_factories.insert(EdgeDistance::TYPE,                          std::make_shared<EdgeDistanceFactory>(factory));
  m_factories.insert(SegmentationIssues::TYPE,                    std::make_shared<SegmentationIssuesFactory>());
  m_factories.insert(MorphologicalInformation::TYPE,              std::make_shared<MorphologicalInformationFactory>());
  m_factories.insert(SegmentationNotes::TYPE,                     std::make_shared<SegmentationNotesFactory>());
  m_factories.insert(SegmentationTags::TYPE,                      std::make_shared<SegmentationTagsFactory>());

  auto skeletonExtensionsFactory = std::make_shared<SkeletonInformationFactory>();
  m_factories.insert(DendriteSkeletonInformation::TYPE, skeletonExtensionsFactory);
  m_factories.insert(AxonSkeletonInformation::TYPE, skeletonExtensionsFactory);
  m_factories.insert(SynapseConnectionInformation::TYPE, skeletonExtensionsFactory);
}

//-----------------------------------------------------------------------
Core::SegmentationExtensionSPtr ESPINA::LibrarySegmentationExtensionFactory::createExtension(const Core::SegmentationExtension::Type &type,
                                                                                             const Core::SegmentationExtension::InfoCache &cache,
                                                                                             const State& state) const
{
  SegmentationExtensionSPtr extension = nullptr;

  if(m_factories.keys().contains(type))
  {
    try
    {
      extension = m_factories[type]->createExtension(type, cache, state);
    }
    catch(...)
    {
      // emtpy
    }
  }

  if(!extension || !extension.get())
  {
    auto message = QObject::tr("Unknown segmentation extension type %1").arg(type);
    auto details = QObject::tr("LibrarySegmentationExtensionFactory::createExtension() -> ") + message;

    throw EspinaException(message, details);
  }

  return extension;
}

//-----------------------------------------------------------------------
SegmentationExtension::TypeList ESPINA::LibrarySegmentationExtensionFactory::providedExtensions() const
{
  SegmentationExtension::TypeList list;
  list << m_factories.keys();

  return list;
}
