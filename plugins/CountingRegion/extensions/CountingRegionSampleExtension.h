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

#ifndef COUNTINGREGIONSAMPLEEXTENSION_H
#define COUNTINGREGIONSAMPLEEXTENSION_H

#include <common/extensions/SampleExtension.h>

class CountingRegion;
class BoundingRegion;

class CountingRegionSampleExtension
: public SampleExtension
{
public:
  static const ExtId ID;
  static const ModelItem::ArgumentId REGIONS;

  explicit CountingRegionSampleExtension(CountingRegion *plugin);;
  virtual ~CountingRegionSampleExtension();

  virtual ExtId id();
  virtual void initialize(Sample *sample, ModelItem::Arguments args = ModelItem::Arguments());

  virtual ModelItemExtension::ExtIdList dependencies() const
  { return SampleExtension::dependencies(); }

  virtual ModelItemExtension::InfoList availableInformations() const
  { return SampleExtension::availableInformations(); }

  virtual RepList availableRepresentations() const
  { return SampleExtension::availableRepresentations(); }

  virtual QVariant information(InfoTag tag) const
  { return SampleExtension::information(tag); }

  virtual SampleExtension *clone();

  void addRegion(BoundingRegion *region);
  void removeRegion(BoundingRegion *region);
  QList<BoundingRegion *> regions() const {return m_regions;}

private:
  CountingRegion         *m_plugin;
  QList<BoundingRegion *> m_regions;
};

#endif // COUNTINGREGIONSAMPLEEXTENSION_H
