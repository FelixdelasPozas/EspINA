/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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

//----------------------------------------------------------------------------
// File:    Channel.h
// Purpose: Model some processing done to a physical sample which make
//          samples visible to the user in a specific way
//----------------------------------------------------------------------------
#ifndef CHANNEL_H
#define CHANNEL_H

#include "common/selection/SelectableItem.h"
#include <itkImageIOBase.h>
#include <itkImageFileReader.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkAlgorithmOutput.h>

#include <QColor>
#include <QFileInfo>

class Sample;
class Filter;
// Forward declarations
class ChannelExtension;
class vtkAlgorithmOutput;

class Channel
: public SelectableItem
{
  typedef itk::ImageFileReader<EspinaVolume> EspinaVolumeReader;

public:
  // Argument Ids
  static const ArgumentId ID;
  static const ArgumentId COLOR;
  static const ArgumentId VOLUME;

  static const QString STAINLINK;
  static const QString VOLUMELINK;

// Extended Information and representation tags
  static const QString NAME;
  static const QString VOLUMETRIC;

  class CArguments : public ModelItem::Arguments
  {
  public:
    explicit CArguments(){}
    explicit CArguments(const Arguments args) : Arguments(args) {}

    virtual ArgumentId argumentId(QString name) const
    {
      if (ID == name)
        return ID;
      if (COLOR == name)
        return COLOR;
      if (VOLUME == name)
        return VOLUME;

      return Arguments::argumentId(name);
    }

    /// Channel dye color. Hue's value in range (0,1)
    void setColor(double color)
    {
      (*this)[COLOR] = QString::number(color);
    }
    /// Channel dye color. Hue's value in range (0,1)
    double color() const
    {
      return (*this)[COLOR].toFloat();
    }

    void setOutputNumber(OutputNumber number)
    {
      (*this)[VOLUME] = QString("%1_%2")
                        .arg(VOLUMELINK)
                        .arg(number);
      m_outputNumber = number;
    }

    OutputNumber outputNumber() const
    {
      return m_outputNumber;
    }

  private:
    OutputNumber m_outputNumber;
    //double m_spacing[3];
  };

public:
  explicit Channel(Filter *filter, OutputNumber output);
  virtual ~Channel();

  void extent(int out[6]);
  void bounds(double out[6]);
  void spacing(double out[3]);

  void setPosition(double pos[3]);
  void position(double pos[3]);

  void setColor(double color);
  double color() const;

  void setVisible(bool visible) {m_visible = visible;}
  bool isVisible() const {return m_visible;}

  /// Model Item Interface
  virtual QString id() const {return m_args[ID];}
  virtual QVariant data(int role) const;
  virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);
  virtual ItemType type() const {return ModelItem::CHANNEL;}
  virtual QString  serialize() const;
  virtual void initialize(Arguments args = Arguments());

  virtual QStringList availableInformations() const;
  virtual QStringList availableRepresentations() const;
  virtual QVariant information(QString name);

  /// Get the sample which channel belongs to
  Sample *sample();

  /// Selectable Item Interface
  virtual Filter* filter();
  virtual OutputNumber outputNumber();
  virtual EspinaVolume *volume();

  // vtk image of the channel
  vtkAlgorithmOutput* image();

  /// Add a new extension to the segmentation
  /// Extesion won't be available until requirements are satisfied
  void addExtension(ChannelExtension *ext);

private:
  bool   m_visible;
  int    m_extent[6];
  double m_bounds[6];
  double m_pos[3];/*in nm*/

  Filter            *m_filter;
  mutable CArguments m_args;
//   QMap<ExtensionId, IChannelExtension *> m_extensions;
//   QMap<ExtensionId, IChannelExtension *> m_pendingExtensions;
//   QList<IChannelExtension *> m_insertionOrderedExtensions;
//   QMap<IChannelRepresentation::RepresentationId, IChannelExtension *> m_representations;
  typedef itk::ImageToVTKImageFilter<EspinaVolume> itk2vtkFilterType;
  itk2vtkFilterType::Pointer itk2vtk;
};
#endif // CHANNEL_H
