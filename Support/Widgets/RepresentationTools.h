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

#ifndef ESPINA_SUPPORT_WIDGETS_VISUALZE_TOOLS_H
#define ESPINA_SUPPORT_WIDGETS_VISUALZE_TOOLS_H

#include <Support/Widgets/RepresentationsGroupTool.h>
#include <Support/Representations/RepresentationFactory.h>

namespace ESPINA
{
  namespace Support
  {
    namespace Widgets
    {
      class RepresentationTools
      : public QObject
      {
        Q_OBJECT

      public:
        explicit RepresentationTools(Timer &timer);

        /** \brief Add render switch to group render group tool
         *
         */
        void addRepresentationSwitch(RepresentationGroup      group,
                                     RepresentationSwitchSPtr repSwitch,
                                     const QIcon             &groupIcon        = QIcon(),
                                     const QString           &groupDescription = QString());

        RepresentationGroupToolSPtr channelRepresentations() const;

        RepresentationGroupToolSPtr segmentationRepresentations() const;

        ToolSList representationTools() const;

      signals:
        emit void representationToolAdded(ToolSPtr tool);

      private:
        using RepresentationGroupTools = QMap<RepresentationGroup, RepresentationGroupToolSPtr>;

        RepresentationGroupToolSPtr m_channelsGroup;
        RepresentationGroupToolSPtr m_segmentationsGroup;
        RepresentationGroupTools    m_dynamicRepresentationGroups;

        Timer &m_timer;
      };
    }
  }
}

#endif // ESPINA_SUPPORT_WIDGETS_VISUALZE_TOOLS_H
