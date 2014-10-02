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

// ESPINA
#include <GUI/Model/ModelAdapter.h>
#include <GUI/ModelFactory.h>
#include <Support/Widgets/ToolGroup.h>
#include <Support/ViewManager.h>
#include "SkeletonTool.h"

// Qt
#include <QUndoStack>

namespace ESPINA
{
  class SkeletonToolGroup
  : public ToolGroup
  {
    Q_OBJECT

    public:
      /** \brief SkeletonToolGroup class constructor.
       * \param[in] model model adapter smart pointer.
       * \param[in] factory model factory smart pointer.
       * \param[in] viewManager view manager smart pointer.
       * \parma[in] parent raw pointer of the QObject parent of this one.
       */
      SkeletonToolGroup(ModelAdapterSPtr model,
                        ModelFactorySPtr factory,
                        ViewManagerSPtr  viewManager,
                        QUndoStack      *undoStack,
                        QObject *parent = nullptr);

      /** \brief SkeletonToolGroup class virtual destructor.
       *
       */
      virtual ~SkeletonToolGroup();

      /** \brief Implements ToolGroup::setEnabled().
       *
       */
      virtual void setEnabled(bool value);

      /** \brief Implements ToolGroup::enabled().
       *
       */
      virtual bool enabled() const
      { return m_enabled; }

      /** \brief Implements ToolGroup::tools().
       *
       */
      virtual ToolSList tools();

    public slots:
      /** \brief Helper method to create a segmentation and assigns a skeleton output to it.
       *
       */
      void createSegmentation();

    private:
      ModelAdapterSPtr m_model;
      ModelFactorySPtr m_factory;
      QUndoStack      *m_undoStack;

      SkeletonToolSPtr m_tool;
      bool             m_enabled;
  };

  using SkeletonToolGroupPtr  = SkeletonToolGroup *;
  using SkeletonToolGroupSPtr = std::shared_ptr<SkeletonToolGroup>;

} // namespace EspINA

#endif // ESPINA_SKELETON_TOOLGROUP_H_
