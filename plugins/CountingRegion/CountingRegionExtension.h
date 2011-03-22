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


#ifndef COUNTINGREGIONEXTENSION_H
#define COUNTINGREGIONEXTENSION_H

#include "EspinaPlugin.h"
#include <QList>

// Forward declaration
class CountingRegion;
class Segmentation;
class pqPipelineSource;

class CountingRegionExtension : public ISegmentationExtension
{
public:
  static const ExtensionId ID;
  
public:
  //! Implement ISegmentationExtension 
  virtual ExtensionId id() {return "CountinRegionExtension";}
  virtual void initialize(Segmentation *seg); //TODO: Use Segmentation or pqPipelineSource
  virtual void addInformation(InformationMap& map);
  virtual void addRepresentations(RepresentationMap& map);
  
  virtual ISegmentationExtension* clone();
  
  void updateRegions(QList<pqPipelineSource *> &regions);
  
  //TODO: Make it private
  CountingRegionExtension(CountingRegion *manager) 
  : m_manager(manager)
  , m_countingRegion(NULL)
  {}
  
private:
  CountingRegion *m_manager;
  pqPipelineSource *m_countingRegion;
};

#endif // COUNTINGREGIONEXTENSION_H
