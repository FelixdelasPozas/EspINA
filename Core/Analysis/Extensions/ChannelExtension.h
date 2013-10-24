/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This program is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_CHANNEL_EXTENSION_H
#define ESPINA_CHANNEL_EXTENSION_H

#include "EspinaCore_Export.h"

#include "Core/EspinaTypes.h"

namespace EspINA
{
  class EspinaCore_EXPORT ChannelExtension
  {
  public:
    using Type = QString;
    
    struct Existing_Extension{};
    struct Extension_Not_Found{};

  public:
    virtual ~ChannelExtension(){}

    void setChannel(ChannelPtr channel)
    { m_channel = channel; onChannelSet(channel); }

    ChannelPtr channel() const {return m_channel;}

    virtual void onChannelSet(ChannelPtr channel) = 0;

    virtual void initialize() = 0;

    virtual void invalidate() = 0;

    virtual Type type() const = 0;

  protected:
    explicit ChannelExtension() 
    : m_channel{nullptr} {}

    ChannelPtr m_channel;
  };

  using ChannelExtensionPtr   = ChannelExtension *;
  using ChannelExtensionList  = QList<ChannelExtensionPtr>;
  using ChannelExtensionSPtr  = std::shared_ptr<ChannelExtension>;
  using ChannelExtensionSList = QList<ChannelExtensionSPtr>;
} // namespace EspINA

#endif // ESPINA_CHANNEL_EXTENSION_H
