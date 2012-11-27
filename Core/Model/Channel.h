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

#include "Core/Model/PickableItem.h"

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
: public PickableItem
{
  typedef itk::ImageFileReader<EspinaVolume> EspinaVolumeReader;
public:

public:
  // Argument Ids
  static const ArgumentId ID;
  static const ArgumentId COLOR;
  static const ArgumentId VOLUME;

  static const QString STAINLINK;
  static const QString LINK;
  static const QString VOLUMELINK;

// Extended Information and representation tags
  static const QString NAME;
  static const QString VOLUMETRIC;

  class CArguments : public ModelItem::Arguments
  {
  public:
    explicit CArguments() : m_outputId(0) {}
    explicit CArguments(const Arguments &args)
    : Arguments(args), m_outputId(0) {}

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

    void setOutputId(Filter::OutputId oId)
    {
      (*this)[VOLUME] = QString("%1_%2")
                        .arg(VOLUMELINK)
                        .arg(oId);
      m_outputId = oId;
    }

    Filter::OutputId outputId() const
    {
      return m_outputId;
    }

  private:
    Filter::OutputId m_outputId;
    //double m_spacing[3];
  };

public:
  virtual ~Channel();

  void extent(int out[6]);
  void bounds(double out[6]);
  void spacing(double out[3]);

  void setPosition(Nm pos[3]);
  void position(Nm pos[3]);

  void setColor(double color);
  double color() const;

  void setVisible(bool visible) {m_visible = visible;}
  bool isVisible() const {return m_visible;}

  /// Model Item Interface
  virtual QVariant data(int role=Qt::DisplayRole) const;
  virtual bool setData(const QVariant& value, int role = Qt::UserRole +1);
  virtual ItemType type() const {return ModelItem::CHANNEL;}
  virtual QString  serialize() const;

  virtual QStringList availableInformations() const;
  virtual QStringList availableRepresentations() const;
  virtual QVariant information(QString name);

  virtual void initialize(Arguments args = Arguments());
  virtual void initializeExtensions(Arguments args = Arguments());

  /// Get the sample which channel belongs to
  Sample *sample();

  /// Pickable Item Interface
  virtual const Filter* filter() const;
  virtual Filter* filter() { return PickableItem::filter(); }
  virtual Filter::OutputId outputId();
  virtual EspinaVolume *itkVolume();

  // vtk image of the channel
  vtkAlgorithmOutput *vtkVolume();

  /// Add a new extension to the segmentation
  /// Extesion won't be available until requirements are satisfied
  void addExtension(ChannelExtension *ext);

public slots:
  virtual void notifyModification(bool force = false);

private:
  explicit Channel(Filter* filter, Filter::OutputId oId);
  friend class EspinaFactory;
private:
  bool   m_visible;
  Nm m_pos[3];

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
