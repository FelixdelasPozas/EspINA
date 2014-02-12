/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include "Core/Analysis/Extension.h"

namespace EspINA {

  class ReadOnlyChannelExtension
  : public ChannelExtension
  {
  public:
    explicit ReadOnlyChannelExtension(ChannelExtension::Type type,
                                  const State               &state=State());

    virtual ChannelExtension::Type type() const
    { return m_type; }

    void setInvalidateOnChange(bool value)
    { m_invalidateOnChange = value; }

    virtual bool invalidateOnChange() const
    { return m_invalidateOnChange; }

    virtual State state() const
    { return m_state; }

    virtual Snapshot snapshot() const
    { return Snapshot(); } // TODO

    virtual TypeList dependencies() const
    { return TypeList(); }

    virtual InfoTagList availableInformations() const
    { return InfoTagList(); } // TODO

  protected:
    virtual void onExtendedItemSet(Channel* item);

    virtual QVariant cacheFail(const InfoTag &tag) const
    { return QVariant(); } //TODO

  private:
    ChannelExtension::Type m_type;
    bool  m_invalidateOnChange;
    State m_state;
  };

} // namespace EspINA

#endif // ESPINA_READ_ONLY_CHANNEL_EXTENSION_H