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


#ifndef FILTER_H
#define FILTER_H

#include "common/model/ModelItem.h"

#include "common/EspinaTypes.h"
#include <itkImageFileReader.h>

class vtkImplicitFunction;
const QString CREATELINK = "CreateSegmentation";

class Filter
: public ModelItem
{
protected:
  typedef itk::ImageFileReader<EspinaVolume> EspinaVolumeReader;

public:
  typedef QMap<QString, Filter *> NamedInputs;

  static const ModelItem::ArgumentId ID;
  static const ModelItem::ArgumentId INPUTS;
  static const ModelItem::ArgumentId EDIT;
public:
  virtual ~Filter(){}

  void setId(QString id) {m_args[ID] = id;}

  // Implements Model Item Interface common to filters
  virtual ItemType type() const {return ModelItem::FILTER;}
  virtual QString id() const {return m_args[ID];}
  virtual void initialize(Arguments args = Arguments()){};
  virtual void initializeExtensions(Arguments args = Arguments()){};
  virtual QString serialize() const {return m_args.serialize();}


  static void resetId();
  static QString generateId();

  struct Link
  {
    Filter      *filter;
    OutputNumber outputPort;
  };

  ///WARNING: Current implementation will expand the image
  ///         when drawing with value = 0!

  /// Manually Edit Filter Output
  virtual void draw(OutputNumber i,
		    vtkImplicitFunction *brush,
		    double bounds[6],
		    EspinaVolume::PixelType value = SEG_VOXEL_VALUE);
  virtual void draw(OutputNumber i,
		    EspinaVolume::IndexType index,
		    EspinaVolume::PixelType value = SEG_VOXEL_VALUE);
  virtual void draw(OutputNumber i,
		    Nm x, Nm y, Nm z,
		    EspinaVolume::PixelType value = SEG_VOXEL_VALUE);

  /// Returns whether or not the filter was edited by the user
  bool isEdited() const;
  /// Returns a list of modified outputs
  QList<OutputNumber> editedOutputs() const;
  /// Specify how many outputs this filter generates
  virtual int numberOutputs() const;
  /// Return the i-th output
  virtual EspinaVolume *output(OutputNumber i) const;
  virtual void markAsModified(){}
  /// Determine whether the filter needs to be updated or not
  virtual bool needUpdate() const {return true;}
  /// Updates filter outputs.
  /// If a snapshot exits it will try to load it from disk
  void update();
  /// Turn on internal filters' release data flags
  virtual void releaseDataFlagOn(){}
  /// Turn off internal filters' release data flags
  virtual void releaseDataFlagOff(){}

  /// Return a widget used to configure filter's parameters
  virtual QWidget *createConfigurationWidget();

protected:
  explicit Filter(NamedInputs namedInputs,
                  Arguments args);

  /// Method which actually executes the filter
  virtual void run() {};
  /// Try to locate an snapshot of the filter in the hard drive
  virtual bool prefetchFilter();

  EspinaVolumeReader::Pointer tmpFileReader(const QString file);

  EspinaVolume::Pointer addRegionToVolume(EspinaVolume::Pointer volume,
					  EspinaVolume::RegionType region);

protected:
  QList<EspinaVolume *> m_inputs;
  NamedInputs           m_namedInputs;
  Arguments             m_args;

  QStringList        m_editedOutputs;
  QMap<OutputNumber, EspinaVolume::Pointer> m_outputs;

private:
  static unsigned int m_lastId;
};

#endif // FILTER_H
