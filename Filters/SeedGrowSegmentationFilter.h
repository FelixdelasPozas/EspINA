/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
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
#ifndef ESPINA_SEED_GROW_SEGMENTATION_FILTER_H
#define ESPINA_SEED_GROW_SEGMENTATION_FILTER_H

//#include "EspinaFilters_Export.h"

#include "Core/Analysis/Filter.h"
#include <Core/Utils/BinaryMask.h>

// #include <itkImage.h>
// #include <itkVTKImageToImageFilter.h>
// #include <itkImageToVTKImageFilter.h>
// #include <itkConnectedThresholdImageFilter.h>
// #include <itkStatisticsLabelObject.h>
// #include <itkLabelImageToShapeLabelMapFilter.h>
// #include <itkBinaryBallStructuringElement.h>
// #include <itkBinaryMorphologicalClosingImageFilter.h>
// #include <itkExtractImageFilter.h>

class vtkImageData;
class vtkConnectedThresholdImageFilter;

namespace EspINA
{
  
  class SeedGrowSegmentationFilter
  : public Filter
  {
  public:
    explicit SeedGrowSegmentationFilter(OutputSList inputs, Type type, SchedulerSPtr scheduler);

    void setLowerThreshold(int th);

    int lowerThreshold() const;

    void setUpperThreshold(int th);
    int upperThreshold() const;

    // Convenience method to set symmetrical lower/upper thresholds
    void setThreshold(int th)
    {
      setLowerThreshold(th);
      setUpperThreshold(th);
    };

    void setSeed(Nm seed[3]);
    void seed(Nm seed[3])const;

    void setVOI(const Bounds& bounds);

    template<typename T>
    void setVOI(const BinaryMask<T>& mask);

    template<typename T>
    BinaryMask<T> voi() const;

    void setClosingRadius(unsigned char value);

    unsigned char closingRadius();

    virtual OutputSPtr output(Output::Id id) const;

  protected:
    virtual Snapshot saveFilterSnapshot() const;
    
    virtual bool needUpdate() const;
    
    virtual bool needUpdate(Output::Id id) const;
    
    virtual DataSPtr createDataProxy(Output::Id id, const Data::Type& type);
    
    virtual void execute();
    
    virtual void execute(Output::Id id);
    
    virtual bool invalidateEditedRegions();
  };

//   class ViewManager;
// 
//   class SeedGrowSegmentationFilter
//   : public BasicSegmentationFilter
//   {
//     typedef itk::ExtractImageFilter<itkVolumeType, itkVolumeType> ExtractType;
//     typedef itk::ConnectedThresholdImageFilter<itkVolumeType, itkVolumeType> ConnectedThresholdFilterType;
//     typedef itk::StatisticsLabelObject<unsigned int, 3> LabelObjectType;
//     typedef itk::LabelMap<LabelObjectType> LabelMapType;
//     typedef itk::LabelImageToShapeLabelMapFilter<itkVolumeType, LabelMapType> Image2LabelFilterType;
//     typedef itk::BinaryBallStructuringElement<itkVolumeType::PixelType, 3> StructuringElementType;
//     typedef itk::BinaryMorphologicalClosingImageFilter<itkVolumeType, itkVolumeType, StructuringElementType> bmcifType;
// 
//   public:
//     static const QString INPUTLINK;
// 
//     static const ModelItem::ArgumentId SEED;
//     static const ModelItem::ArgumentId LTHRESHOLD;
//     static const ModelItem::ArgumentId UTHRESHOLD;
//     static const ModelItem::ArgumentId VOI;
//     static const ModelItem::ArgumentId CLOSE;
// 
//     class EspinaCore_EXPORT Parameters
//     {
//     public:
//       explicit Parameters(Arguments &args);
// 
//       void setSeed(itkVolumeType::IndexType seed)
//       {
//         m_args[SEED] = QString("%1,%2,%3")
//         .arg(seed[0])
//         .arg(seed[1])
//         .arg(seed[2]);
//       }
//       itkVolumeType::IndexType seed() const
//       {
//         itkVolumeType::IndexType res;
//         QStringList values = m_args[SEED].split(",");
//         for(int i=0; i<3; i++)
//           res[i] = values[i].toInt();
// 
//         return res;
//       }
// 
//       void setLowerThreshold(int th)
//       {m_args[LTHRESHOLD] = QString::number(th);}
// 
//       int lowerThreshold() const
//       {return m_args.value(LTHRESHOLD, 0).toInt();}
// 
//       void setUpperThreshold(int th)
//       {m_args[UTHRESHOLD] = QString::number(th);}
//       int upperThreshold() const {return m_args.value(UTHRESHOLD, 0).toInt();}
// 
//       void setVOI(int voi[6])
//       {
//         m_args[VOI] = arg6(voi);
//       }
//       void voi(int value[6]) const
//       {
//         QStringList values = m_args[VOI].split(",");
//         for(int i=0; i<6; i++)
//           value[i] = values[i].toInt();
//       }
// 
//       void setCloseValue(int value)
//       {m_args[CLOSE] = QString::number(value);}
//       int closeValue() const
//       {return m_args[CLOSE].toInt();}
// 
//     private:
//       Arguments &m_args;
//     };
// 
//   public:
// 
//     void setSeed(itkVolumeType::IndexType seed, bool ignoreUpdate = false);
//     itkVolumeType::IndexType seed() const;
// 
//     void setVOI(int VOI[6], bool ignoreUpdate = false);
//     void voi(int VOI[6]) const {m_param.voi(VOI);}
// 
//     unsigned int closeValue() {return m_param.closeValue();}
//     void setCloseValue(unsigned int value, bool ignoreUpdate = false)
//     {m_param.setCloseValue(value); m_ignoreCurrentOutputs = !ignoreUpdate; }
// 
//     bool isTouchingVOI() const;
// 
//   protected:
//     virtual bool ignoreCurrentOutputs() const
//     { return m_ignoreCurrentOutputs; }
// 
//     virtual bool needUpdate(FilterOutputId oId) const;
// 
//     virtual void run();
// 
//     virtual void run(FilterOutputId oId);
// 
//   private:
//     bool       m_ignoreCurrentOutputs;
//     Parameters m_param;
// 
//     ConnectedThresholdFilterType::Pointer ctif;
//     ExtractType::Pointer                  voiFilter;
//     bmcifType::Pointer                    bmcif;
// 
//     friend class FilterInspector;
//   };

} // namespace EspINA

#endif // ESPINA_SEED_GROW_SEGMENTATION_FILTER_H
