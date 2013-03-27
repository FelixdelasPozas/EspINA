/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "EspinaFactory.h"

#include "Core/Interfaces/IFileReader.h"
#include "Core/Interfaces/IFilterCreator.h"
#include "Core/Model/Sample.h"
#include "Core/Model/Channel.h"
#include "Core/Model/Segmentation.h"
#include <Core/Extensions/ChannelExtension.h>
#include <Core/Extensions/SegmentationExtension.h>
#include <Core/IO/ErrorHandler.h>
#include "GUI/Renderers/Renderer.h"

#include <QDebug>

using namespace EspINA;

//------------------------------------------------------------------------
EspinaFactory::EspinaFactory()
{
  // Register Default Extensions
  m_supportedFiles << CHANNEL_FILES;
  m_supportedExtensions << "*.mhd" << "*.mha" << "*.tif";

  m_supportedFiles << SEG_FILES;
  m_supportedExtensions << "*.seg";
}

//------------------------------------------------------------------------
EspinaFactory::~EspinaFactory()
{
  qDebug() << "Destroying Espina Factory";
}

//------------------------------------------------------------------------
QStringList EspinaFactory::supportedFiles() const
{
  QStringList files;
  QString espinaFiles =  QObject::tr("EspINA Files (%1)").arg(m_supportedExtensions.join(" "));

  files << espinaFiles << m_supportedFiles;
  return files;
}

//------------------------------------------------------------------------
void EspinaFactory::registerFilter(IFilterCreatorPtr creator,
                                   const QString &filter)
{
  Q_ASSERT(m_filterCreators.contains(filter) == false);
  m_filterCreators[filter] = creator;
}

//------------------------------------------------------------------------
void EspinaFactory::registerReaderFactory(IFileReaderPtr     reader,
                                          const QString     &description,
                                          const QStringList &extensions)
{
  m_supportedFiles << description;

  foreach(QString extension, extensions)
  {
    Q_ASSERT(m_fileReaders.contains(extension) == false);
    m_fileReaders[extension] = reader;
    m_supportedExtensions << QString("*.%1").arg(extension);
  }
}

//------------------------------------------------------------------------
void EspinaFactory::registerChannelExtension(Channel::ExtensionPtr extension)
{
  Q_ASSERT(m_channelExtensions.contains(extension) == false);
  m_channelExtensions << extension;
}

//------------------------------------------------------------------------
void EspinaFactory::unregisterChannelExtension(Channel::ExtensionPtr extension)
{
  Q_ASSERT(m_channelExtensions.contains(extension));
  m_channelExtensions.removeOne(extension);
}

//------------------------------------------------------------------------
Channel::ExtensionPtr EspinaFactory::channelExtension(ModelItem::ExtId extensionId) const
{
  Channel::ExtensionPtr extension = NULL;

  int i = 0;
  while (!extension && i < m_channelExtensions.size())
  {
    Channel::ExtensionPtr tmp = m_channelExtensions[i];
    if (tmp->id() == extensionId)
      extension = tmp;
    ++i;
  }

  return extension;
}

//------------------------------------------------------------------------
void EspinaFactory::registerSegmentationExtension(Segmentation::InformationExtension extension)
{
  Q_ASSERT(m_segmentationExtensions.contains(extension) == false);
  m_segmentationExtensions << extension;
}

//------------------------------------------------------------------------
void EspinaFactory::unregisterSegmentationExtension(Segmentation::InformationExtension extension)
{
  Q_ASSERT(m_segmentationExtensions.contains(extension));
  m_segmentationExtensions.removeOne(extension);
}

//------------------------------------------------------------------------
Segmentation::InformationExtension EspinaFactory::segmentationExtension(ModelItem::ExtId extensionId) const
{
  Segmentation::InformationExtension extension = NULL;

  int i = 0;
  while (!extension && i < m_segmentationExtensions.size())
  {
    Segmentation::InformationExtension tmp = m_segmentationExtensions[i];
    if (tmp->id() == extensionId)
      extension = tmp;
    ++i;
  }

  return extension;
}

//------------------------------------------------------------------------
Segmentation::InformationExtension EspinaFactory::informationProvider(Segmentation::InfoTag tag) const
{
  Segmentation::InformationExtension extension = NULL;

  int i = 0;
  while (!extension && i < m_segmentationExtensions.size())
  {
    Segmentation::InformationExtension tmp = m_segmentationExtensions[i];
    if (tmp->availableInformations().contains(tag))
      extension = tmp;
    ++i;
  }

  return extension;
}

//------------------------------------------------------------------------
void EspinaFactory::registerRenderer(IRenderer *renderer)
{
  Q_ASSERT(!m_renderers.contains(renderer->name()));
  m_renderers[renderer->name()] = renderer;
}

//------------------------------------------------------------------------
void EspinaFactory::unregisterRenderer(IRenderer *renderer)
{
  Q_ASSERT(m_renderers.contains(renderer->name()));
  m_renderers.remove(renderer->name());
}

//------------------------------------------------------------------------
FilterSPtr EspinaFactory::createFilter(const QString              &filter,
                                      const Filter::NamedInputs  &inputs,
                                      const ModelItem::Arguments &args)
{
  Q_ASSERT(m_filterCreators.contains(filter));
  return m_filterCreators[filter]->createFilter(filter, inputs, args);
}

//------------------------------------------------------------------------
bool EspinaFactory::readFile(const QString &file, const QString &ext, EspinaIO::ErrorHandler *handler)
{
  bool success = false;
  if (m_fileReaders.contains(ext))
    success = m_fileReaders[ext]->readFile(file, handler);
  else if (handler)
    handler->error(QObject::tr("%1 file extension is not supported").arg(ext));

  return success;
}

//------------------------------------------------------------------------
SampleSPtr EspinaFactory::createSample(const QString &id, const QString &args)
{
  SampleSPtr sample;

  if (args.isNull())
    sample = SampleSPtr(new Sample(id));
  else
    sample = SampleSPtr(new Sample(id, args));

  return sample;
}

//------------------------------------------------------------------------
ChannelSPtr EspinaFactory::createChannel(FilterSPtr filter,
                                         const Filter::OutputId &oId)
{
  ChannelSPtr channel(new Channel(filter, oId));
//   foreach(ChannelExtensionPtr ext, m_channelExtensions)
//     channel->addExtension(ext->clone());

  return channel;
}

//------------------------------------------------------------------------
SegmentationSPtr EspinaFactory::createSegmentation(FilterSPtr        filter,
                                                        const Filter::OutputId &oId)
{
//   std::cout << "Factory is going to create a segmentation for vtkObject: " << vtkRef->id().toStdString() << std::endl;
  SegmentationSPtr seg(new Segmentation(filter, oId));
//   foreach(SegmentationExtensionPtr ext, m_segExtensions)
//     seg->addExtension(ext->clone());

  return seg;
}
