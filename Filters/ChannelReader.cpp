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
#include <Core/IO/ErrorHandler.h>

#include <itkMetaImageIO.h>
#include <itkTIFFImageIO.h>

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

using namespace EspINA;

const QString ChannelReader::TYPE = "Channel Reader";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ChannelReader::FILE    = "File";
const ArgumentId ChannelReader::SPACING = "Spacing";

//----------------------------------------------------------------------------
ChannelReader::ChannelReader(NamedInputs             inputs,
                             Arguments               args,
                             FilterType              type,
                             EspinaIO::ErrorHandler *handler)
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
bool ChannelReader::needUpdate() const
{
  return Filter::needUpdate();
}

//----------------------------------------------------------------------------
void ChannelReader::run()
{
  //qDebug() << "Creating channel from args" << m_args;
  QFileInfo file = m_args[FILE];
  if (!file.exists())
    file = m_handler->fileNotFound(file);

  if (file.exists())
    m_args[FILE] = file.absoluteFilePath();
  else
    throw 1; // TODO: Find proper extension code

  m_reader = EspinaVolumeReader::New();

  QString ext = file.suffix();
  if ("mhd" == ext || "mha" == ext)
    m_io = itk::MetaImageIO::New();
  else if ("tif" == ext || "tiff" == ext)
    m_io = itk::TIFFImageIO::New();
  else
  {
    qWarning() << QString("Cached Object Builder: Couldn't load file %1."
    "Extension not supported")
    .arg(file.absoluteFilePath());
    Q_ASSERT(false);
  }
  m_io->SetFileName(m_args[FILE].toUtf8());
  m_reader->SetImageIO(m_io);
  m_reader->SetFileName(m_args[FILE].toUtf8().data());
  m_reader->Update();

  if (m_args.contains(SPACING))
    m_reader->GetOutput()->SetSpacing(spacing());

  createOutput(0, m_reader->GetOutput());
}

//----------------------------------------------------------------------------
void ChannelReader::setSpacing(itkVolumeType::SpacingType spacing)
{
  Q_ASSERT(m_outputs.size() == 1);

  m_args[SPACING] = QString("%1,%2,%3")
  .arg(spacing[0]).arg(spacing[1]).arg(spacing[2]);

  m_outputs[0].volume->toITK()->SetSpacing(spacing);

  emit modified(this);
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
