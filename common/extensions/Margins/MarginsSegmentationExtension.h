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

#include "common/extensions/SegmentationExtension.h"


class MarginsSegmentationExtension
: public SegmentationExtension
{
  static const QString ID;

public:
  static const QString LeftMargin;
  static const QString TopMargin;
  static const QString UpperMargin;
  static const QString RightMargin;
  static const QString BottomMargin;
  static const QString LowerMargin;

  explicit MarginsSegmentationExtension();
  virtual ~MarginsSegmentationExtension();

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

  virtual SegmentationExtension* clone();

private:
  mutable double m_distances[6];
};

#endif // MARGINSSEGMENTATIONEXTENSION_H
