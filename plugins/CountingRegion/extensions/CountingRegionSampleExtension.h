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

class BoundingRegion;

class CountingRegionSampleExtension
: public SampleExtension
{
public:
  static const QString ID;

  explicit CountingRegionSampleExtension();;
  virtual ~CountingRegionSampleExtension();

  virtual QString id();
  virtual void initialize(Sample* sample);

  virtual QStringList dependencies() const
  {return SampleExtension::dependencies();}

  virtual QStringList availableRepresentations() const
  {return SampleExtension::availableRepresentations();}


  virtual QStringList availableInformations() const
  {return SampleExtension::availableInformations();}

  virtual QVariant information(QString info) const
  {return SampleExtension::information(info);}

  virtual SampleExtension *clone();

  void addRegion(BoundingRegion *region);
  QList<BoundingRegion *> regions() const {return m_regions;}

private:
  QList<BoundingRegion *> m_regions;
};

#endif // COUNTINGREGIONSAMPLEEXTENSION_H
