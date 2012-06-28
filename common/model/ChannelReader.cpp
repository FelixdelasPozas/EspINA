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

#include <itkMetaImageIO.h>
#include <itkTIFFImageIO.h>

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>

const QString ChannelReader::TYPE = "Channel Reader";

typedef ModelItem::ArgumentId ArgumentId;
const ArgumentId ChannelReader::FILE    = ArgumentId("File", true);
const ArgumentId ChannelReader::SPACING = ArgumentId("Spacing", true);

//----------------------------------------------------------------------------
ChannelReader::ChannelReader(Filter::NamedInputs inputs,
                             ModelItem::Arguments args)
: Filter(inputs, args)
, m_volume(NULL)
{
}

//----------------------------------------------------------------------------
QVariant ChannelReader::data(int role) const
{
  if (Qt::DisplayRole == role)
    return TYPE;
  else
    return QVariant();
}

//----------------------------------------------------------------------------
QString ChannelReader::serialize() const
{
  return m_args.serialize();
}

//----------------------------------------------------------------------------
void ChannelReader::run()
{
  qDebug() << "Creating channel from args" << m_args;
  QFileInfo file = m_args[FILE];
  if (!file.exists())
  {
    QString filters; //TODO: Get previous extension
    QFileDialog fileDialog(0,
                           QString(),
                           QString(),
                           filters);
    fileDialog.setObjectName("SelectChannelFile");
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setWindowTitle(QString("Select file for %1:").arg(m_args[ID]));

    Q_ASSERT(fileDialog.exec() == QDialog::Accepted);

    m_args[FILE] = fileDialog.selectedFiles().first();
  }

  double channelSpacing[3];
  //spacing(spacing);

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
  m_io->SetFileName(m_args[FILE].toAscii());
  //   if (spacing[0] > 0)
  //   {
    //     for(int i=0; i<3; i++)
  //       io->SetSpacing(i, spacing[i]);
  //   }
  m_reader->SetImageIO(m_io);
  m_reader->SetFileName(m_args[FILE].toStdString());
  m_reader->Update();
  for(int i=0; i<3; i++)
    channelSpacing[i] = m_io->GetSpacing(i);

  setSpacing(channelSpacing);

  m_volume = m_reader->GetOutput();
}

//----------------------------------------------------------------------------
int ChannelReader::numberOutputs() const
{
  //Filter::update();
  return m_volume?1:0;
}

//----------------------------------------------------------------------------
EspinaVolume* ChannelReader::output(OutputNumber i) const
{
  //Filter::update();
  if (m_volume && 0 == i)
    return m_volume;

  Q_ASSERT(false);
  return NULL;
}

//----------------------------------------------------------------------------
void ChannelReader::setSpacing(double value[3])
{
//       memcpy(m_spacing, value, 3*sizeof(double));
//       (*this)[SPACING] = QString("%1,%2,%3")
//                          .arg(value[0])
//                          .arg(value[1])
//                          .arg(value[2]);
}

//----------------------------------------------------------------------------
void ChannelReader::spacing(double value[3])
{
//       memcpy(value, m_spacing, 3*sizeof(double));
}