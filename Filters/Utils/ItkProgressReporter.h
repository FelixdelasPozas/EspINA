/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_ITK_PROGRESS_REPORTER_H
#define ESPINA_ITK_PROGRESS_REPORTER_H

// #include "EspinaFilters_Export.h"

#include "Core/Analysis/Filter.h"
#include <itkCommand.h>

namespace EspINA
{
  template<typename T>
  class ITKProgressReporter
  {
  public:
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
    void reportProgress()
    {
      m_task->reportProgress(m_initialProgress + m_delta*m_filter->GetProgress());
    }

  private:
    Task*               m_task;
    typename T::Pointer m_filter;
    int m_initialProgress;
    int m_delta;
  };
} // namespace EspINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_FILTER_H
