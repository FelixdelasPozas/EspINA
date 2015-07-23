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

#ifndef ESPINA_READ_ONLY_CHANNEL_EXTENSION_H
#define ESPINA_READ_ONLY_CHANNEL_EXTENSION_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/Analysis/Extension.h"

namespace ESPINA {

  // TODO 2015-04-20: Implement this class
  class EspinaCore_EXPORT ReadOnlyChannelExtension
  : public ChannelExtension
  {
  public:
    /** \brief ReadOnlyChannelExtension class constructor.
     * \param[in] type channel extension type.
     * \param[in] cache cache object.
     * \param[in] state state of the extension.
     */
    explicit ReadOnlyChannelExtension(const ChannelExtension::Type       type,
                                      const ChannelExtension::InfoCache &cache,
                                      const State &state);

    /** \brief Implements Extension::type().
     *
     */
    virtual ChannelExtension::Type type() const
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
    { return InformationKeyList(); }

  protected:
    virtual void onExtendedItemSet(Channel* item);

    virtual QVariant cacheFail(const InformationKey &tag) const
    { return QVariant(); }

  private:
    ChannelExtension::Type m_type;
    State m_state;
    bool  m_invalidateOnChange;
  };

} // namespace ESPINA

#endif // ESPINA_READ_ONLY_CHANNEL_EXTENSION_H
