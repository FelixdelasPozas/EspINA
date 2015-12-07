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
#include <Core/Utils/EspinaException.h>

// ITK
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;

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
      auto what    = QObject::tr("Can't find image file, file name: %1").arg(m_fileName.absoluteFilePath());
      auto details = QObject::tr("VolumetricStreamReader::execute() -> Can't find image file, file name: %1").arg(m_fileName.absoluteFilePath());

      throw EspinaException(what, details);
    }
  }

  using VolumeReader = itk::ImageFileReader<itkVolumeType>;
  using VolumeWriter = itk::ImageFileWriter<itkVolumeType>;

  QFileInfo mhdFile   = m_fileName;
  VolumeReader::Pointer reader = VolumeReader::New();
  reader->SetFileName(mhdFile.absoluteFilePath().toUtf8().data());
  try
  {
    reader->Update();
  }
  catch(itk::ExceptionObject &e)
  {
    auto what    = QObject::tr("Error in itk, exception message: %1").arg(QString(e.what()));
    auto details = QObject::tr("VolumetricStreamReader::execute() -> Error in itk, exception message: %1").arg(QString(e.what()));

    throw EspinaException(what, details);
  }

  auto volume  = std::make_shared<RawVolume<itkVolumeType>>(reader->GetOutput());
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

  qDebug() << "Loading Channel with Spacing" << volume->bounds().spacing() << "output spacing" << m_outputs[0]->spacing();
}
