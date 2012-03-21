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

#include "common/extensions/Margins/MarginsChannelExtension.h"
#include "common/extensions/Margins/MarginsSegmentationExtension.h"
#include "common/extensions/Morphological/MorphologicalExtension.h"

//------------------------------------------------------------------------
EspinaFactory *EspinaFactory::m_instance = NULL;

//------------------------------------------------------------------------
EspinaFactory::EspinaFactory()
{
  // Register Default Extensions
//   registerChannelExtension(ChannelExtension::SPtr(new MarginsChannelExtension()));
//   registerSegmentationExtension(SegmentationExtension::SPtr(new MarginsSegmentationExtension()));
  registerSegmentationExtension(SegmentationExtension::SPtr(new MorphologicalExtension()));
}

//------------------------------------------------------------------------
EspinaFactory* EspinaFactory::instance()
{
  if (!m_instance)
    m_instance = new EspinaFactory();
  return m_instance;
}

//------------------------------------------------------------------------
void EspinaFactory::registerFilter(const QString filter, FilterFactory* factory)
{
  Q_ASSERT(m_filterFactory.contains(filter) == false);
  m_filterFactory[filter] = factory;
}

//------------------------------------------------------------------------
void EspinaFactory::registerReader(const QString extension, ReaderFactory* factory)
{
  Q_ASSERT(m_readers.contains(extension) == false);
  m_readers[extension] = factory;
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
Filter *EspinaFactory::createFilter(const QString filter, const ModelItem::Arguments args)
{
  Q_ASSERT(m_filterFactory.contains(filter));

  return m_filterFactory[filter]->createFilter(filter, args);
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
Channel* EspinaFactory::createChannel(const QString id, const ModelItem::Arguments args)
{
  Channel *channel = new Channel(id, args);
  foreach(ChannelExtension::SPtr ext, m_channelExtensions)
    channel->addExtension(ext->clone());

  return channel;
}

//------------------------------------------------------------------------
Segmentation *EspinaFactory::createSegmentation(Filter* parent, int output, pqData data)
{
//   std::cout << "Factory is going to create a segmentation for vtkObject: " << vtkRef->id().toStdString() << std::endl;
  Segmentation *seg = new Segmentation(parent, output, data);
  foreach(SegmentationExtension::SPtr ext, m_segExtensions)
    seg->addExtension(ext->clone());

  return seg;
}

//------------------------------------------------------------------------
void EspinaFactory::readFile(const QString file, const QString ext)
{
  Q_ASSERT(m_readers.contains(ext));
  m_readers[ext]->readFile(file);
}


// 
// void EspinaFactory::addSampleExtension(ISampleExtension* ext)
// {
// //   qDebug() << ext->id() << "registered in Sample Factory";
//   m_sampleExtensions.append(ext->clone());
// }

// QStringList EspinaFactory::segmentationAvailableInformations()
// {
//   QStringList informations;
//   informations << "Name" << "Taxonomy";
//   foreach (ISegmentationExtension *ext, m_segExtensions)
//     informations << ext->availableInformations();
//   
//   return informations;
// }
// 
// 
// VolumeView* EspinaFactory::CreateVolumeView()
// {
//   VolumeView *view = new VolumeView();
//   foreach(IViewWidget *widget, m_widgets)
//   {
//     view->addWidget(widget);
//   }
//   return view;
// }
// 
// void EspinaFactory::addViewWidget(IViewWidget* widget)
// {
// //   qDebug() << "registered new widget in Factory";
//   m_widgets.append(widget/*->clone()*/); //TODO clone method hasn't an implementation
// }
// 


