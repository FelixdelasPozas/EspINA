/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#ifndef SEEDGROWSEGMENTATIONFILTER_H
#define SEEDGROWSEGMENTATIONFILTER_H

#include "common/model/Filter.h"
#include "common/processing/pqData.h"

// #include "ui_SeedGrowSegmentationFilterSetup.h"
// class IVOI;

static const QString SGSF = "SeedGrowSegmentation::SeedGrowSegmentationFilter";

class Channel;
class pqFilter;

class SeedGrowSegmentationFilter
: public Filter
{
//   class SetupWidget : public QWidget, public Ui_SeedGrowSegmentationFilterSetup
//   {
//   public:
//     SetupWidget(EspinaFilter *filter);
//   };
Q_OBJECT
public:
  explicit SeedGrowSegmentationFilter(pqData input, int seed[3], int threshold, int VOI[6]);
  /// Create a new filter from given arguments
  explicit SeedGrowSegmentationFilter(Arguments args);
  virtual ~SeedGrowSegmentationFilter();

  void run();

  void setInput(pqData data);
  void setThreshold(int th);
  int threshold() const {return m_threshold;}
  void setSeed(int seed[3]);
  void seed(int seed[3]) const {memcpy(seed,m_seed,3*sizeof(int));}
  void setVOI(int VOI[6]);
  void voi(int VOI[6]) const {memcpy(VOI, m_VOI, 6*sizeof(int));}

  /// Implements Model Item Interface
  virtual QString serialize() const;
  virtual QVariant data(int role) const;
  virtual ItemType type() const {return ModelItem::FILTER;}

  /// Implements Filter Interface
  pqData preview();
  virtual int numProducts() const;
  virtual pqData product(int index) const;
//   virtual QList<vtkProduct *> products() {return m_finalFilter->products();}
//   virtual QString getFilterArguments() const {return EspinaFilter::getFilterArguments();}
//   virtual void removeProduct(vtkProduct* product);


//   virtual QString label() const {return getArgument("Type");}
//   virtual QString getArgument(QString name) const {return (name=="Type")?"SeedGrowSegmentation::SeedGrowSegmentationFilter":"";}
//   virtual QString getArguments() const {return m_args;}

//   virtual QWidget* createWidget();
signals:
  void modified();

private:
  QString m_input;
  int m_seed[3];
  int m_threshold;
  int m_VOI[6];
  pqFilter *grow, *extract;

//   friend class SetupWidget;
};


#endif // SEEDGROWSEGMENTATIONFILTER_H
