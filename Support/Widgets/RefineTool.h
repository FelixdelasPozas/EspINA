/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_
#define ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_

// ESPINA
#include <Support/Widgets/Tool.h>
#include <Support/Factory/FilterDelegateFactory.h>

class QAction;
class QUndoStack;

namespace ESPINA
{
  namespace Support
  {
    namespace Widgets
    {
      class RefineTool
      : public ProgressTool
      {
        Q_OBJECT
      public:
        explicit RefineTool(const QString& icon, const QString& tooltip, Support::Context& context);

        /** \brief RefineTools class destructor.
         *
         */
        virtual ~RefineTool();

        virtual void onToolGroupActivated() override;

      protected:
        bool acceptsVolumetricSegmenations(SegmentationAdapterList segmentations);

      private slots:
        /** \brief
         *
         */
        void updateStatus();

      private:
        virtual bool acceptsNInputs(int n) const = 0;

        virtual bool acceptsSelection(SegmentationAdapterList segmentations);

        bool selectionIsNotBeingModified(SegmentationAdapterList segmentations);
     };

      //   using RefineToolPtr  = RefineTool *;
      //   using RefineToolSPtr = std::shared_ptr<RefineTool>;
    }
  }
} // namespace ESPINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_