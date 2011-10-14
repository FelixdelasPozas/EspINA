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


#ifndef VESICLEVALIDATORFILTER_H
#define VESICLEVALIDATORFILTER_H

#include <filter.h>
#include <processingTrace.h>

class vtkSMProxy;
class RectangularVOI;

class VesicleValidatorFilter 
: public EspinaFilter
, public ITraceNode
{
public:
  explicit VesicleValidatorFilter(EspinaProduct *sample, const Point &pos, double SVA[6]);
  explicit VesicleValidatorFilter(const ITraceNode::Arguments &args);
//   explicit VesicleValidatorFilter(const RectangularVOI &SVA);
  // Implement IFilter interface
  virtual int numProducts();
  virtual vtkProduct product(int i);
  virtual QList<vtkProduct *> products();
  virtual QString getFilterArguments() const {
    return EspinaFilter::getFilterArguments();
  }
  
  // Implement EspinaFilter interface
  virtual void removeProduct(vtkProduct* product);
  virtual QWidget* createWidget();
  
  // Implement ITraceNode interface
  virtual QString label() const {
    return getArgument("Type");
  }
  virtual QString getArguments() const {
    return m_args;
  }
  virtual QString getArgument(QString name) const;
  
  void validateVesicle(const Point &pos);
  
private:
//   const RectangularVOI &m_SVA;
  double m_SVA[6];
  vtkSMProxy *m_SVA2;
  QList<vtkProduct *> m_vesicles;
};

#endif // VESICLEVALIDATORFILTER_H
