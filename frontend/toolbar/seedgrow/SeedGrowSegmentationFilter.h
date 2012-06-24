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
#include <common/model/Segmentation.h>

// #include "ui_SeedGrowSegmentationFilterSetup.h"
// class IVOI;

class vtkImageData;
class vtkConnectedThresholdImageFilter;
static const QString SGSF = "SeedGrowSegmentation::SeedGrowSegmentationFilter";

class Channel;
class pqFilter;

class SeedGrowSegmentationFilter
: public Filter
{
  class SetupWidget;
Q_OBJECT
public:
  static const ModelItem::ArgumentId CHANNEL;
  static const ModelItem::ArgumentId SEED;
  static const ModelItem::ArgumentId LTHRESHOLD;
  static const ModelItem::ArgumentId UTHRESHOLD;
  static const ModelItem::ArgumentId VOI;

  class SArguments : public Arguments
  {
  public:
    explicit SArguments(){}
    explicit SArguments(const ModelItem::Arguments args);

    virtual ArgumentId argumentId(QString name) const
    {
      if (CHANNEL == name)
	return CHANNEL;
      if (SEED == name)
	return SEED;
      if (LTHRESHOLD == name)
	return LTHRESHOLD;
      if (UTHRESHOLD == name)
	return UTHRESHOLD;
      if (VOI == name)
	return VOI;
      return Arguments::argumentId(name);
    }

    void setInput(const QString input)
    {
      (*this)[CHANNEL] = input;
    }
    QString input() const {return (*this)[CHANNEL];}

    void setSeed(int seed[3])
    {
      memcpy(m_seed, seed, 3*sizeof(int));
      (*this)[SEED] = arg3(seed);
    }
    void seed(int value[3]) const {memcpy(value, m_seed, 3*sizeof(int));}

    void setLowerThreshold(int th)
    {
      m_threshold[0] = th;
      (*this)[LTHRESHOLD] = QString::number(th);
    }
    int lowerThreshold() const {return m_threshold[0];}

    void setUpperThreshold(int th)
    {
      m_threshold[1] = th;
      (*this)[UTHRESHOLD] = QString::number(th);
    }
    int upperThreshold() const {return m_threshold[1];}

    void setVOI(int voi[6])
    {
      memcpy(m_VOI, voi, 6*sizeof(int));
      (*this)[VOI] = arg6(voi);
    }
    void voi(int value[6]) const {memcpy(value, m_VOI, 6*sizeof(int));}
 
  private:
    int m_seed[3];
    int m_threshold[2];
    int m_VOI[6];
  };

public:
  explicit SeedGrowSegmentationFilter(SelectableItem *input, int seed[3], int threshold[2], int VOI[6]);
  /// Create a new filter from given arguments
  explicit SeedGrowSegmentationFilter(Arguments args);
  virtual ~SeedGrowSegmentationFilter();

  void run();

  void setInput(pqData data);
  void setThreshold(int th);
  int threshold() const {return m_args.lowerThreshold();}
  void setSeed(int seed[3]);
  void seed(int seed[3]) const {m_args.seed(seed);}
  void setVOI(int VOI[6]);
  void voi(int VOI[6]) const {m_args.voi(VOI);}

  /// Implements Model Item Interface
  virtual QString  id() const;
  virtual QVariant data(int role) const;
  virtual QString  serialize() const;

  /// Implements Filter Interface
  pqData preview();
  virtual int numProducts() const;
  virtual Segmentation *product(int index) const;
  virtual QWidget* createConfigurationWidget();

protected:
  virtual vtkAlgorithmOutput* output(unsigned int outputNb);

private:
  SArguments m_args;
  pqFilter *grow, *extract, *close;
  pqFilter *segFilter;
  vtkImageData *m_segImg;
  vtkAlgorithmOutput *m_filterOutput;

  Segmentation *m_seg;
  friend class SetupWidget;
};

#endif // SEEDGROWSEGMENTATIONFILTER_H
