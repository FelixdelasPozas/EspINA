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

#ifndef ESPINA_ITK_PROGRESS_REPORTER_H
#define ESPINA_ITK_PROGRESS_REPORTER_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include "Core/Analysis/Filter.h"

// ITK
#include <itkCommand.h>

namespace ESPINA
{
	/** \class ITKProgressReporter
	 * \brief Encapsulates ITK filters progress report so it will be reported by the task class.
	 *
	 */
  template<typename T>
  class EspinaFilters_EXPORT ITKProgressReporter
  {
  public:
  	/** \brief ITKProgressReporter class constructor.
  	 * \param[in] reporter, raw pointer of the task that will report progress.
  	 * \param[in] filter, smart pointer of itk filter contained in task.
  	 * \param[in] fromValue, initial progress value.
  	 * \param[in] toValue, end progress value.
  	 *
  	 */
    ITKProgressReporter(Task* reporter, typename T::Pointer filter, int fromValue=0, int toValue = 100)
    : m_task(reporter)
    , m_filter(filter)
    , m_initialProgress(fromValue)
    , m_delta(toValue - fromValue)
    {
      auto progressCommand = itk::SimpleMemberCommand<ITKProgressReporter<T>>::New();
      progressCommand->SetCallbackFunction(this, &ITKProgressReporter<T>::reportProgress);

      m_filter->AddObserver(itk::ProgressEvent(), progressCommand);
    }

  private:
    /** \brief Makes the task report the progress of the itk filter.
     *
     */
    void reportProgress()
    {
      m_task->reportProgress(m_initialProgress + m_delta*m_filter->GetProgress());
    }

  private:
    Task*               m_task;
    typename T::Pointer m_filter;
    int                 m_initialProgress;
    int                 m_delta;
  };
} // namespace ESPINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_FILTER_H
