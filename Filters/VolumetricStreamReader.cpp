/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "VolumetricStreamReader.h"

#include <Core/Analysis/Data/Volumetric/StreamedVolume.h>

#include <QFileDialog>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <Core/IO/ErrorHandler.h>

using namespace EspINA;

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
  return needUpdate(0);
}

//----------------------------------------------------------------------------
bool VolumetricStreamReader::needUpdate(Output::Id id) const
{
  if (id != 0) throw Undefined_Output_Exception();

  return m_outputs.isEmpty() || !validOutput(0);
}

//----------------------------------------------------------------------------
void VolumetricStreamReader::execute()
{
  execute(0);
}

//----------------------------------------------------------------------------
void VolumetricStreamReader::execute(Output::Id id)
{
  Q_ASSERT(0 == id);
  if (!m_fileName.exists())
  {
    if (handler())
    {
      m_fileName = m_handler->fileNotFound(m_fileName);
    }

    if (!m_fileName.exists())
    {
      throw File_Not_Found_Exception();
    }
  }

  QFileInfo mhdFile = m_fileName;
  QString mhdFileName = m_fileName.baseName() + QString(".mhd");

  QString fileInAnotherStorage = storage()->findFile(mhdFileName);
  if(fileInAnotherStorage != QString())
  {
    mhdFile = QFileInfo(fileInAnotherStorage);
  }
  else
  {
    if (mhdFile.fileName().endsWith(".tif"))
    {
      using VolumeReader = itk::ImageFileReader<itkVolumeType>;
      using VolumeWriter = itk::ImageFileWriter<itkVolumeType>;

      VolumeReader::Pointer reader = VolumeReader::New();
      reader->SetFileName(mhdFile.absoluteFilePath().toUtf8().data());
      reader->Update();

      mhdFile = QFileInfo(storage()->absoluteFilePath(mhdFile.baseName() + ".mhd"));

      VolumeWriter::Pointer writer = VolumeWriter::New();
      writer->SetFileName(mhdFile.absoluteFilePath().toUtf8().data());
      writer->SetInput(reader->GetOutput());
      writer->Write();
    }
  }

  if (!m_outputs.contains(0))
  {
    m_outputs[0] = OutputSPtr{new Output(this, 0)};
  }

  DefaultVolumetricDataSPtr volume{new StreamedVolume<itkVolumeType>(mhdFile)};

  m_outputs[0]->setData(volume);

  if (m_outputs[0]->spacing() == NmVector3())
  {
    m_outputs[0]->setSpacing(volume->spacing());
  }
  else
  {
    volume->setSpacing(m_outputs[0]->spacing());
  }
}
