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

#include <itkImage.h>
#include <itkVTKImageToImageFilter.h>
#include <itkImageToVTKImageFilter.h>
#include <itkConnectedThresholdImageFilter.h>
#include <itkStatisticsLabelObject.h>
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkExtractImageFilter.h>

class vtkImageData;
class vtkConnectedThresholdImageFilter;

class Channel;
class pqFilter;

class SeedGrowSegmentationFilter
: public Filter
{
  Q_OBJECT
  class SetupWidget;
  typedef itk::ExtractImageFilter<EspinaVolume, EspinaVolume> ExtractType;
  typedef itk::ConnectedThresholdImageFilter<EspinaVolume, EspinaVolume> ConnectedThresholdFilterType;
  typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
  typedef itk::LabelMap<LabelObjectType> LabelMapType;
  typedef itk::LabelImageToShapeLabelMapFilter<EspinaVolume, LabelMapType> Image2LabelFilterType;
  typedef itk::BinaryBallStructuringElement<EspinaVolume::PixelType, 3> StructuringElementType;
  typedef itk::BinaryMorphologicalClosingImageFilter<EspinaVolume, EspinaVolume, StructuringElementType> bmcifType;

public:
  static const QString TYPE;

  static const ModelItem::ArgumentId SEED;
  static const ModelItem::ArgumentId LTHRESHOLD;
  static const ModelItem::ArgumentId UTHRESHOLD;
  static const ModelItem::ArgumentId VOI;
  static const ModelItem::ArgumentId CLOSE;

  class Parameters
  {
  public:
    explicit Parameters(Arguments &args);

    virtual ArgumentId argumentId(QString name) const
    {
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

    void setSeed(int seed[3])
      {m_args[SEED] = arg3(seed);}
    EspinaVolume::IndexType seed() const
    {
      EspinaVolume::IndexType res;
      QStringList values = m_args[SEED].split(",");
      for(int i=0; i<3; i++)
        res[i] = values[i].toInt();

      return res;
    }

    void setLowerThreshold(int th)
      {m_args[LTHRESHOLD] = QString::number(th);}

    int lowerThreshold() const
      {return m_args.value(LTHRESHOLD, 0).toInt();}

    void setUpperThreshold(int th)
      {m_args[UTHRESHOLD] = QString::number(th);}
    int upperThreshold() const {return m_args.value(UTHRESHOLD, 0).toInt();}

    void setVOI(int voi[6])
    {
      m_args[VOI] = arg6(voi);
    }
    void voi(int value[6]) const
    {
      QStringList values = m_args[VOI].split(",");
      for(int i=0; i<6; i++)
        value[i] = values[i].toInt();
    }

    void setCloseValue(int value)
      {m_args[CLOSE] = QString::number(value);}
    int closeValue() const
      {return m_args[CLOSE].toInt();}

  private:
    Arguments &m_args;
  };

public:
  explicit SeedGrowSegmentationFilter(NamedInputs inputs,
                                      Arguments args);
  virtual ~SeedGrowSegmentationFilter();


  void setLowerThreshold(int th);
  int lowerThreshold() const {return m_param.lowerThreshold();}
  void setUpperThreshold(int th);
  int upperThreshold() const {return m_param.upperThreshold();}
  void setSeed(int seed[3]);
  void seed(int seed[3]) const;
  void setVOI(int VOI[6]);
  void voi(int VOI[6]) const {m_param.voi(VOI);}

  /// Implements Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;

  /// Implements Filter Interface
  virtual bool needUpdate() const;
  void run();
  virtual int numberOutputs() const;
  virtual EspinaVolume* output(OutputNumber i) const;
  virtual bool prefetchFilter();

  virtual QWidget* createConfigurationWidget();

private:
  bool         m_needUpdate;
  Parameters   m_param;
  EspinaVolume *m_input;
  EspinaVolume *m_volume;
  EspinaVolumeReader::Pointer m_cachedFilter;

  ExtractType::Pointer extractFilter;
  ConnectedThresholdFilterType::Pointer ctif;
  Image2LabelFilterType::Pointer image2label;
  bmcifType::Pointer bmcif;

  friend class SetupWidget;
};

#endif // SEEDGROWSEGMENTATIONFILTER_H
