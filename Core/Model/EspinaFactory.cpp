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

#include "Core/Extensions/Margins/MarginsChannelExtension.h"
#include "Core/Extensions/Margins/MarginsSegmentationExtension.h"
#include "Core/Extensions/Morphological/MorphologicalExtension.h"
#include "Core/Interfaces/IFileReader.h"
#include "Core/Interfaces/IFilterCreator.h"
#include "Core/Model/Segmentation.h"
#include "Filters/ChannelReader.h"
#include "GUI/Renderers/Renderer.h"


//------------------------------------------------------------------------
EspinaFactory::EspinaFactory()
{
  // Register Default Extensions
  registerChannelExtension(ChannelExtension::SPtr(new MarginsChannelExtension()));
  registerSegmentationExtension(SegmentationExtension::SPtr(new MarginsSegmentationExtension()));
  registerSegmentationExtension(SegmentationExtension::SPtr(new MorphologicalExtension()));

  m_supportedFiles << CHANNEL_FILES;
  m_supportedExtensions << "*.mhd" << "*.mha" << "*.tif";
  m_supportedFiles << SEG_FILES;
  m_supportedExtensions << "*.seg";
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
void EspinaFactory::registerFilter(IFilterCreator* creator, const QString filter)
{
  Q_ASSERT(m_filterCreators.contains(filter) == false);
  m_filterCreators[filter] = creator;
}

//------------------------------------------------------------------------
void EspinaFactory::registerReaderFactory(IFileReader* reader,
                                          const QString description,
                                          const QStringList extensions)
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
void EspinaFactory::registerSampleExtension(SampleExtension::SPtr extension)
{
  Q_ASSERT(m_sampleExtensions.contains(extension) == false);
  m_sampleExtensions << extension;
}

//------------------------------------------------------------------------
void EspinaFactory::registerChannelExtension(ChannelExtension::SPtr extension)
{
  Q_ASSERT(m_channelExtensions.contains(extension) == false);
  m_channelExtensions << extension;
}

//------------------------------------------------------------------------
void EspinaFactory::registerSegmentationExtension(SegmentationExtension::SPtr extension)
{
  Q_ASSERT(m_segExtensions.contains(extension) == false);
  m_segExtensions << extension;
}

//------------------------------------------------------------------------
void EspinaFactory::registerRenderer(Renderer* renderer)
{
  m_renderers[renderer->name()] = renderer;
}

//------------------------------------------------------------------------
Filter *EspinaFactory::createFilter(const QString filter,
                                    Filter::NamedInputs inputs,
                                    const ModelItem::Arguments args)
{
  if (ChannelReader::TYPE == filter)
    return new ChannelReader(inputs, args);

  Q_ASSERT(m_filterCreators.contains(filter));
  return m_filterCreators[filter]->createFilter(filter, inputs, args);
}

//------------------------------------------------------------------------
bool EspinaFactory::readFile(const QString file, const QString ext)
{
  Q_ASSERT(m_fileReaders.contains(ext));
  return m_fileReaders[ext]->readFile(file);
}

//------------------------------------------------------------------------
Sample *EspinaFactory::createSample(const QString id, const QString args)
{
  Sample *sample;
  if (args.isNull())
    sample = new Sample(id);
  else
    sample = new Sample(id, args);

  foreach(SampleExtension::SPtr ext, m_sampleExtensions)
    sample->addExtension(ext->clone());

  return sample;
}

//------------------------------------------------------------------------
Channel* EspinaFactory::createChannel(Filter *filter,
                                      Filter::OutputId oId)
{
  Channel *channel = new Channel(filter, oId);
  foreach(ChannelExtension::SPtr ext, m_channelExtensions)
    channel->addExtension(ext->clone());

  return channel;
}

//------------------------------------------------------------------------
Segmentation *EspinaFactory::createSegmentation(Filter* parent,
                                                Filter::OutputId oId)
{
//   std::cout << "Factory is going to create a segmentation for vtkObject: " << vtkRef->id().toStdString() << std::endl;
  Segmentation *seg = new Segmentation(parent, oId);
  foreach(SegmentationExtension::SPtr ext, m_segExtensions)
    seg->addExtension(ext->clone());

  return seg;
}
