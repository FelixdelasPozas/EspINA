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


#include "ChannelReader.h"
#include <Core/IO/IOErrorHandler.h>
#include <Core/Outputs/ChannelVolumeProxy.h>
#include <Core/Outputs/RawVolume.h>

#include <itkMetaImageIO.h>
#include <itkTIFFImageIO.h>
#include <itkChangeInformationImageFilter.h>

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

using namespace EspINA;

const QString ChannelReader::TYPE = "Channel Reader";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ChannelReader::FILE    = "File";
const ArgumentId ChannelReader::SPACING = "Spacing";

typedef itk::ChangeInformationImageFilter<itkVolumeType> ChangeInformationFilter;

//----------------------------------------------------------------------------
ChannelReader::ChannelReader(NamedInputs     inputs,
                             Arguments       args,
                             FilterType      type,
                             IOErrorHandler *handler)
: ChannelFilter(inputs, args, type)
, m_handler(handler)
{
}

//----------------------------------------------------------------------------
QString ChannelReader::serialize() const
{
  return m_args.serialize();
}

//----------------------------------------------------------------------------
ChannelRepresentationSPtr ChannelReader::createRepresentationProxy(FilterOutputId id, const FilterOutput::OutputRepresentationName &type)
{
  ChannelRepresentationSPtr proxy;

  Q_ASSERT(m_outputs.contains(id));
  Q_ASSERT( NULL == m_outputs[id]->representation(type));

  if (ChannelVolume::TYPE == type)
    proxy  =ChannelVolumeProxySPtr(new ChannelVolumeProxy());
  else
    Q_ASSERT(false);

  m_outputs[id]->setRepresentation(type, proxy);

  return proxy;
}

//----------------------------------------------------------------------------
bool ChannelReader::needUpdate(FilterOutputId oId) const
{
  return ChannelFilter::needUpdate(oId);
}

//----------------------------------------------------------------------------
void ChannelReader::run()
{
  run(0);
}

//----------------------------------------------------------------------------
void ChannelReader::run(FilterOutputId oId)
{
  qDebug() << "Executing ChannelReader run";
  Q_ASSERT(0 == oId);
  QFileInfo file = m_args[FILE];
  if (!file.exists())
    file = m_handler->fileNotFound(file);

  m_args[FILE] = file.absoluteFilePath();
  EspinaVolumeReader::Pointer reader = EspinaVolumeReader::New();

  itk::ImageIOBase::Pointer io;

  QString ext = file.suffix();
  if ("mhd" == ext || "mha" == ext)
    io = itk::MetaImageIO::New();
  else if ("tif" == ext || "tiff" == ext)
      io = itk::TIFFImageIO::New();
  else
  {
    qWarning() << QString("Cached Object Builder: Couldn't load file %1."
    "Extension not supported")
    .arg(file.absoluteFilePath());
    Q_ASSERT(false);
  }

  io->SetFileName(m_args[FILE].toUtf8());
  reader->SetImageIO(io);
  reader->SetFileName(m_args[FILE].toUtf8().data());
  reader->Update();

  addOutputRepresentation(0, RawChannelVolumeSPtr(new RawChannelVolume(reader->GetOutput())));

  if (m_args.contains(SPACING))
    setSpacing(spacing());
}

//----------------------------------------------------------------------------
void ChannelReader::setSpacing(itkVolumeType::SpacingType spacing)
{
  Q_ASSERT(m_outputs.size() == 1);

  m_args[SPACING] = QString("%1,%2,%3").arg(spacing[0]).arg(spacing[1]).arg(spacing[2]);

  ChannelVolumeSPtr volume = channelVolume(m_outputs[0]);

  ChangeInformationFilter::Pointer changer = ChangeInformationFilter::New();
  changer->SetInput(volume->toITK());
  changer->SetChangeSpacing(true);
  changer->SetOutputSpacing(spacing);
  changer->Update();

  addOutputRepresentation(0, RawChannelVolumeSPtr(new RawChannelVolume(changer->GetOutput())));
}

//----------------------------------------------------------------------------
itkVolumeType::SpacingType ChannelReader::spacing()
{
  itkVolumeType::SpacingType res;
  QStringList values = m_args.value(SPACING, "-1,-1,-1").split(",");
  for(int i = 0; i < 3; i++)
    res[i] = values[i].toDouble();
  return res;
}
