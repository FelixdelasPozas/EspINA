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

#include <GUI/Utils/Timer.h>
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
        explicit RepresentationInvalidator(Timer &timer);

        /** \brief Invalidates item representations
         *
         */
        void invalidateRepresentations(ViewItemAdapterSList items);

        /** \brief Invalidates item representations and those which
         *         depend on them
         *
         */
        void invalidateDependentRepresentations(ViewItemAdapterSList items);

        const Timer &timer() const;

      signals:
        void representationsInvalidated(ViewItemAdapterSList items, TimeStamp t);

      private:
        Timer &m_timer;
      };
    }
  }
}

#endif // ESPINA_GUI_REPRESENTATION_INVALIDATOR_H
