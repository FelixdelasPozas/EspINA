/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SEEDGROWSEGMENTATIONFILTER_H
#define SEEDGROWSEGMENTATIONFILTER_H

#include <filter.h>

class IVOI;

class SeedGrowSegmentationFilter 
: public EspinaFilter
{
public:
  SeedGrowSegmentationFilter(EspinaFilter::Arguments &args);
  
  //! Implements IFilter Interface
  virtual int numProducts() {return 1;}
  virtual vtkProduct *product(int i) {return NULL;}
  virtual QList<vtkProduct *> products() {QList<vtkProduct*> a; return a;}
  
  virtual QString label() {return "SeedGrowSegmentation";}
  virtual QString getArguments() const;

private:
  IVOI *m_voi;
  vtkFilter *m_grow;
};


#endif // SEEDGROWSEGMENTATIONFILTER_H
