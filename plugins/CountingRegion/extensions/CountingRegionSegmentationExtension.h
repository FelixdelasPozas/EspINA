/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef COUNTINGREGIONSEGMENTATIONEXTENSION_H
#define COUNTINGREGIONSEGMENTATIONEXTENSION_H

#include <common/extensions/SegmentationExtension.h>


// Forward declaration
class pqFilter;
class Segmentation;
class BoundingRegion;

class CountingRegionSegmentationExtension
: public SegmentationExtension
{
public:
  static const QString ID;
  static const QString Discarted;

public:
  explicit CountingRegionSegmentationExtension();
  virtual ~CountingRegionSegmentationExtension();

  virtual QString id();
  virtual void initialize(Segmentation* seg);

  virtual QStringList dependencies() const
    {return SegmentationExtension::dependencies();}

  virtual QStringList availableRepresentations() const
    {return SegmentationExtension::availableRepresentations();}

  virtual SegmentationRepresentation *representation(QString rep);

  virtual QStringList availableInformations() const
    {return SegmentationExtension::availableInformations();}

  virtual QVariant information(QString info) const;

  void updateRegions(QList<BoundingRegion *> regions);

  virtual SegmentationExtension* clone();

private:
  pqFilter *m_countingRegion;
};

#endif // COUNTINGREGIONSEGMENTATIONEXTENSION_H