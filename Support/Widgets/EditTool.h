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

#include <Support/EspinaSupport_Export.h>

// ESPINA
#include <Support/Widgets/ProgressTool.h>

class QAction;
class QUndoStack;

namespace ESPINA
{
  namespace Support
  {
    namespace Widgets
    {
      /** \class EditTool
       * \brief Base class for tools that edit segmentations.
       *
       */
      class EspinaSupport_EXPORT EditTool
      : public ProgressTool
      {
          Q_OBJECT
        public:
          /** \brief EditTool class constructor.
           * \param[in] id tool id.
           * \param[in] icon tool icon.
           * \param[in] tooltip tool tooltip text.
           * \param[in] context application context.
           *
           */
          explicit EditTool(const QString &id, const QString& icon, const QString& tooltip, Support::Context& context);

          /** \brief EditTools class destructor.
           *
           */
          virtual ~EditTool()
          {}

          virtual void onToolGroupActivated() override;

        protected:
          /** \brief Returns true if the list of given segmentations have all volumetric data and false otherwise.
           * \param[in] segmentations list of segmentations.
           *
           */
          const bool acceptsVolumetricSegmentations(const SegmentationAdapterList &segmentations) const;

          /** \brief Returns true if the list of given segmentation have all skeleton data and false otherwise.
           * \param[in] segmentations list of segmentations.
           *
           */
          const bool acceptsSkeletonSegmentations(const SegmentationAdapterList &segmentations) const;

          /** \brief Marks the given segmentation as being currently modified.
           * \param[in] segmentation segmentation to mark.
           * \param[in] value true to mark as being modified and false otherwise.
           *
           */
          void markAsBeingModified(SegmentationAdapterPtr segmentation, bool value);

        protected slots:
          /** \brief Enables/Disables the tool depending on the current segmentation selection.
           *
           */
          virtual void updateStatus() override;

        private:
          /** \brief Returns true if the tool accepts the given number of inputs and false otherwise.
           * \param[in] n numerical value > 0;
           */
          virtual bool acceptsNInputs(int n) const = 0;

          /** \brief Returns true if the tool can operate with the given segmentation list.
           * \param[in] segmentations list of segmentations.
           *
           */
          virtual bool acceptsSelection(SegmentationAdapterList segmentations);

          /** \brief Returns true if none of the segmentations in the given groups is being modified and false otherwise.
           * \param[in] segmentations list of segmentations.
           *
           */
          virtual bool selectionIsNotBeingModified(SegmentationAdapterList segmentations);
     };
    }
  }
} // namespace ESPINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_TOOL_H_
