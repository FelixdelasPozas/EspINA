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


#ifndef FREEFORMSOURCE_H
#define FREEFORMSOURCE_H

#include <model/Filter.h>
#include <common/views/vtkSliceView.h>

#include <itkChangeInformationImageFilter.h>

#include <QVector3D>


class FreeFormSource
: public Filter
{
  typedef itk::ChangeInformationImageFilter<EspinaVolume> FilterType;
public:
  static const QString TYPE;

  static const ModelItem::ArgumentId SPACING;

  class FreeFormArguments : public Arguments
  {
  public:
    explicit FreeFormArguments(){}
    explicit FreeFormArguments(const Arguments args) : Arguments(args){}

    void setSpacing(double value[3])
    {
      (*this)[SPACING] = QString("%1,%2,%3")
                         .arg(value[0])
                         .arg(value[1])
                         .arg(value[2]);
    }
    EspinaVolume::SpacingType spacing() const
    {
      EspinaVolume::SpacingType res;
      QStringList values = (*this)[SPACING].split(",");
      for(int i=0; i<3; i++)
        res[i] = values[i].toDouble();
      return res;
    }
  };
public:
  explicit FreeFormSource(NamedInputs inputs,
                          Arguments args);
  virtual ~FreeFormSource();

  void draw(vtkSliceView::VIEW_PLANE plane,  QVector3D center, int radius = 0);
  void erase(vtkSliceView::VIEW_PLANE plane, QVector3D center, int radius = 0);

  /// Implements Model Item Interface
  virtual QString id() const;
  virtual QVariant data(int role) const;
  virtual QString serialize() const;

  /// Implements Filter Interface
  virtual int numberOutputs() const;
  virtual EspinaVolume* output(OutputNumber i) const;
  virtual void run(){}

  virtual QWidget* createConfigurationWidget();

protected:
  EspinaVolume::RegionType region(int extent[6]) const;

private:
  FreeFormArguments m_args;

  bool   m_hasPixels;
  bool   m_init;
  int    Extent[6];
  int    DrawExtent[6];
  double Spacing[3];
  EspinaVolume::Pointer m_volume;
  FilterType::Pointer m_filter;
};

#endif // FREEFORMSOURCE_H
