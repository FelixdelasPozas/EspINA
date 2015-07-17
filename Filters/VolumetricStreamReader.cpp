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
#include "VolumetricStreamReader.h"
#include <Core/IO/ErrorHandler.h>
#include <Core/Analysis/Data/Volumetric/RawVolume.hxx>

// Qt

// ITK
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

using namespace ESPINA;

//----------------------------------------------------------------------------
VolumetricStreamReader::VolumetricStreamReader(InputSList inputs, Type type, SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
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
  }
}

//----------------------------------------------------------------------------
State VolumetricStreamReader::state() const
{
  State state;

  state += QString("File=%1").arg(m_fileName.absoluteFilePath());

  return state;
}

//----------------------------------------------------------------------------
void VolumetricStreamReader::setFileName(const QFileInfo& fileName)
{
  m_fileName = fileName;
}

//----------------------------------------------------------------------------
bool VolumetricStreamReader::needUpdate() const
{
  return m_outputs.isEmpty() || !validOutput(0);
}

//----------------------------------------------------------------------------
void VolumetricStreamReader::execute()
{
  if (!m_fileName.exists())
  {
    if (handler())
    {
      m_fileName = m_handler->fileNotFound(m_fileName, QDir(), IO::ErrorHandler::SameFormat(m_fileName));
    }

    if (!m_fileName.exists())
    {
      throw File_Not_Found_Exception();
    }
  }

  using VolumeReader = itk::ImageFileReader<itkVolumeType>;
  using VolumeWriter = itk::ImageFileWriter<itkVolumeType>;

  QFileInfo mhdFile   = m_fileName;
  QString mhdFileName = m_fileName.baseName() + QString(".mhd");

//   QString fileInAnotherStorage = storage()->findFile(mhdFileName);
//   if(fileInAnotherStorage != QString())
//   {
//     mhdFile = QFileInfo(fileInAnotherStorage);
//   }
//   else
//   {
//     if (mhdFile.fileName().endsWith(".tif"))
//     {
//
//       VolumeReader::Pointer reader = VolumeReader::New();
//       reader->SetFileName(mhdFile.absoluteFilePath().toUtf8().data());
//       reader->Update();
//
//       mhdFile = QFileInfo(storage()->absoluteFilePath(mhdFile.baseName() + ".mhd"));
//
//       VolumeWriter::Pointer writer = VolumeWriter::New();
//       writer->SetFileName(mhdFile.absoluteFilePath().toUtf8().data());
//       writer->SetInput(reader->GetOutput());
//       writer->Write();
//     }
//   }

  VolumeReader::Pointer reader = VolumeReader::New();
  reader->SetFileName(mhdFile.absoluteFilePath().toUtf8().data());
  try
  {
    reader->Update();
  }
  catch(...)
  {
    QString warning = (mhdFile.absoluteFilePath().endsWith("mhd", Qt::CaseInsensitive) ? " Check if raw file inside mhd file exists." : "");

    qWarning() << "exception in itk file reader. File:" << mhdFile.absoluteFilePath() << warning;

    throw File_Not_Found_Exception();
  }

  auto volume = std::make_shared<RawVolume<itkVolumeType>>(reader->GetOutput());

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = std::make_shared<Output>(this, 0, volume->spacing());
  }

  m_outputs[0]->setData(volume);

  if (m_outputs[0]->spacing() == NmVector3())
  {
    m_outputs[0]->setSpacing(volume->spacing());
  }
  else
  {
    volume->setSpacing(m_outputs[0]->spacing());
  }

  qDebug() << "Loading Channel with Spacing" << volume->spacing();
}
