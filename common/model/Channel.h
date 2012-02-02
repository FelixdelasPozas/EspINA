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

#include "IModelItem.h"

#include "paraview/pqData.h"

// Forward declarations
class pqOutputPort;
class pqPipelineSource;

class Channel : public IModelItem
{
public:
  Channel(pqData data);
  virtual ~Channel();

  pqOutputPort *outputPort();
  void extent(int val[6]);
  void bounds(double val[6]);
  void spacing(double val[3]);

  pqData data() const {return m_data;};

  virtual QVariant data(int role) const;
  virtual ItemType type() const {return IModelItem::CHANNEL;}

private:
  pqData m_data;
  int    m_extent[6];
  double m_bounds[6], m_spacing[3];

private:
//   QList<Segmentation *> m_segs;

//   QMap<ExtensionId, IChannelExtension *> m_extensions;
//   QMap<ExtensionId, IChannelExtension *> m_pendingExtensions;
//   QList<IChannelExtension *> m_insertionOrderedExtensions;
//   QMap<IChannelRepresentation::RepresentationId, IChannelExtension *> m_representations;
//   QMap<QString, IChannelExtension *> m_informations;
};

#endif // CHANNEL_H