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

#include <QDir>

class QUndoStack;
class vtkImplicitFunction;
class vtkPolyData;

namespace EspINA
{
  class ViewManager;


  typedef QSharedPointer<Filter> FilterSPtr;
  typedef QList<FilterSPtr>      FilterSList;

  class Filter
  : public ModelItem
  {
  public:
    static const QString CREATELINK;

    typedef int OutputId;

    typedef QMap<QString, FilterSPtr> NamedInputs;

    static const QString NamedInput(const QString &label, OutputId oId)
    { return QString("%1_%2").arg(label).arg(oId); }

    struct Output
    {
      static const int INVALID_OUTPUT_ID;

      explicit Output(Filter               *filter = NULL,
                      const OutputId       &id     = INVALID_OUTPUT_ID,
                      EspinaVolume::Pointer volume = EspinaVolume::Pointer()
      )
      : id(id)
      , isCached(false)
      , filter(filter)
      , volume(volume)
      { if (isValid()) this->volume->toITK()->DisconnectPipeline();}

      QList<EspinaRegion>   editedRegions;
      OutputId              id;
      bool                  isCached; /// Whether output is used by a segmentation
      FilterPtr             filter;
      EspinaVolume::Pointer volume;

      bool isValid() const
      {
        return NULL != filter
        && INVALID_OUTPUT_ID < id
        && volume.get()
        && volume->toITK().IsNotNull();
      }

      void addEditedRegion(const EspinaRegion &region);

      /// Whether output has been manually edited
      bool isEdited() const
      {return !editedRegions.isEmpty();} 
    };

    typedef QList<Output> OutputList;

    typedef QString FilterType;

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

  protected:
    typedef itk::ImageFileReader<itkVolumeType> EspinaVolumeReader;

  public:
    virtual ~Filter();

    void setCacheDir(QDir dir);
    QDir cacheDir() const { return m_cacheDir; }

    void setId(int id) {m_args[ID] = QString::number(id);}
    QString id() const {return m_args[ID];}

    // Implements Model Item Interface common to filters
    virtual QVariant data(int role = Qt::DisplayRole) const;
    virtual QString serialize() const;
    virtual ModelItemType type() const {return FILTER;}

    virtual void initialize(const Arguments &args = Arguments()){};
    virtual void initializeExtensions(const Arguments &args = Arguments()){};

    FilterType filterType() { return m_type; }

    struct Link
    {
      FilterPtr filter;
      OutputId  outputPort;
    };

    ///NOTE: Current implementation will expand the image
    ///      when drawing with value != 0

    /// Manually Edit Filter Output
    virtual void draw(OutputId oId,
                      vtkImplicitFunction *brush,
                      const Nm bounds[6],
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);
    virtual void draw(OutputId oId,
                      itkVolumeType::IndexType index,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);
    virtual void draw(OutputId oId,
                      Nm x, Nm y, Nm z,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);
    virtual void draw(OutputId oId,
                      vtkPolyData *contour,
                      Nm slice,
                      PlaneType plane,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);
    virtual void draw(OutputId oId,
                      itkVolumeType::Pointer volume,
                      bool emitSignal = true);
    virtual void fill(OutputId oId,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);
    virtual void fill(OutputId oId,
                      const EspinaRegion &region,
                      itkVolumeType::PixelType value = SEG_VOXEL_VALUE,
                      bool emitSignal = true);

    //NOTE 2012-11-20 suggest a better name
    virtual void restoreOutput(OutputId oId,
                               itkVolumeType::Pointer volume);

    /// Returns filter's outputs
    OutputList outputs() const {return m_outputs.values();}
    /// Returns a list of outputs edited by the user
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

    /// Some filters may need to stablish connections with other items on the model
    /// in order to keep updated
    virtual void upkeeping() {}

    void setFilterInspector(FilterInspectorPtr filterInspector)
    { m_filterInspector = filterInspector; }
    /// Return a widget used to configure filter's parameters
    FilterInspectorPtr const filterInspector() { return m_filterInspector; }

    void resetCacheFlags();

    /// Try to locate an snapshot of the filter in tmpDir
    /// Returns true if all volume snapshot can be recovered
    /// and false otherwise
    virtual bool fetchSnapshot();
    /// QMap<file name, file byte array> of filter's data to save to seg file
    virtual bool dumpSnapshot(Snapshot &snapshot);

    /// returns if the filter has been executed at least once in the session
    virtual bool executed() { return m_executed; }

  protected:
    explicit Filter(NamedInputs namedInputs,
                    Arguments   args,
                    FilterType  type);

    /// Subclasses need to specify which subtype of EspinaVolume they use
    virtual void createOutput(OutputId id, itkVolumeType::Pointer volume = NULL) = 0;
    virtual void createOutput(OutputId id, EspinaVolume::Pointer volume) = 0;
    virtual void createOutput(OutputId id, const EspinaRegion &region, itkVolumeType::SpacingType spacing) = 0;
    /// Method which actually executes the filter
    virtual void run() {};

    /// Reader to access snapshots
    EspinaVolumeReader::Pointer tmpFileReader(const QString file);

  protected:
    QList<EspinaVolume::Pointer> m_inputs;
    NamedInputs                  m_namedInputs;
    mutable Arguments            m_args;
    FilterType                   m_type;

    QMap<OutputId, Output>       m_outputs;
    int  m_cacheId;
    QDir m_cacheDir;
    bool m_traceable;
    bool m_executed;

  private:
    FilterInspectorPtr m_filterInspector;
  };

  class ChannelFilter
  : public Filter
  {
  public:
    virtual ~ChannelFilter(){}

  protected:
    explicit ChannelFilter(NamedInputs namedInputs,
                           Arguments   args,
                           FilterType  type)
    : Filter(namedInputs, args, type){}

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
    explicit SegmentationFilter(NamedInputs namedInputs,
                                Arguments   args,
                                FilterType  type)
    : Filter(namedInputs, args, type){}

    virtual void createOutput(OutputId id, itkVolumeType::Pointer volume = NULL);
    virtual void createOutput(OutputId id, EspinaVolume::Pointer volume);
    virtual void createOutput(OutputId id, const EspinaRegion &region, itkVolumeType::SpacingType spacing);
  };

  FilterPtr filterPtr(ModelItemPtr item);
  FilterSPtr filterPtr(ModelItemSPtr &item);

} // namespace EspINA

#endif // FILTER_H
