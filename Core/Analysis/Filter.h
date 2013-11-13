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

#include "Filters/EspinaFilters_Export.h"

#include "Core/EspinaTypes.h"
#include "Core/Analysis/Output.h"
#include "Core/Analysis/Persistent.h"
#include "Core/MultiTasking/Task.h"

#include <QDir>

namespace EspINA
{
  class EspinaFilters_EXPORT Filter
  : public Persistent
  , public Task
  {
  public:
    using Type = QString;

    struct Invalid_Number_Of_Inputs_Exception{};
    struct Invalid_Input_Data_Exception{};
    struct Undefined_Output_Exception{};

//   protected:
//     typedef itk::ImageFileReader<itkVolumeType> EspinaVolumeReader;
  public:
    virtual ~Filter();

    virtual Snapshot saveSnapshot() const;

    virtual void unload();

    Type type() { return m_type; }

    /** \brief Update all filter outputs
     *
     *  If filter inputs are outdated a pull request will be done
     */
    bool update();

    bool update(Output::Id id);

    unsigned int numberOfOutputs() const;

    OutputSList outputs() const {return m_outputs;}

    /** \brief Return whether or not i is a valid output for the filter
     *
     */
    bool validOutput(Output::Id id) throw(Undefined_Output_Exception);

    /** \brief Return filter's output 
     * 
     *   If there is no output with given oId, nullptr will be returned
     *
     */
    OutputSPtr output(Output::Id id) const throw(Undefined_Output_Exception);

    // TODO 2013-11-17: Move to utils or sth similar
//     /// Reader to access snapshot data in filter's cache dir
//     itkVolumeType::Pointer readVolumeFromCache(const QString &file);

  protected:
    explicit Filter(OutputSList inputs, Type  type, SchedulerSPtr scheduler);

    virtual Snapshot saveFilterSnapshot() const = 0;

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

    //virtual DataSPtr createDataProxy(Output::Id id, const Data::Type &type) = 0;

    virtual void run() 
    {
      update();
      emit finished();
    }

    /** \brief Method which actually executes the filter to generate all its outputs
     *
     */
    virtual void execute() = 0;

    /// Method which actually executes the filter to generate output oId
    virtual void execute(Output::Id id) = 0;

    virtual bool invalidateEditedRegions() = 0;

  protected:
    Type        m_type;
    OutputSList m_inputs;
    OutputSList m_outputs;
  };
} // namespace EspINA

#endif // ESPINA_FILTER_H
