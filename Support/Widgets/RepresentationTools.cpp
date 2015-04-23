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

#include "RepresentationTools.h"

#include <Support/Representations/RepresentationUtils.h>

using namespace ESPINA;
using namespace ESPINA::Support::Representations::Utils;
using namespace ESPINA::Support::Widgets;

//----------------------------------------------------------------------------
RepresentationTools::RepresentationTools(Timer &timer)
: m_channelsGroup     (new RepresentationsGroupTool(":/espina/channels_switch.png",      tr("Show Channels"), timer))
, m_segmentationsGroup(new RepresentationsGroupTool(":/espina/segmentations_switch.svg", tr("Show Segmentations"), timer))
, m_timer(timer)
{
  m_channelsGroup->showActiveRepresentations();
  m_segmentationsGroup->showActiveRepresentations();
}

//----------------------------------------------------------------------------
void RepresentationTools::addRepresentationSwitch(RepresentationGroup      group,
                                                  RepresentationSwitchSPtr repSwitch,
                                                  const QIcon             &groupIcon,
                                                  const QString           &groupDescription)
{
  if (CHANNELS_GROUP == group)
  {
    m_channelsGroup->addRepresentationSwitch(repSwitch);
  }
  else if (SEGMENTATIONS_GROUP == group)
  {
    m_segmentationsGroup->addRepresentationSwitch(repSwitch);
  }
  else
  {
    auto addRepresentationGroup = !m_dynamicRepresentationGroups.contains(group);

    auto newGroup               = std::make_shared<RepresentationsGroupTool>(groupIcon, groupDescription, m_timer);
    auto representationGroup    = m_dynamicRepresentationGroups.value(group, newGroup);

    representationGroup->addRepresentationSwitch(repSwitch);

    m_dynamicRepresentationGroups[group] = representationGroup;

    if (addRepresentationGroup)
    {
      emit representationToolAdded(representationGroup);
    }
  }

}

//----------------------------------------------------------------------------
RepresentationGroupToolSPtr RepresentationTools::channelRepresentations() const
{
  return m_channelsGroup;
}

//----------------------------------------------------------------------------
RepresentationGroupToolSPtr RepresentationTools::segmentationRepresentations() const
{
  return m_segmentationsGroup;
}

//----------------------------------------------------------------------------
ToolSList RepresentationTools::representationTools() const
{
  ToolSList tools;

  tools << m_channelsGroup << m_segmentationsGroup;

  for (auto dynamicTool : m_dynamicRepresentationGroups)
  {
    tools << dynamicTool;
  }

  return tools;
}
