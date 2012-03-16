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

#include "common/selection/SelectableItem.h"

#include "common/processing/pqData.h"
#include "common/File.h"
#include <QColor>

class ChannelExtension;
class ChannelExtension;
// Forward declarations
class pqOutputPort;
class pqPipelineSource;

class Channel : public SelectableItem
{
public:
  static const QString ID;
  static const QString COLOR;

  static const QString NAME;
  static const QString MARGIN;
  static const QString VOLUME;

  class CArguments : public ModelItem::Arguments
  {
  public:
    explicit CArguments(){}
//     explicit CArguments(const QMap< QString, QString >& args) : Arguments(args){}
//     explicit CArguments(const QString args) : Arguments(args) {}
    explicit CArguments(const Arguments args) : Arguments(args) {}
    void setColor(double color)
    {
      (*this)[COLOR] = QString::number(color);
    }

    double color() const
    {
      return (*this)[COLOR].toFloat();
    }
  };

public:
  explicit Channel(const QString file, pqData data);
  explicit Channel(const QString file, const Arguments args);
  virtual ~Channel();

  pqOutputPort *outputPort();
  void extent(int val[6]);
  void bounds(double val[6]);
  void spacing(double val[3]);

  void setPosition(int pos[3]);
  void position(int pos[3]);

  void setColor(double color);
  double color() const;

  void setVisible(bool visible) {m_visible = visible;}
  bool isVisible() const {return m_visible;}

  /// Model Item Interface
  virtual QString id() const {return File::name(m_args[ID]);}
  virtual QVariant data(int role) const;
  virtual ItemType type() const {return ModelItem::CHANNEL;}
  virtual QString  serialize() const;

  virtual QStringList availableInformations() const;
  virtual QStringList availableRepresentations() const;
  virtual QVariant information(QString name) const;

  /// Selectable Item Interface
  virtual pqData volume() {return m_data;}

  /// Add a new extension to the segmentation
  /// Extesion won't be available until requirements are satisfied
  void addExtension(ChannelExtension *ext);
  void initialize();

private:
  pqData m_data;
  int    m_extent[6];
  double m_bounds[6], m_spacing[3];
  int    m_pos[3];/*in nm*/
  bool   m_visible;

  CArguments m_args;

  pqFilter *m_spacingFilter;
//   QList<Segmentation *> m_segs;

//   QMap<ExtensionId, IChannelExtension *> m_extensions;
//   QMap<ExtensionId, IChannelExtension *> m_pendingExtensions;
//   QList<IChannelExtension *> m_insertionOrderedExtensions;
//   QMap<IChannelRepresentation::RepresentationId, IChannelExtension *> m_representations;
};

#endif // CHANNEL_H