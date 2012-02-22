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
#include <common/processing/pqFilter.h>

/// Bounding Regions' base class
class BoundingRegion : public QStandardItem
{
public:
  explicit BoundingRegion(int left,  int top,    int upper,
			  int right, int bottom, int lower );
  virtual ~BoundingRegion(){}

  /// Return total volume in pixels
  unsigned int totalVolume();
  /// Return inclusion volume in pixels
  unsigned int inclusionVolume();
  /// Return exclusion volume in pixels
  unsigned int exclusionVolume();

  virtual void setInclusive(int left, int top, int upper){};
  virtual void setExclusive(int right, int bottom, int lower){};

private:
  pqFilter *m_boundingRegion;
  int m_inclusion[3];
  int m_exclusion[3];
};

#endif // BOUNDINGREGION_H
