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
const ArgumentId ChannelReader::FILE    = "File";
const ArgumentId ChannelReader::SPACING = "Spacing";

//----------------------------------------------------------------------------
ChannelReader::ChannelReader(Filter::NamedInputs inputs,
                             ModelItem::Arguments args)
: Filter(inputs, args)
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
bool ChannelReader::needUpdate() const
{
  return m_outputs.isEmpty();
}

//----------------------------------------------------------------------------
void ChannelReader::run()
{
  //qDebug() << "Creating channel from args" << m_args;
  QFileInfo file = m_args[FILE];
  if (!file.exists())
  {
    QFileDialog fileDialog;
    fileDialog.setObjectName("SelectChannelFile");
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setWindowTitle(QString("Select file for %1:").arg(file.fileName()));
    fileDialog.setFilter(tr("Channel File (*.%1)").arg(file.suffix()));

    if (fileDialog.exec() != QDialog::Accepted)
      return;

    m_args[FILE] = fileDialog.selectedFiles().first();
  }

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
  m_reader->SetImageIO(m_io);
  m_reader->SetFileName(m_args[FILE].toStdString());
  m_reader->Update();

  if (m_args.contains(SPACING))
    m_reader->GetOutput()->SetSpacing(spacing());

  EspinaVolume::Pointer volume = m_reader->GetOutput();

  m_outputs.clear();
  m_outputs << FilterOutput(this, 0, volume);
}

//----------------------------------------------------------------------------
void ChannelReader::setSpacing(itk::Image< unsigned int, 3 >::SpacingType spacing)
{
  Q_ASSERT(m_outputs.size() == 1);

  m_args[SPACING] = QString("%1,%2,%3")
  .arg(spacing[0]).arg(spacing[1]).arg(spacing[2]);

  m_outputs[0].volume->SetSpacing(spacing);

  emit modified(this);
}

//----------------------------------------------------------------------------
EspinaVolume::SpacingType ChannelReader::spacing()
{
  EspinaVolume::SpacingType res;
  QStringList values = m_args.value(SPACING, "-1,-1,-1").split(",");
  for(int i = 0; i < 3; i++)
    res[i] = values[i].toDouble();
  return res;
}
