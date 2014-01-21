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
#include "FetchBehaviour.h"
#include "Core/MultiTasking/Task.h"
#include <Core/IO/ErrorHandler.h>

#include <QDir>

namespace EspINA
{
  class Analysis;
  using AnalysisPtr = Analysis *;

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

    void setAnalysis(AnalysisPtr analysis)
    { m_analysis = analysis; }

    AnalysisPtr analysis() const
    { return m_analysis; }

    virtual Snapshot snapshot() const;

    virtual void unload();

    Type type() { return m_type; }

    void setInputs(OutputSList inputs)
    { m_inputs = inputs;}

    OutputSList inputs() const
    { return m_inputs; }

    void setFetchBehaviour(FetchBehaviourSPtr behaviour)
    { m_fetchBehaviour = behaviour; }

    void setErrorHandler(ErrorHandlerSPtr handler)
    { m_handler = handler; }

    ErrorHandlerSPtr handler() const
    { return m_handler; }

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
    { update(); }

    /** \brief Method which actually executes the filter to generate all its outputs
     *
     */
    virtual void execute() = 0;

    /** \brief Method which actually executes the filter to generate output oId
     *
     */
    virtual void execute(Output::Id id) = 0;

    /** \brief Determine whether or not data at persistent storage is still valid
     *
     *  Due to lazy execution some filters may change their state (i.e. parameters)
     *  before actually accessing triggering an update. Fetching storage data in such
     *  cases may lead to output inconsistencies
     */
    virtual bool ignoreStorageContent() const = 0;

    virtual bool invalidateEditedRegions() = 0;

  private:
    QString prefix() const
    { return "Filters/" + uuid().toString() + "/"; }

    QString outputFile() const
    { return prefix() + "outputs.xml"; }

  protected:
    AnalysisPtr m_analysis;
    Type        m_type;
    OutputSList m_inputs;
    OutputSList m_outputs;

    FetchBehaviourSPtr m_fetchBehaviour;
    ErrorHandlerSPtr   m_handler;
  };
} // namespace EspINA

#endif // ESPINA_FILTER_H
