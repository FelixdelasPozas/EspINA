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


#ifndef BOUNDINGREGION_H
#define BOUNDINGREGION_H

#include <QStandardItemModel>
#include <common/widgets/RectangularSelection.h>

#include <common/processing/pqFilter.h>

class CountingRegionSampleExtension;

/// Bounding Regions' base class
class BoundingRegion
: public QStandardItem
, public QObject
, public EspinaWidget
{
public:
  enum Role
  {
    DescriptionRole = Qt::UserRole + 1
  };
public:
  explicit BoundingRegion(CountingRegionSampleExtension *sampleExt,
			  double inclusion[3],
			  double exclusion[3]);
  virtual ~BoundingRegion(){}

  virtual QVariant data(int role = Qt::UserRole + 1) const;

  /// Return total volume in pixels
  double totalVolume() const;
  /// Return inclusion volume in pixels
  double inclusionVolume() const;
  /// Return exclusion volume in pixels
  double exclusionVolume() const;

  virtual pqData region() const {return m_boundingRegion->data(0);}

protected:
  double left()  const {return m_inclusion[0];}
  double top()   const {return m_inclusion[1];}
  double upper() const {return m_inclusion[2];}
  double right() const {return m_exclusion[0];}
  double bottom()const {return m_exclusion[1];}
  double lower() const {return m_exclusion[2];}

protected:
  pqFilter *m_boundingRegion;
  CountingRegionSampleExtension *m_sampleExt;
  double  m_inclusion[3];
  double  m_exclusion[3];
};

#endif // BOUNDINGREGION_H
