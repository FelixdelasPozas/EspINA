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
  typedef itk::ImageFileReader<EspinaVolume> EspinaVolumeReader;

public:
  typedef int OutputNumber;

  typedef QMap<QString, Filter *> NamedInputs;
  static const QString NamedInput(const QString &label, OutputNumber i)
  { return QString("%1_%2").arg(label).arg(i); }

  struct FilterOutput
  {
    static const int INVALID_OUTPUT_NUMBER = -1;

    bool                  isCached;
    bool                  isEdited;
    Filter               *filter;
    OutputNumber          number;
    EspinaVolume::Pointer volume;

    bool isValid() const {return NULL != filter && INVALID_OUTPUT_NUMBER != number && NULL != volume.GetPointer(); }

    explicit FilterOutput(Filter *f=NULL, OutputNumber n=INVALID_OUTPUT_NUMBER, EspinaVolume::Pointer v =EspinaVolume::Pointer())
    : isCached(false), isEdited(false), filter(f), number(n), volume(v)
    {}
  };

  typedef QList<FilterOutput> OutputList;

  static const ModelItem::ArgumentId ID;
  static const ModelItem::ArgumentId INPUTS;
  static const ModelItem::ArgumentId EDIT;
  static const ModelItem::ArgumentId CACHED;

public:
  virtual ~Filter(){}

  void setTmpDir(QDir dir) {m_tmpDir = dir;}

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
    OutputNumber outputPort;
  };

  ///NOTE: Current implementation will expand the image
  ///      when drawing with value != 0

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
  virtual void draw(OutputNumber i,
                    vtkPolyData *contour,
                    Nm slice,
                    PlaneType plane,
                    EspinaVolume::PixelType value = SEG_VOXEL_VALUE);
  virtual void draw(OutputNumber i,
                    EspinaVolume::Pointer volume);

  //TODO 2012-11-20 cambiar nombre y usar FilterOutput
  virtual void restoreOutput(OutputNumber i,
                           EspinaVolume::Pointer volume);

  /// Returns filter's outputs
  OutputList outputs() const {return m_outputs;}
  /// Returns a list of outputs edited by the user //NOTE: Deberia ser private?
  OutputList editedOutputs() const;
  /// Return whether or not i is an output of the filter
  bool validOutput(OutputNumber i);
  /// Return an output with id i. Ids are not necessarily sequential
  virtual FilterOutput output(OutputNumber i) const;
  FilterOutput &output(OutputNumber i);
  /// Convencience method to get the volume associated wit output i
  EspinaVolume *volume(OutputNumber i) {return output(i).volume;}
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

  /// Return a widget used to configure filter's parameters
  virtual QWidget *createFilterInspector(QUndoStack *undoStack, ViewManager *vm);

  void updateCacheFlags();

protected:
  explicit Filter(NamedInputs namedInputs,
                  Arguments args);

  /// Method which actually executes the filter
  virtual void run() {};
  /// Try to locate an snapshot of the filter in tmpDir
  /// Returns true if all volume snapshot can be recovered
  /// and false otherwise
  virtual bool prefetchFilter();

  /// Reader to access snapshots
  EspinaVolumeReader::Pointer tmpFileReader(const QString file);

  /// Expands @volume to contain @region. If @volume doesn't need to
  /// be expanded it returns volume itself, otherwhise a pointer to a
  /// new volume is returned
  EspinaVolume::Pointer expandVolume(EspinaVolume::Pointer volume,
                                     EspinaVolume::RegionType region);
  /// Update output isEdited flag and filter EDIT argument
  void markAsEdited(OutputNumber i);

protected:
  QList<EspinaVolume *> m_inputs;
  NamedInputs           m_namedInputs;
  mutable Arguments     m_args;

  OutputList m_outputs;

private:
  QDir m_tmpDir;
};

#endif // FILTER_H
