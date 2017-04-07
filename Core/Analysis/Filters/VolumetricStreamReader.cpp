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
#include <Core/Analysis/Filters/VolumetricStreamReader.h>
#include <Core/Utils/EspinaException.h>
#include <Core/Utils/ITKProgressReporter.h>

// ITK
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

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
      m_fileName = QFileInfo(tokens[1].trimmed());
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

  state += QString("File=%1;").arg(m_fileName.absoluteFilePath());
  state += QString("Streaming=%1;").arg(m_streaming ? "true" : "false");

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

  DataSPtr volume = nullptr;

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

        itkVolumeType::Pointer image = nullptr;
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

          auto imageIO = itk::ImageIOFactory::CreateImageIO(filename, itk::ImageIOFactory::ReadMode);
          imageIO->SetGlobalWarningDisplay(false);
          imageIO->SetFileName(filename);
          imageIO->ReadImageInformation();

          if((imageIO->GetPixelType() != itk::ImageIOBase::IOPixelType::SCALAR) || (imageIO->GetComponentSize() != 1))
          {
            auto message = QObject::tr("Can't read image file, file name: %1. Pixel type is not 8-bits.").arg(m_fileName.absoluteFilePath());
            auto details = QObject::tr("VolumetricStreamReader::execute() -> ") + message;

            throw EspinaException(message, details);
          }

          if(!canExecute()) return;

          auto reader  = itk::ImageFileReader<itkVolumeType>::New();
          reader->SetGlobalWarningDisplay(false);
          reader->UseStreamingOff();
          reader->SetNumberOfThreads(1);
          reader->SetFileName(filename);

          ITKProgressReporter<itk::ImageFileReader<itkVolumeType>> readReporter(this, reader, 0, 50);

          reader->Update();

          image = reader->GetOutput();
          image->DisconnectPipeline();
        }

        if(!canExecute()) return;

        auto writer = itk::ImageFileWriter<itkVolumeType>::New();
        writer->SetGlobalWarningDisplay(false);
        writer->ReleaseDataFlagOn();
        writer->SetNumberOfThreads(1);
        writer->UseCompressionOff();
        writer->UseInputMetaDataDictionaryOff();
        writer->SetNumberOfStreamDivisions(1);
        writer->SetInput(image);
        writer->SetFileName(m_streamingStorage->absoluteFilePath(STREAM_FILENAME).toStdString());

        ITKProgressReporter<itk::ImageFileWriter<itkVolumeType>> writeReporter(this, writer, (needRead ? 50 : 0), 100);

        writer->Write();

        if(!canExecute()) return;

        m_streamingFile = QFileInfo(m_streamingStorage->absoluteFilePath(STREAM_FILENAME));
      }
    }
    else
    {
      // mhd files are streamable, hurayy
      m_streamingFile = m_fileName;
    }

    volume = std::make_shared<StreamedVolume<itkVolumeType>>(m_streamingFile);
  }
  else
  {
    if(!canExecute()) return;

    auto fileName = m_fileName.absoluteFilePath().toUtf8();
    fileName.detach();
    auto filename = fileName.constData();

    auto imageIO = itk::ImageIOFactory::CreateImageIO(filename, itk::ImageIOFactory::ReadMode);
    imageIO->SetGlobalWarningDisplay(false);
    imageIO->SetFileName(filename);
    imageIO->ReadImageInformation();

    if((imageIO->GetPixelType() != itk::ImageIOBase::IOPixelType::SCALAR) || (imageIO->GetComponentSize() != 1))
    {
      auto message = QObject::tr("Can't read image file, file name: %1. Pixel type is not 8-bits.").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("VolumetricStreamReader::execute() -> ") + message;

      throw EspinaException(message, details);
    }

    auto reader  = itk::ImageFileReader<itkVolumeType>::New();
    reader->SetGlobalWarningDisplay(false);
    reader->SetFileName(filename);
    reader->SetUseStreaming(false);
    reader->SetNumberOfThreads(1);
    try
    {
      reader->Update();
      if(!canExecute()) return;
    }
    catch(const itk::ExceptionObject &e)
    {
      auto what    = QObject::tr("Error in itk, exception message: %1").arg(QString(e.what()));
      auto details = QObject::tr("VolumetricStreamReader::execute() -> Error in itk, exception message: %1").arg(QString(e.what()));

      throw EspinaException(what, details);
    }

    volume  = std::make_shared<RawVolume<itkVolumeType>>(reader->GetOutput());
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
}
