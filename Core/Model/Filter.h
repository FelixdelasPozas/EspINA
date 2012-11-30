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

#include "Core/Model/ModelItem.h"

#include "Core/EspinaTypes.h"
#include <Core/EspinaVolume.h>

#include <boost/shared_ptr.hpp>

#include <itkImageFileReader.h>
#include <vtkPolyData.h>

#include <QDir>

class QUndoStack;
class ViewManager;
class vtkImplicitFunction;

const QString CREATELINK = "CreateSegmentation";

class Filter
: public ModelItem
{
protected:
  typedef itk::ImageFileReader<itkVolumeType> EspinaVolumeReader;

public:
  typedef int OutputId;

  typedef QMap<QString, Filter *> NamedInputs;

  static const QString NamedInput(const QString &label, OutputId oId)
  { return QString("%1_%2").arg(label).arg(oId); }

  struct Output
  {
    explicit Output(Filter               *filter = NULL,
                    OutputId              id     = INVALID_OUTPUT_ID,
                    EspinaVolume::Pointer volume = EspinaVolume::Pointer()
                   )
    : isCached(false)
    , isEdited(false)
    , filter(filter)
    , id(id)
    , volume(volume)
    {}

    bool                  isCached;
    bool                  isEdited;
    Filter               *filter;
    OutputId              id;
    EspinaVolume::Pointer volume;

    bool isValid() const
    {
      return NULL != filter
          && INVALID_OUTPUT_ID < id
          && volume.get()
          && volume->toITK().IsNotNull();
    }

    static const int INVALID_OUTPUT_ID = -1;
  };

  typedef QList<Output> OutputList;

  class FilterInspector
  {
  public:
    virtual ~FilterInspector(){}

    virtual QWidget *createWidget(QUndoStack *stack, ViewManager *viewManager) = 0;
  };

  typedef boost::shared_ptr<FilterInspector> FilterInspectorPtr;

  static const ModelItem::ArgumentId ID;
  static const ModelItem::ArgumentId INPUTS;
  static const ModelItem::ArgumentId EDIT;

public:
  virtual ~Filter(){}

  void setTmpDir(QDir dir);

  void setTmpId(int id) {m_args[ID] = QString::number(id);}
  QString tmpId() const {return m_args[ID];}

  // Implements Model Item Interface common to filters
  virtual ItemType type() const {return ModelItem::FILTER;}
  virtual void initialize(Arguments args = Arguments()){};
  virtual void initializeExtensions(Arguments args = Arguments()){};
  virtual QString serialize() const;

  struct Link
  {
    Filter      *filter;
    OutputId outputPort;
  };

  ///NOTE: Current implementation will expand the image
  ///      when drawing with value != 0

  /// Manually Edit Filter Output
  virtual void draw(OutputId oId,
                    vtkImplicitFunction *brush,
                    const Nm bounds[6],
                    itkVolumeType::PixelType value = SEG_VOXEL_VALUE);
  virtual void draw(OutputId oId,
                    itkVolumeType::IndexType index,
                    itkVolumeType::PixelType value = SEG_VOXEL_VALUE);
  virtual void draw(OutputId oId,
                    Nm x, Nm y, Nm z,
                    itkVolumeType::PixelType value = SEG_VOXEL_VALUE);
  virtual void draw(OutputId oId,
                    vtkPolyData *contour,
                    Nm slice,
                    PlaneType plane,
                    itkVolumeType::PixelType value = SEG_VOXEL_VALUE);
  virtual void draw(OutputId oId,
                    itkVolumeType::Pointer volume);

  //TODO 2012-11-20 cambiar nombre y usar FilterOutput
  virtual void restoreOutput(OutputId oId,
                           itkVolumeType::Pointer volume);

  /// Returns filter's outputs
  OutputList outputs() const {return m_outputs.values();}
  /// Returns a list of outputs edited by the user //NOTE: Deberia ser private?
  OutputList editedOutputs() const;
  /// Return whether or not i is an output of the filter
  bool validOutput(OutputId oId);
  /// Return an output with id i. Ids are not necessarily sequential
  virtual const Output output(OutputId oId) const;
  virtual Output &output(OutputId oId);
  /// Convencience method to get the volume associated wit output i
  EspinaVolume::Pointer volume(OutputId oId) {return output(oId).volume;}
  const EspinaVolume::Pointer volume(OutputId oId) const {return output(oId).volume;}
  /// Determine whether the filter needs to be updated or not
  /// Default implementation will request an update if there are no filter outputs
  /// or there is at least one invalid output
  virtual bool needUpdate() const = 0;
  /// Updates filter outputs.
  /// If a snapshot exits it will try to load it from disk
  void update();

  /// Turn on internal filters' release data flags
  virtual void releaseDataFlagOn(){}
  /// Turn off internal filters' release data flags
  virtual void releaseDataFlagOff(){}

  void setFilterInspector(FilterInspectorPtr filterInspector)
  { m_filterInspector = filterInspector; }
  /// Return a widget used to configure filter's parameters
  FilterInspectorPtr const filterInspector() { return m_filterInspector; }

  void updateCacheFlags();

protected:
  explicit Filter(NamedInputs namedInputs,
                  Arguments args);

  /// Subclasses need to specify which subtype of EspinaVolume they use
  virtual void createOutput(OutputId id, itkVolumeType::Pointer volume = NULL) = 0;
  virtual void createOutput(OutputId id, EspinaVolume::Pointer volume) = 0;
  virtual void createOutput(OutputId id, const EspinaRegion &region, itkVolumeType::SpacingType spacing) = 0;
  /// Method which actually executes the filter
  virtual void run() {};
  /// Try to locate an snapshot of the filter in tmpDir
  /// Returns true if all volume snapshot can be recovered
  /// and false otherwise
  virtual bool prefetchFilter();

  /// Reader to access snapshots
  EspinaVolumeReader::Pointer tmpFileReader(const QString file);

  /// Update output isEdited flag and filter EDIT argument
  void markAsEdited(OutputId oId);

protected:
  QList<EspinaVolume::Pointer> m_inputs;
  NamedInputs                  m_namedInputs;
  mutable Arguments            m_args;

  QMap<OutputId, Output> m_outputs;

private:
  FilterInspectorPtr m_filterInspector;
  QDir m_tmpDir;
};

class ChannelFilter
: public Filter
{
public:
  virtual ~ChannelFilter(){}

protected:
  explicit ChannelFilter(NamedInputs namedInputs, Arguments args)
  : Filter(namedInputs, args){}

  virtual void createOutput(OutputId id, itkVolumeType::Pointer volume = NULL);
  virtual void createOutput(OutputId id, EspinaVolume::Pointer volume);
  virtual void createOutput(OutputId id, const EspinaRegion &region, itkVolumeType::SpacingType spacing);
};

class SegmentationFilter
: public Filter
{
public:
  virtual ~SegmentationFilter(){}

protected:
  explicit SegmentationFilter(NamedInputs namedInputs, Arguments args)
  : Filter(namedInputs, args){}

  virtual void createOutput(OutputId id, itkVolumeType::Pointer volume = NULL);
  virtual void createOutput(OutputId id, EspinaVolume::Pointer volume);
  virtual void createOutput(OutputId id, const EspinaRegion &region, itkVolumeType::SpacingType spacing);
};
#endif // FILTER_H
