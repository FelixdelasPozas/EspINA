/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

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


#ifndef COUNTINGREGIONCHANNELEXTENSION_H
#define COUNTINGREGIONCHANNELEXTENSION_H

#include "common/extensions/ChannelExtension.h"

class BoundingRegion;
class CountingRegion;
class ViewManager;

class CountingRegionChannelExtension
: public ChannelExtension
{
public:
  static const ExtId ID;
  static const ModelItem::ArgumentId REGIONS;

  explicit CountingRegionChannelExtension(CountingRegion *plugin, ViewManager *vm);
  virtual ~CountingRegionChannelExtension();

  virtual ModelItemExtension::ExtId id()
  { return ID; }

  virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());
  virtual QString serialize() const;

  virtual ModelItemExtension::ExtIdList dependencies() const;

  virtual ModelItemExtension::InfoList availableInformations() const
  { return ChannelExtension::availableInformations(); }

  virtual ModelItemExtension::RepList availableRepresentations() const
  { return ChannelExtension::availableRepresentations(); }

  virtual QVariant information(ModelItemExtension::InfoTag tag) const
  { return ChannelExtension::information(tag); }

  virtual ChannelExtension* clone();

  void addRegion(BoundingRegion *region);
  void removeRegion(BoundingRegion *region);
  QList<BoundingRegion *> regions() const {return m_regions;}

private:
  CountingRegion         *m_plugin;
  ViewManager            *m_viewManager;
  QList<BoundingRegion *> m_regions;
  mutable ModelItem::Arguments    m_args;
};

#endif // COUNTINGREGIONCHANNELEXTENSION_H
