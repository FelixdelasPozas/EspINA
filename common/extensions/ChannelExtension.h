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


#ifndef CHANNELEXTENSION_H
#define CHANNELEXTENSION_H

#include "common/extensions/ModelItemExtension.h"

#include <QSharedPointer>
#include <QStringList>
#include <QVariant>

class Channel;

/// Interface to extend channel's behaviour
class ChannelExtension
: public ModelItemExtension
{
public:
  typedef QSharedPointer<ChannelExtension> SPtr;

public:
  virtual ~ChannelExtension(){}

  virtual void initialize(Channel *channel) = 0;

  virtual Channel *channel() const {return m_channel;}

  /// Prototype
  virtual ChannelExtension *clone() = 0;

protected:
  explicit ChannelExtension() : m_channel(NULL){}

  Channel *m_channel;
};

#endif // CHANNELEXTENSION_H
