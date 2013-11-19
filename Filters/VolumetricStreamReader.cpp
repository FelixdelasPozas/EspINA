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

#include <QDebug>
#include <QFileDialog>

using namespace EspINA;


//----------------------------------------------------------------------------
VolumetricStreamReader::VolumetricStreamReader(OutputSList inputs, Type type, SchedulerSPtr scheduler)
: Filter(inputs, type, scheduler)
{

}

//----------------------------------------------------------------------------
void VolumetricStreamReader::restoreState(const State& state)
{
  m_fileName = QFileInfo(state.section("", 6).trimmed());
}

//----------------------------------------------------------------------------
State VolumetricStreamReader::saveState() const
{
  State state;

  state += QString("FILE=%1").arg(m_fileName.absoluteFilePath());

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
  if (!m_fileName.exists()) throw File_Not_Found_Exception();

  if (m_outputs.isEmpty()) 
  {
    m_outputs << OutputSPtr{new Output(this, 0)};
  }

  DefaultVolumetricDataSPtr volume{new StreamedVolume<itkVolumeType>(m_fileName)};

  m_outputs[0]->setSpacing(volume->spacing());

  m_outputs[0]->setData(volume);
}

// typedef itk::ChangeInformationImageFilter<itkVolumeType> ChangeInformationFilter;

// //----------------------------------------------------------------------------
// ChannelRepresentationSPtr VolumetricStreamReader::createRepresentationProxy(FilterOutputId id, const FilterOutput::OutputRepresentationName &type)
// {
//   ChannelRepresentationSPtr proxy;
// 
//   Q_ASSERT(m_outputs.contains(id));
//   Q_ASSERT( NULL == m_outputs[id]->representation(type));
// 
//   if (ChannelVolume::TYPE == type)
//     proxy  = ChannelVolumeProxySPtr(new ChannelVolumeProxy());
//   else
//     Q_ASSERT(false);
// 
//   m_outputs[id]->setRepresentation(type, proxy);
// 
//   return proxy;
// }
// 
// //----------------------------------------------------------------------------
// void VolumetricStreamReader::run(FilterOutputId oId)
// {
// }
// 
// //----------------------------------------------------------------------------
// void VolumetricStreamReader::setSpacing(itkVolumeType::SpacingType spacing)
// {
//   Q_ASSERT(m_outputs.size() == 1);
// 
//   m_args[SPACING] = QString("%1,%2,%3").arg(spacing[0]).arg(spacing[1]).arg(spacing[2]);
// 
//   ChannelVolumeSPtr volume = channelVolume(m_outputs[0]);
// 
//   ChangeInformationFilter::Pointer changer = ChangeInformationFilter::New();
//   changer->SetInput(volume->toITK());
//   changer->SetChangeSpacing(true);
//   changer->SetOutputSpacing(spacing);
//   changer->Update();
// 
//   addOutputRepresentation(0, RawChannelVolumeSPtr(new RawChannelVolume(changer->GetOutput())));
// }
// 
// //----------------------------------------------------------------------------
// itkVolumeType::SpacingType VolumetricStreamReader::spacing()
// {
//   itkVolumeType::SpacingType res;
//   QStringList values = m_args.value(SPACING, "-1,-1,-1").split(",");
//   for(int i = 0; i < 3; i++)
//     res[i] = values[i].toDouble();
//   return res;
// }
// 