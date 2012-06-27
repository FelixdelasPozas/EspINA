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


#ifndef FILTER_H
#define FILTER_H

#include "common/model/ModelItem.h"

#include <EspinaTypes.h>

class Filter
: public ModelItem
{
public:
  typedef QMap<QString, Filter *> NamedInputs;

  static const ModelItem::ArgumentId ID;
  static const ModelItem::ArgumentId INPUTS;
  static const ModelItem::ArgumentId EDIT;
public:
  virtual ~Filter(){}

  // Implements Model Item Interface common to filters
  virtual ItemType type() const {return ModelItem::FILTER;}

  // Defines Filter's Interface
  /// Generate unique ID for current analysis.
  static QString generateId();
  /// Manually Edit Filter Output
  virtual void changePixelValue(int x,
                                int y,
                                int z,
                                EspinaVolume::PixelType value,
                                OutputNumber output){/*TODO*/}
  /// Specify how many outputs this filter generates
  virtual int numberOutputs() const = 0;
  /// Return the i-th output
  virtual EspinaVolume *output(OutputNumber i) const = 0;
  /// Updates filter outputs.
  /// If a snapshot exits it will try to load it
  void update();
  /// Method which actually executes the filter
  virtual void run() = 0;

  /// Return a widget used to configure filter's parameters
  virtual QWidget *createConfigurationWidget() = 0;

protected:
  /// Try to locate an snapshot of the filter in the hard drive
  virtual void prefetchFilter(){}

private:
  static unsigned int m_lastId;
};

#endif // FILTER_H
