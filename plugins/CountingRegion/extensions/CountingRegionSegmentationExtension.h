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
class BoundingRegion;
class Segmentation;

class CountingRegionSegmentationExtension
: public SegmentationExtension
{
  Q_OBJECT
public:
  static const ExtId ID;
  static const InfoTag DISCARTED;

public:
  explicit CountingRegionSegmentationExtension();
  virtual ~CountingRegionSegmentationExtension();

  virtual ExtId id();
  virtual void initialize(Segmentation* seg);

  virtual ExtIdList dependencies() const;

  virtual InfoList availableInformations() const
  { return SegmentationExtension::availableInformations(); }

  virtual RepList availableRepresentations() const
  { return SegmentationExtension::availableRepresentations(); }

  virtual QVariant information(InfoTag tag) const;

  virtual SegmentationRepresentation *representation(QString rep);

  void setBoundingRegions(QList<BoundingRegion *> bRegions);

  virtual SegmentationExtension* clone();

protected slots:
  void evaluateBoundingRegions();

private:
  bool m_isDiscarted;
  QList<BoundingRegion *> m_boundingRegions;
};

#endif // COUNTINGREGIONSEGMENTATIONEXTENSION_H