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
#include <Core/EspinaRegion.h>
#include <Core/Model/Output.h>

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

  typedef QList<ChannelRepresentationSPtr>      ChannelRepresentationSList;
  typedef QList<SegmentationRepresentationSPtr> SegmentationRepresentationSList;

  class Filter
  : public ModelItem
  {
  public:
    static const QString CREATELINK;

    typedef QMap<QString, FilterSPtr> NamedInputs;

    static const QString NamedInput(const QString &label, FilterOutputId oId)
    { return QString("%1_%2").arg(label).arg(oId); }


    typedef QString FilterType;

    class FilterInspector
    {
    public:
      virtual ~FilterInspector(){}

      virtual QWidget *createWidget(QUndoStack *stack, ViewManager *viewManager) = 0;
    };

    typedef boost::shared_ptr<FilterInspector> FilterInspectorPtr;

    struct Link
    {
      FilterPtr filter;
      FilterOutputId  outputPort;
    };

    static const ModelItem::ArgumentId ID;
    static const ModelItem::ArgumentId INPUTS;
    static const ModelItem::ArgumentId EDIT;

  protected:
    typedef itk::ImageFileReader<itkVolumeType> EspinaVolumeReader;

  public:
    virtual ~Filter();

    void setTraceable(bool traceable)
    { m_traceable = traceable; }
    bool isTraceable() const
    { return m_traceable; }

    virtual void setCacheDir(QDir dir);
    QDir cacheDir() const { return m_cacheDir; }

    void setId(int id) {m_args[ID] = QString::number(id);}
    QString id() const {return m_args[ID];}

    // Implements Model Item Interface common to filters
    virtual QVariant data(int role = Qt::DisplayRole) const;
    virtual QString serialize() const;
    virtual ModelItemType type() const {return FILTER;}

    virtual void initialize(const Arguments &args = Arguments()){};
    //virtual void initializeExtensions(const Arguments &args = Arguments()){};

    FilterType filterType() { return m_type; }

    /// Return filter's output or NULL pointer if there is no output with given oId
    virtual OutputSPtr output(FilterOutputId oId) const = 0;

    /// Update all filter outputs
    /// If a snapshot exits, filter will try to load it from disk
    virtual void update() = 0;

    /// Update filter output oId.
    /// If a snapshot exits, filter will try to load it from disk
    virtual void update(FilterOutputId oId) = 0;

    /// Return whether or not i is an output of the filter
    virtual bool validOutput(FilterOutputId oId) = 0;

    /// Some filters may need to stablish connections with other items on the model
    /// in order to keep updated
    virtual void upkeeping() {}

    void setFilterInspector(FilterInspectorPtr filterInspector)
    { m_filterInspector = filterInspector; }

    /// Return a widget used to configure filter's parameters
    /// Filter Inspector are available for traceable filters and those executed
    /// on current session
    FilterInspectorPtr const filterInspector() 
    { return (m_traceable || m_executed)?m_filterInspector:FilterInspectorPtr(); }

    /// Filter's data to save to seg file
    virtual bool dumpSnapshot(Snapshot &snapshot);

    /// returns if the filter has been executed at least once in the session
    virtual bool executed() { return m_executed; }

    /// Reader to access snapshots
    itkVolumeType::Pointer readVolumeFromCache(const QString &file);

    virtual int numberOfOutputs() const = 0;

    virtual QList<FilterOutputId> availableOutputIds() const = 0;

  protected:
    explicit Filter(NamedInputs namedInputs,
                    Arguments   args,
                    FilterType  type);

    virtual void createDummyOutput(FilterOutputId id, const FilterOutput::OutputRepresentationName &type) = 0;

    /// Current outputs must be ignored due to changes on filter state
    virtual bool ignoreCurrentOutputs() const = 0;

    /// Whether the filter needs to be updated or not
    /// A filter will need an update if there exists an invalid output or no outputs exist at all
    virtual bool needUpdate() const = 0;
 
    /// Whether the filter needs to be updated or not
    /// Default implementation will request an update if there is no filter output
    /// or it is an invalid output
    virtual bool needUpdate(FilterOutputId oId) const = 0;

    /// Try to locate an snapshot of the filter in tmpDir
    /// Returns true if all volume snapshot can be recovered
    /// and false otherwise
    virtual bool fetchSnapshot(FilterOutputId oId) = 0;

    /// Method which actually executes the filter to generate all its outputs
    virtual void run() = 0;

    /// Method which actually executes the filter to generate output oId
    virtual void run(FilterOutputId oId) = 0;

  protected:

    OutputSList       m_inputs;
    NamedInputs       m_namedInputs;
    mutable Arguments m_args;
    FilterType        m_type;

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

    /// Returns filter's outputs
    ChannelOutputSList outputs() const {return m_outputs.values();}

    /// Return filter's output or NULL pointer if there is no output with given oId
    virtual OutputSPtr output(FilterOutputId oId) const
    { return m_outputs.value(oId, ChannelOutputSPtr()); }

    virtual void update()
    { Filter::update(); }

    /// Update filter output oId.
    /// If a snapshot exits, filter will try to load it from disk
    virtual void update(FilterOutputId oId);

    virtual bool validOutput(FilterOutputId oId);

    virtual int numberOfOutputs() const { return m_outputs.size(); }
    virtual QList<FilterOutputId> availableOutputIds() const { return m_outputs.keys(); }

  protected:
    explicit ChannelFilter(NamedInputs namedInputs,
                           Arguments   args,
                           FilterType  type)
    : Filter(namedInputs, args, type){}

    /// Try to locate an snapshot of the filter in tmpDir
    /// Returns true if all volume snapshot can be recovered
    /// and false otherwise
    virtual bool fetchSnapshot(FilterOutputId oId) {return false;}

    virtual void createOutput(FilterOutputId id, ChannelRepresentationSPtr  data);
    virtual void createOutput(FilterOutputId id, ChannelRepresentationSList repList);

    virtual void createOutputRepresentations(ChannelOutputSPtr output) = 0;

    /// Whether the filter needs to be updated or not
    /// A filter will need an update if there exists an invalid output or no outputs exist at all
    virtual bool needUpdate() const;
 
    /// Whether the filter needs to be updated or not
    /// Default implementation will request an update if there is no filter output
    /// or it is an invalid output
    virtual bool needUpdate(FilterOutputId oId) const = 0;

  protected:
    QMap<FilterOutputId, ChannelOutputSPtr> m_outputs;
  };

  class SegmentationFilter
  : public Filter
  {
  public:
    virtual ~SegmentationFilter(){}

    /// Returns filter's outputs
    SegmentationOutputSList outputs() const {return m_outputs.values();}

    /// Return filter's output or NULL pointer if there is no output with given oId
    virtual OutputSPtr output(FilterOutputId oId) const
    { return m_outputs.value(oId, SegmentationOutputSPtr()); }

    virtual void update()
    { Filter::update(); }

    /// Update filter output oId.
    /// If a snapshot exits, filter will try to load it from disk
    virtual void update(FilterOutputId oId);

    virtual bool validOutput(FilterOutputId oId);

    virtual bool dumpSnapshot(Snapshot &snapshot);

    virtual int numberOfOutputs() const { return m_outputs.size(); }
    virtual QList<FilterOutputId> availableOutputIds() const { return m_outputs.keys(); }

  protected:
    explicit SegmentationFilter(NamedInputs namedInputs,
                                Arguments   args,
                                FilterType  type)
    : Filter(namedInputs, args, type){}

    virtual void setCacheDir(QDir dir);

    /// Try to locate an snapshot of the filter in tmpDir
    /// Returns true if all volume snapshot can be recovered
    /// and false otherwise
    virtual bool fetchSnapshot(FilterOutputId oId);


    virtual void createOutput(FilterOutputId id, SegmentationRepresentationSPtr  data);
    virtual void createOutput(FilterOutputId id, SegmentationRepresentationSList repList);

    virtual void createOutputRepresentations(SegmentationOutputSPtr output) = 0;

    /// Whether the filter needs to be updated or not
    /// A filter will need an update if there exists an invalid output or no outputs exist at all
    virtual bool needUpdate() const;

    /// Whether the filter needs to be updated or not
    /// Default implementation will request an update if there is no filter output
    /// or it is an invalid output
    virtual bool needUpdate(FilterOutputId oId) const = 0;

  protected:
    QMap<FilterOutputId, SegmentationOutputSPtr> m_outputs;
  };

  FilterPtr filterPtr(ModelItemPtr item);
  FilterSPtr filterPtr(ModelItemSPtr &item);

} // namespace EspINA

#endif // FILTER_H
