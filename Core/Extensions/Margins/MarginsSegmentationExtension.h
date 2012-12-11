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


#ifndef MARGINSSEGMENTATIONEXTENSION_H
#define MARGINSSEGMENTATIONEXTENSION_H

#include "Core/Extensions/SegmentationExtension.h"
#include "Core/EspinaTypes.h"


class MarginsSegmentationExtension
: public SegmentationExtension
{
public:
  static const ExtId ID;

  static const InfoTag LEFT_MARGIN;
  static const InfoTag TOP_MARGIN;
  static const InfoTag UPPER_MARGIN;
  static const InfoTag RIGHT_MARGIN;
  static const InfoTag BOTTOM_MARGIN;
  static const InfoTag LOWER_MARGIN;

  explicit MarginsSegmentationExtension();
  virtual ~MarginsSegmentationExtension();

  virtual ExtId id();

  virtual ModelItemExtension::ExtIdList dependencies() const
  { return SegmentationExtension::dependencies(); }

  virtual InfoList availableInformations() const
  { return SegmentationExtension::availableInformations(); }

  virtual RepList availableRepresentations() const
  { return SegmentationExtension::availableRepresentations(); }

  virtual QVariant information(InfoTag tag) const;

  virtual SegmentationRepresentation *representation(QString rep);

  virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());

  virtual SegmentationExtension* clone();

  void margins(Nm margins[6]) const
  {
    updateDistances();
    memcpy(margins, m_distances, 6*sizeof(Nm));
  }

private:
  void updateDistances() const;
  void setMargins(Nm distances[6]);

private:
  mutable Nm m_distances[6];

  friend class MarginsChannelExtension;
};

#endif // MARGINSSEGMENTATIONEXTENSION_H