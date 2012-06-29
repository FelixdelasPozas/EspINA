/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
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


#ifndef IMAGELOGICFILTER_H
#define IMAGELOGICFILTER_H

#include <model/Filter.h>
#include <model/Segmentation.h>

#include <itkChangeInformationImageFilter.h>
// #include <itkConstantPadImageFilter.h>
// #include <itkOrImageFilter.h>

class ImageLogicFilter
: public Filter
{
  typedef itk::ChangeInformationImageFilter<EspinaVolume> FilterType;

//   typedef itk::ConstantPadImageFilter<EspinaVolume, EspinaVolume> PadFilterType;
//   typedef itk::OrImageFilter<EspinaVolume, EspinaVolume, EspinaVolume> OrFilterType;
public:
  enum Operation
  {
    ADDITION,
    SUBSTRACTION,
    NOSIGN
  };
  static const QString TYPE;

  static const ModelItem::ArgumentId OPERATION;

  class Parameters
  {
  public:
    explicit Parameters(Arguments &args) : m_args(args) {}

    void setOperation(Operation op)
    {
      m_args[OPERATION] = QString::number(op);
    }
    Operation operation() const {return Operation(m_args[OPERATION].toInt());}
  private:
    Arguments &m_args;
  };

public:
  explicit ImageLogicFilter(NamedInputs inputs, Arguments args);
  virtual ~ImageLogicFilter();


  /// Implements Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;

  /// Implements Filter Interface
  virtual int numberOutputs() const;
  virtual EspinaVolume* output(OutputNumber i) const;
  virtual bool prefetchFilter();
  virtual bool needUpdate() const;
  void run();

protected:
  void addition();
  void substraction();

private:
  Parameters   m_param;
  QList<int *> m_inputExtents;
  int          m_outputExtent[6];

  EspinaVolume::Pointer m_volume;
  FilterType::Pointer   m_filter;
//   PadFilterType::Pointer m_pad1;
//   PadFilterType::Pointer m_pad2;
//   OrFilterType::Pointer  m_orFilter;
};



#endif // IMAGELOGICFILTER_H
