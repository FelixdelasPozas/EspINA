/*

 Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_SKELETON_TOOLGROUP_H_
#define ESPINA_SKELETON_TOOLGROUP_H_

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include "SkeletonTool.h"
#include <ToolGroups/ToolGroup.h>

// Qt
#include <QUndoStack>

namespace ESPINA
{
  class EspinaGUI_EXPORT SkeletonToolGroup
  : public ToolGroup
  {
    Q_OBJECT

    public:
      /** \brief SkeletonToolGroup class constructor.
       * \param[in] context ESPINA context
       *
       */
      SkeletonToolGroup(Support::Context &context);

      /** \brief SkeletonToolGroup class virtual destructor.
       *
       */
      virtual ~SkeletonToolGroup()
      {}

      virtual void setEnabled(bool value);

      virtual bool enabled() const
      { return m_enabled; }

      virtual Support::Widgets::ToolSList tools();

    private:
      SkeletonToolSPtr m_tool;
      bool             m_enabled;
  };

  using SkeletonToolGroupPtr  = SkeletonToolGroup *;
  using SkeletonToolGroupSPtr = std::shared_ptr<SkeletonToolGroup>;

} // namespace EspINA

#endif // ESPINA_SKELETON_TOOLGROUP_H_
