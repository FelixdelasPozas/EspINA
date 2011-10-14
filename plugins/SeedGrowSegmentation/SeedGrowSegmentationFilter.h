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
#include <QPushButton>

#include "ui_SeedGrowSegmentationFilterSetup.h"

class IVOI;

class SeedGrowSegmentationFilter 
: public EspinaFilter
, public ITraceNode
{
  class SetupWidget : public QWidget, public Ui_SeedGrowSegmentationFilterSetup
  {
  public:
    SetupWidget(EspinaFilter *filter);
  };
  
public:
  
  //! Constructor interactivo
  SeedGrowSegmentationFilter(EspinaProduct *input, IVOI *voi, ITraceNode::Arguments &args);
  //! Constructor desde lista de argumentos
  SeedGrowSegmentationFilter(ITraceNode::Arguments &args);
  
  virtual ~SeedGrowSegmentationFilter();
  
  //! Implements IFilter Interface
  virtual int numProducts() {return m_numSeg;}
  virtual vtkProduct product(int i) {return vtkProduct(m_finalFilter->product(i).creator(),i);}
  virtual QList<vtkProduct *> products() {return m_finalFilter->products();}
  virtual QString getFilterArguments() const {return EspinaFilter::getFilterArguments();}
  virtual void removeProduct(vtkProduct* product);
  
  int threshold(){return m_threshold;}
  
  virtual QString label() const {return getArgument("Type");}
  virtual QString getArgument(QString name) const {return (name=="Type")?"SeedGrowSegmentation::SeedGrowSegmentationFilter":"";}
  virtual QString getArguments() const {return m_args;}
  
  virtual QWidget* createWidget();

private:
  EspinaFilter *m_applyFilter;
  vtkFilter *m_grow;
  EspinaFilter *m_restoreFilter;
  IFilter *m_finalFilter;
  int m_seed[3];
  int m_threshold;
  int m_numSeg;
  
  friend class SetupWidget;
};


#endif // SEEDGROWSEGMENTATIONFILTER_H
