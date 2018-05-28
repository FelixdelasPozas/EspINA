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

// ESPINA
#include <Core/IO/ErrorHandler.h>
#include <Core/Analysis/Data/Volumetric/RawVolume.hxx>
#include <Core/Analysis/Data/Volumetric/StreamedVolume.hxx>
#include <Core/Analysis/Data/VolumetricDataUtils.hxx>
#include <Core/Analysis/Filters/VolumetricStreamReader.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/StatePair.h>
#include <Core/Utils/ITKProgressReporter.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;

const QString STREAM_FILENAME = "streamingData.mhd";

//----------------------------------------------------------------------------
VolumetricStreamReader::VolumetricStreamReader(InputSList inputs, Type type, SchedulerSPtr scheduler)
: Filter            {inputs, type, scheduler}
, m_streaming       {false}
, m_streamingStorage{nullptr}
, m_changedStreaming{false}
{
}

//----------------------------------------------------------------------------
void VolumetricStreamReader::restoreState(const State& state)
{
  for(auto element : state.split(";"))
  {
    auto tokens = element.split("=");
    if ("File" == tokens[0])
    {
      auto filename = tokens[1].simplified();
      auto file = QFileInfo(filename);

      if(handler() && (handler()->defaultDir() != QDir()))
      {
        m_fileName = QFileInfo(handler()->defaultDir().filePath(file.fileName()));
      }
      else
      {
        m_fileName = QFileInfo(file.fileName());
      }
    }

    if("Streaming" == tokens[0])
    {
      m_streaming = (tokens[1].simplified() == "true");
    }
  }
}

//----------------------------------------------------------------------------
State VolumetricStreamReader::state() const
{
  State state;

  if(!m_fileName.exists() || !m_fileName.isReadable())
  {
    auto message = QObject::tr("Can't find image file or can't read it. File name: %1").arg(m_fileName.fileName());
    auto details = QObject::tr("VolumetricStreamReader::execute() -> ") + message;

    throw EspinaException(message, details);
  }

  state += StatePair("File", m_fileName.absoluteFilePath());
  state += StatePair("Streaming", (m_streaming ? "true" : "false"));

  return state;
}

//----------------------------------------------------------------------------
void VolumetricStreamReader::setFileName(const QFileInfo& fileName)
{
  m_fileName = fileName;
}

//----------------------------------------------------------------------------
void VolumetricStreamReader::setStreaming(bool value)
{
  if(value != m_streaming)
  {
    m_streaming = value;
    m_changedStreaming = true;
  }
}

//----------------------------------------------------------------------------
bool VolumetricStreamReader::needUpdate() const
{
  return m_outputs.isEmpty() || !validOutput(0) || m_changedStreaming;
}

//----------------------------------------------------------------------------
void VolumetricStreamReader::execute()
{
  m_changedStreaming = false;

  if (!m_fileName.exists() || !m_fileName.isReadable())
  {
    auto stackFilename = m_fileName;

    if (handler())
    {
      m_fileName = m_handler->fileNotFound(m_fileName, QDir(), IO::ErrorHandler::SameFormat(m_fileName));
    }

    if (!m_fileName.exists() || !m_fileName.isReadable())
    {
      auto message = QObject::tr("Can't find image file or can't read it. File name: %1").arg(stackFilename.fileName());
      auto details = QObject::tr("VolumetricStreamReader::execute() -> ") + message;

      throw EspinaException(message, details);
    }
  }

  DataSPtr volume              = nullptr;
  itkVolumeType::Pointer image = nullptr;

  if(!canExecute()) return;

  if(m_streaming)
  {
    if(!m_fileName.fileName().endsWith(".mhd", Qt::CaseInsensitive))
    {
      // needs conversion and temporal storage, tiff is not an streamable format: http://www.awaresystems.be/imaging/tiff/faq.html#q7
      // we can't stream tiff files because trying to do so will kill the application due to excessive memory use by libtiff.
      if(!m_streamingStorage)
      {
        m_streamingStorage = std::make_shared<TemporalStorage>(&storage()->rootDirectory());

        bool needRead = false;

        if(m_outputs[0]->hasData(VolumetricData<itkVolumeType>::TYPE))
        {
          // take advantage of whats in memory before deleting it.
          image = readLockVolume(m_outputs[0])->itkImage();
        }
        else
        {
          needRead = true;

          auto fileName = m_fileName.absoluteFilePath().toUtf8();
          fileName.detach();
          auto filename = fileName.constData();

          try
          {
            image = readVolumeWithProgress<itkVolumeType>(filename, this, 0, 50);
          }
          catch(const itk::ExceptionObject &e)
          {
            auto what    = QObject::tr("Error in itk, exception message: %1. Filename: %2").arg(QString(e.what())).arg(m_fileName.absoluteFilePath());
            auto details = QObject::tr("VolumetricStreamReader::execute() -> Error in itk, exception message: %1").arg(QString(e.what()));

            throw EspinaException(what, details);
          }
        }

        if(!canExecute()) return;

        try
        {
          exportVolumeWithProgress<itkVolumeType>(image, m_streamingStorage->absoluteFilePath(STREAM_FILENAME), this, (needRead ? 50 : 0), 100);
        }
        catch(const itk::ExceptionObject &e)
        {
          auto what    = QObject::tr("Error in itk, exception message: %1. Filename: %2").arg(QString(e.what())).arg(m_fileName.absoluteFilePath());
          auto details = QObject::tr("VolumetricStreamReader::execute() -> Error in itk, exception message: %1").arg(QString(e.what()));

          throw EspinaException(what, details);
        }

        if(!canExecute()) return;

        m_streamingFile = QFileInfo(m_streamingStorage->absoluteFilePath(STREAM_FILENAME));
      }
    }
    else
    {
      // mhd files are streamable
      m_streamingFile = m_fileName;
    }

    volume = std::make_shared<StreamedVolume<itkVolumeType>>(m_streamingFile);
  }
  else
  {
    if(!canExecute()) return;

    try
    {
      image = readVolumeWithProgress<itkVolumeType>(m_fileName.absoluteFilePath(), this);
      if(!canExecute()) return;
    }
    catch(const itk::ExceptionObject &e)
    {
      auto what    = QObject::tr("Error in itk, exception message: %1. Filename: %2").arg(QString(e.what())).arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("VolumetricStreamReader::execute() -> Error in itk, exception message: %1").arg(QString(e.what()));

      throw EspinaException(what, details);
    }

    volume  = std::make_shared<RawVolume<itkVolumeType>>(image);
  }

  auto spacing = volume->bounds().spacing();

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, spacing);
  }

  m_outputs[0]->setData(volume);

  if (m_outputs[0]->spacing() == NmVector3())
  {
    m_outputs[0]->setSpacing(spacing);
  }
  else
  {
    volume->setSpacing(m_outputs[0]->spacing());
  }

  reportProgress(100);
}
