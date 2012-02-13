/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

//----------------------------------------------------------------------------
// File:    Channel.h
// Purpose: Model some processing done to a physical sample which make
//          samples visible to the user in a specific way
//----------------------------------------------------------------------------
#ifndef CHANNEL_H
#define CHANNEL_H

#include <selection/SelectableItem.h>

#include "processing/pqData.h"
#include <QColor>

// Forward declarations
class pqOutputPort;
class pqPipelineSource;

class Channel : public SelectableItem
{
public:
  Channel(pqData data);
  virtual ~Channel();

  pqOutputPort *outputPort();
  void extent(int val[6]);
  void bounds(double val[6]);
  void spacing(double val[3]);

  void setPosition(int pos[3]);
  void position(int pos[3]);

  void setColor(const double color);
  double color() const;

  void setOpacity(double opacity);
  double opacity() {return m_opacity;}

  /// Model Item Interface
  virtual QVariant data(int role) const;
  virtual ItemType type() const {return ModelItem::CHANNEL;}
  /// Selectable Item Interface
  virtual pqData volume() {return m_data;}

private:
  pqData m_data;
  int    m_extent[6];
  double m_bounds[6], m_spacing[3];
  int    m_pos[3];/*in nm*/

private:
  double m_color;
  double m_opacity;
//   QList<Segmentation *> m_segs;

//   QMap<ExtensionId, IChannelExtension *> m_extensions;
//   QMap<ExtensionId, IChannelExtension *> m_pendingExtensions;
//   QList<IChannelExtension *> m_insertionOrderedExtensions;
//   QMap<IChannelRepresentation::RepresentationId, IChannelExtension *> m_representations;
//   QMap<QString, IChannelExtension *> m_informations;
};

typedef QSharedPointer<Channel> ChannelPtr;

#endif // CHANNEL_H