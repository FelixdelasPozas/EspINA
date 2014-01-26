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
#include <Core/Analysis/Persistent.h>
#include <QMap>

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

    virtual Type type() const = 0;

    virtual bool invalidateOnChange() const = 0;

    virtual State state() const = 0;

    virtual Snapshot snapshot() const = 0;

    void setChannel(ChannelPtr channel)
    { m_channel = channel; onChannelSet(channel); }

    ChannelPtr channel() const {return m_channel;}
  protected:
    explicit ChannelExtension() 
    : m_channel{nullptr} {}

    virtual void onChannelSet(ChannelPtr channel) = 0;

  protected:
    ChannelPtr m_channel;
  };

  using ChannelExtensionPtr      = ChannelExtension *;
  using ChannelExtensionList     = QList<ChannelExtensionPtr>;
  using ChannelExtensionSPtr     = std::shared_ptr<ChannelExtension>;
  using ChannelExtensionSList    = QList<ChannelExtensionSPtr>;
  using ChannelExtensionSMap     = QMap<QString, ChannelExtensionSPtr>;
  using ChannelExtensionTypeList = QList<ChannelExtension::Type>;

  QString extensionFile(const ChannelSPtr &channel);

} // namespace EspINA

#endif // ESPINA_CHANNEL_EXTENSION_H
