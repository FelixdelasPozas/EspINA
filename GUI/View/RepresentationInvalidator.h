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

#ifndef ESPINA_GUI_REPRESENTATION_INVALIDATOR_H
#define ESPINA_GUI_REPRESENTATION_INVALIDATOR_H

#include <GUI/Types.h>
#include <GUI/Model/ViewItemAdapter.h>

namespace ESPINA
{
  namespace GUI
  {
    namespace View
    {
      class RepresentationInvalidator
      : public QObject
      {
        Q_OBJECT

      public:
        enum class Scope
        {
          SELECTED_ITEMS,
          DEPENDENT_ITEMS
        };

      public:
        explicit RepresentationInvalidator(ViewState &state);

        /** \brief Invalidates item representations
         * \param[in] items to invalide their representations
         * \param[in] scope of the invalidation.
         *
         */
        void invalidateRepresentations(const ViewItemAdapterList &items,
                                       const Scope scope = Scope::SELECTED_ITEMS);

        /** \brief Update item representation colors
         * \param[in] items to invalide their representation colors
         * \param[in] scope of the invalidation.
         *
         */
        void invalidateRepresentationColors(const ViewItemAdapterList &items,
                                            const Scope scope = Scope::SELECTED_ITEMS);

        GUI::Representations::FrameCSPtr createFrame() const;

      public slots:
        /** \brief Invalidates item representations
         *
         */
        void invalidateRepresentations(ViewItemAdapterPtr item);

      signals:
        void representationsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);
        void representationColorsInvalidated(ViewItemAdapterList items, const GUI::Representations::FrameCSPtr frame);

      private:
        ViewItemAdapterList scopedItems(const ViewItemAdapterList &items,
                                        const Scope scope = Scope::SELECTED_ITEMS);

      private:
        ViewState &m_state;
      };
    }
  }
}

#endif // ESPINA_GUI_REPRESENTATION_INVALIDATOR_H
