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
#include "Core/Analysis/Extension.h"

// BOOST
#include <boost/graph/graph_concepts.hpp>

namespace ESPINA {

  class EspinaCore_EXPORT ReadOnlySegmentationExtension
  : public SegmentationExtension
  {
  public:
  	/** \brief ReadOnlySegmentationExtension class constructor.
  	 * \param[in] type, segmentation extension type.
  	 * \param[in] cache, cache object.
  	 * \param[in] state, state of the extension.
  	 *
  	 */
    explicit ReadOnlySegmentationExtension(const SegmentationExtension::Type      &type,
                                           const SegmentationExtension::InfoCache &cache,
                                           const State &state);

  	/** \brief Implements Extension::type().
  	 *
  	 */
    virtual SegmentationExtension::Type type() const
    { return m_type; }

  	/** \brief Sets if the extension data is invalidated when the extended item changes.
  	 * \param[in] value, true to invalidate on change, false otherwise.
  	 *
  	 */
    void setInvalidateOnChange(bool value)
    { m_invalidateOnChange = value; }

  	/** \brief Implements Extension::invalidateOnChange().
  	 *
  	 */
    virtual bool invalidateOnChange() const
    { return m_invalidateOnChange; }

  	/** \brief Implements Extension::state().
  	 *
  	 */
    virtual State state() const
    { return m_state; }

  	/** \brief Implements Extensions::snapshot().
  	 *
  	 */
    virtual Snapshot snapshot() const
    { return Snapshot(); } // TODO

  	/** \brief Implements Extension::dependencies().
  	 *
  	 */
    virtual TypeList dependencies() const
    { return TypeList(); }

  	/** \brief Implements Extension::availabelInformations().
  	 *
  	 */
    virtual InfoTagList availableInformations() const
    { return readyInformation(); }

    /** \brief Implements Extension::validCategory().
  	 *
  	 */
    virtual bool validCategory(const QString& classificationName) const
    { return true; }

  protected:
  	/** \brief Implements Extension::onExtendedItemSet().
  	 *
  	 */
    virtual void onExtendedItemSet(Segmentation* item);

  	/** \brief Implements Extension::cacheFail().
  	 *
  	 */
    virtual QVariant cacheFail(const InfoTag &tag) const
    { return QVariant(); } //TODO

  private:
    SegmentationExtension::Type m_type;
    const State m_state;
    bool m_invalidateOnChange;
  };

} // namespace ESPINA

#endif // ESPINA_READ_ONLY_SEGMENTATION_EXTENSION_H
