/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Core/EspinaTypes.h"
#include "Core/Analysis/Input.h"
#include "Core/Analysis/Persistent.h"
#include "FetchBehaviour.h"
#include "Core/MultiTasking/Task.h"
#include <Core/IO/ErrorHandler.h>
#include <Core/IO/SegFile_V4.h>

// Qt
#include <QDir>

namespace ESPINA
{
  class Analysis;
  using AnalysisPtr = Analysis *;

  class EspinaCore_EXPORT Filter
  : public Persistent
  , public Task
  {
  public:
    using Type = QString;

    struct Invalid_Number_Of_Inputs_Exception{};
    struct Invalid_Input_Data_Exception{};
    struct Undefined_Output_Exception{};

  private:
    using OutputSMap = QMap<Output::Id, OutputSPtr>;

  public:
    /** \brief Filter class destructor.
     *
     */
    virtual ~Filter();

    /** \brief Sets the analysis this filters belongs to.
     *
     */
    void setAnalysis(AnalysisPtr analysis)
    { m_analysis = analysis; }

    /** \brief Returns the analysis that contains this filter.
     *
     */
    AnalysisPtr analysis() const
    { return m_analysis; }

    virtual Snapshot snapshot() const final;

    virtual void unload();

    /** \brief Returns the type of the filter.
     *
     */
    Type type()
    { return m_type; }

    /** \brief Sets the inputs of the filter.
     * \param[in] inputs list of input smart pointers.
     *
     */
    void setInputs(InputSList inputs)
    { m_inputs = inputs;}

    /** \brief Returns the inputs of the filter.
     *
     */
    InputSList inputs() const
    { return m_inputs; }

    /** \brief Sets the fetch data behaviour of the filter.
     * \param[in] behaviour fetch behaviour object smart pointer.
     *
     */
    void setFetchBehaviour(FetchBehaviourSPtr behaviour)
    { m_fetchBehaviour = behaviour; }

    /** \brief Sets the error handler of the filter.
     * \param[in] handler error handler smart pointer.
     *
     */
    void setErrorHandler(ErrorHandlerSPtr handler)
    { m_handler = handler; }

    /** \brief Returns the error handler for this filter.
     *
     */
    ErrorHandlerSPtr handler() const
    { return m_handler; }

    /** \brief Update all filter outputs
     *
     *  If filter inputs are outdated a pull request will be done
     */
    bool update();

    /** \brief Update filter output with the specified id.
     *
     */
    bool update(Output::Id id);

    /** \brief Return the number of outputs of the filter.
     *
     */
    unsigned int numberOfOutputs() const;

    /** \brief Returns the list of outputs of the filter.
     *
     */
    OutputSList outputs() const
    {return m_outputs.values();}

    /** \brief Return whether or not id is a valid output for the filter
     * \param[in] id Output::Id object.
     *
     */
    bool validOutput(Output::Id id) const throw(Undefined_Output_Exception);

    /** \brief Return filter's output with the given id.
     * \param[in] id Output::Id object.
     *
     *   If there is no output with given oId, nullptr will be returned
     *
     */
    OutputSPtr output(Output::Id id) const throw(Undefined_Output_Exception);

  protected:
    /** \brief Filter class constructor.
     * \param[in] inputs list of input smart pointers.
     * \param[in] type type of the filter.
     * \param[in] scheduler smart pointer of the system scheduler.
     *
     */
    explicit Filter(InputSList inputs, Type  type, SchedulerSPtr scheduler);

    /** \brief Returns the snapshot of the filter.
     *
     */
    virtual Snapshot saveFilterSnapshot() const = 0;

    /** \brief Return true if a filter must be executed to update its outputs.
     *
     */
    virtual bool needUpdate() const = 0;

    /** \brief Return true if a filter must be executed to update the specified output.
     * \param[in] id Output::Id object.
     *
     */
    virtual bool needUpdate(Output::Id id) const = 0;

    /** \brief Try to load from cache dir all the output data.
     * \param[in] id Output::Id object.
     *
     *  Returns true if all data snapshot can be recovered
     *  and false otherwise
     */
    bool fetchOutputData(Output::Id id);

    /** \brief Executes the filter to generate/update all its outputs.
     *
     */
    virtual void run()
    { update(); }

    /** \brief Method which actually executes the filter to generate all its outputs
     *
     */
    virtual void execute() = 0;

    /** \brief Method which actually executes the filter to generate output oId.
     * \param[in] id Output::Id object.
     *
     */
    virtual void execute(Output::Id id) = 0;

    /** \brief Determine whether or not data at persistent storage is still valid.
     *
     *  Due to lazy execution some filters may change their state (i.e. parameters)
     *  before actually triggering an update. Fetching storage data in such
     *  cases may lead to output inconsistencies
     */
    virtual bool ignoreStorageContent() const = 0;

  private:
    /** \brief Returns true if the data stored in the persistent storage is valid.
     *
     */
    bool validStoredInformation() const;

    /** \brief Check if output was created during this or previous executions.
     * \param[in] id Output::Id object.
     *
     */
    bool existOutput(Output::Id id) const;

    /** \brief Creates the outputs of the filter using the stored information.
     *
     */
    bool restorePreviousOutputs() const;

  private:
    /** \brief Returns the output file name for the filter.
     *
     */
    QString outputFile() const
    { return prefix() + "outputs.xml"; }

  protected:
    /** \brief Returns the prefix for the data files of this filter.
     *
     */
    QString prefix() const
    { return "Filters/" + uuid().toString() + "/"; }

    AnalysisPtr m_analysis;
    Type        m_type;
    InputSList  m_inputs;

    mutable OutputSMap m_outputs;

    //    bool m_invalidateSortoredOutputs;

    FetchBehaviourSPtr m_fetchBehaviour;
    ErrorHandlerSPtr   m_handler;

    // TODO : Remove with ESPINA 2.1
    friend class IO::SegFile::SegFile_V4;
  };
} // namespace ESPINA

#endif // ESPINA_FILTER_H
