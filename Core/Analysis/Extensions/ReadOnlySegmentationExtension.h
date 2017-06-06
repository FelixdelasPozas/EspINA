/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_READ_ONLY_SEGMENTATION_EXTENSION_H
#define ESPINA_READ_ONLY_SEGMENTATION_EXTENSION_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Extensions.h>

namespace ESPINA
{
  namespace Core
  {
    /** \class ReadOnlySegmentationExtension
     * \brief Extension to use when the SEG reader founds an unknown segmentation extension.
     * All the info is saved on disk except if invalidate is true and the segmentation changed.
     *
     */
    class EspinaCore_EXPORT ReadOnlySegmentationExtension
    : public SegmentationExtension
    {
      public:
        /** \brief ReadOnlySegmentationExtension class constructor.
         * \param[in] type segmentation extension type.
         * \param[in] cache cache object.
         * \param[in] state state of the extension.
         *
         */
        explicit ReadOnlySegmentationExtension(const SegmentationExtension::Type      &type,
                                               const SegmentationExtension::InfoCache &cache,
                                               const State                            &state)
        : SegmentationExtension{cache}
        , m_type               {type}
        , m_state              {state}
        , m_invalidateOnChange {false}
        {}

        virtual SegmentationExtension::Type type() const
        { return m_type; }

        /** \brief Sets if the extension data is invalidated when the extended item changes.
         * \param[in] value true to invalidate on change, false otherwise.
         *
         */
        void setInvalidateOnChange(bool value)
        { m_invalidateOnChange = value; }

        virtual bool invalidateOnChange() const
        { return m_invalidateOnChange; }

        virtual State state() const
        { return m_state; }

        virtual Snapshot snapshot() const
        { return Snapshot(); }

        virtual TypeList dependencies() const
        { return TypeList(); }

        virtual InformationKeyList availableInformation() const
        { return readyInformation(); }

        virtual bool validCategory(const QString& classificationName) const
        { return true; }

        virtual bool validData(const OutputSPtr output) const
        { return true; }

        virtual QString toolTipText() const
        { return tr("%1 as Read-Only").arg(m_type); }

      protected:
        virtual void onExtendedItemSet(Segmentation* item)
        {};

        virtual QVariant cacheFail(const InformationKey &tag) const
        { return QVariant(); }

      private:
        SegmentationExtension::Type m_type;               /** type of the unknown extension.                                                     */
        const State                 m_state;              /** state of the unknown extension.                                                    */
        bool                        m_invalidateOnChange; /** true to invalidate the information when the segmentation changes, false otherwise. */
    };

  } // namespace Core
} // namespace ESPINA

#endif // ESPINA_READ_ONLY_SEGMENTATION_EXTENSION_H
