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

#include <Core/Model/Filter.h>

#include <QVector3D>

class FreeFormSource
: public SegmentationFilter
{
public:
  static const QString TYPE;
  static const ModelItem::ArgumentId SPACING;

  class Parameters
  {
  public:
    explicit Parameters(Arguments &args)
    : m_args(args)
    {
      QStringList values = m_args[SPACING].split(",", QString::SkipEmptyParts);
      if (values.size() == 3)
      {
        for(int i=0; i<3; i++)
          m_spacing[i] = values[i].toDouble();
      }
    }

    void setSpacing(double value[3])
    {
      for(int i=0; i<3; i++)
        m_spacing[i] = value[i];
      m_args[SPACING] = QString("%1,%2,%3")
      .arg(value[0])
      .arg(value[1])
      .arg(value[2]);
    }
    void setSpacing(itkVolumeType::SpacingType value)
    {
      for(int i=0; i<3; i++)
        m_spacing[i] = value[i];
      m_args[SPACING] = QString("%1,%2,%3")
      .arg(value[0])
      .arg(value[1])
      .arg(value[2]);
    }
    itkVolumeType::SpacingType spacing() const
    {
      return m_spacing;
    }
  private:
    Arguments &m_args;
    itkVolumeType::SpacingType m_spacing;
  };

public:
  explicit FreeFormSource(NamedInputs inputs,
                          Arguments args);
  virtual ~FreeFormSource();

  virtual void draw(OutputId oId,
                    vtkImplicitFunction* brush,
                    const Nm bounds[6],
                    itkVolumeType::PixelType value = SEG_VOXEL_VALUE);
  virtual void draw(OutputId oId,
                    itkVolumeType::IndexType index,
                    itkVolumeType::PixelType value = SEG_VOXEL_VALUE);
  virtual void draw(OutputId oId,
                    Nm x, Nm y, Nm z,
                    itkVolumeType::PixelType value = SEG_VOXEL_VALUE);

  /// Implements Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;

  /// Implements Filter Interface
  virtual bool needUpdate() const;

protected:
  virtual void run(){}

private:
  Parameters m_param;
};

#endif // FREEFORMSOURCE_H