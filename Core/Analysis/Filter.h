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


#ifndef ESPINA_FILTER_H
#define ESPINA_FILTER_H

#include "EspinaCore_Export.h"

#include "Core/EspinaTypes.h"
#include "Core/Analysis/Output.h"
#include "Core/Analysis/Persistent.h"
#include "Core/Analysis/AnalysisItem.h"
#include "Core/MultiTasking/Task.h"

#include <QDir>

namespace EspINA
{
  class EspinaCore_EXPORT Filter
  : public AnalysisItem
  , public Persistent
  , public Task
  {
  public:
    using Type = QString;

//   protected:
//     typedef itk::ImageFileReader<itkVolumeType> EspinaVolumeReader;
  public:
    virtual ~Filter();

    virtual void restoreState(const State& state);

    virtual std::ostream saveState() const;

    virtual void saveSnapshot(StorageSPtr storage) const;

    Type type() { return m_type; }

    /** \brief Update all filter outputs
     *
     *  If filter inputs are outdated a pull request will be done
     */
    bool update();

    bool update(Output::Id id);

    unsigned int numberOfOutputs() const;

    OutputIdList availableOutputIds() const;

    OutputSList outputs() const {return m_outputs.values();}

    /** \brief Return whether or not i is a valid output for the filter
     *
     */
    bool validOutput(Output::Id id);

    /** \brief Return filter's output 
     * 
     *   If there is no output with given oId, nullptr will be returned
     *
     */
    virtual OutputSPtr output(Output::Id id) const = 0;

    // TODO 2013-11-17: Move to utils or sth similar
//     /// Reader to access snapshot data in filter's cache dir
//     itkVolumeType::Pointer readVolumeFromCache(const QString &file);

  protected:
    explicit Filter(OutputSList inputs, Type  type, SchedulerSPtr scheduler);

    virtual void loadFilterCache(const QDir& dir) = 0;

    virtual void saveFilterCache(const Persistent::Id id) const = 0;

    /** \brief Return true if a filter must be executed to update its outputs
     */
    virtual bool needUpdate() const = 0;

    /** \brief Return true if a filter must be executed to update the specified ouput
     *
     */
    virtual bool needUpdate(Output::Id id) const = 0;

    /** \brief Try to load from cache dir all the output data
     *
     *  Returns true if all data snapshot can be recovered
     *  and false otherwise
     */
    bool fetchOutputData(Output::Id id);

    void createOutput(Output::Id id);

    virtual DataSPtr createDataProxy(Output::Id id, const Data::Type &type) = 0;

    virtual void addData(Output::Id id, DataSPtr  data);
    virtual void addData(Output::Id id, DataSList dataList);

    virtual void run() { update(); }

    /** \brief Method which actually executes the filter to generate all its outputs
     *
     */
    virtual void execute() = 0;

    /// Method which actually executes the filter to generate output oId
    virtual void execute(Output::Id id) = 0;

  protected:
    using OutputMap = QMap<Output::Id, OutputSPtr>;

    Type        m_type;
    OutputSList m_inputs;
    OutputMap   m_outputs;
  };
} // namespace EspINA

#endif // ESPINA_FILTER_H