/*

 Copyright (C) 2016 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <Core/IO/SaveThread.h>
#include <Core/IO/SegFile.h>
#include <Core/Utils/EspinaException.h>

// ITK
#include <itkImage.h>

// C++
#include <exception>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::IO;
using namespace ESPINA::IO::SegFile;

//-----------------------------------------------------------------------------
SaveThread::SaveThread(SchedulerSPtr scheduler, AnalysisPtr analysis, const QFileInfo& file, ProgressReporterSPtr reporter, ErrorHandlerSPtr handler)
: Task      {scheduler}
, m_analysis{analysis}
, m_file    {file}
, m_reporter{reporter}
, m_handler {handler}
, m_success {false}
{
  setDescription(tr("Save session to file %1.").arg(file.fileName()));

  if(reporter)
  {
    connect(reporter.get(), SIGNAL(progress(int)),
            this,           SLOT(reportProgress(int)));
  }
}

//-----------------------------------------------------------------------------
void SaveThread::run()
{
  try
  {
    save(m_analysis, m_file, m_reporter.get(), m_handler);
    m_success = true;
  }
  catch(EspinaException &e)
  {
    m_success = false;
    m_errorMessage = QString{e.what()};
  }
  catch(itk::ExceptionObject &e)
  {
    m_success = false;
    m_errorMessage = QString{e.what()};
  }
  catch(std::exception &e)
  {
    m_success = false;
    m_errorMessage = QString{e.what()};
  }
  catch(...)
  {
    m_success = false;
    m_errorMessage = tr("Unspecified error.");
  }
}
