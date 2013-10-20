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

#include "CoreFactory.h"

#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Channel.h"

using namespace EspINA;

//------------------------------------------------------------------------
CoreFactory::CoreFactory()
{

}

//-----------------------------------------------------------------------------
CoreFactory::~CoreFactory()
{

}

//-----------------------------------------------------------------------------
void CoreFactory::registerFilter(FilterCreatorPtr creator, const Filter::Type& filter)
{

}

//-----------------------------------------------------------------------------
SampleSPtr CoreFactory::createSample(const QString& name) const
{
  return SampleSPtr(new Sample(name));
}

//-----------------------------------------------------------------------------
FilterSPtr CoreFactory::createFilter(OutputSList inputs, Filter::Type& filter) const
{

}

//-----------------------------------------------------------------------------
ChannelSPtr CoreFactory::createChannel(OutputSPtr output) const
{
  return ChannelSPtr(new Channel(output));
}

//-----------------------------------------------------------------------------
SegmentationSPtr CoreFactory::createSegmentation(OutputSPtr output) const
{

}

//-----------------------------------------------------------------------------
void CoreFactory::registerExtensionProvider(ExtensionProviderPtr provider)
{

}

//-----------------------------------------------------------------------------
void CoreFactory::unregisterExtensionProvider(ExtensionProviderPtr provider)
{

}


//-----------------------------------------------------------------------------
ExtensionProviderPtr CoreFactory::extensionProvider(const ExtensionProvider::Type& type) const
{

}

// //------------------------------------------------------------------------
// void EspinaFactory::registerChannelExtension(Channel::ExtensionPtr extension)
// {
//   Q_ASSERT(m_channelExtensions.contains(extension) == false);
//   m_channelExtensions << extension;
// }
// 
// //------------------------------------------------------------------------
// void EspinaFactory::unregisterChannelExtension(Channel::ExtensionPtr extension)
// {
//   Q_ASSERT(m_channelExtensions.contains(extension));
//   m_channelExtensions.removeOne(extension);
// }
// 
// //------------------------------------------------------------------------
// Channel::ExtensionPtr EspinaFactory::channelExtension(ModelItem::ExtId extensionId) const
// {
//   Channel::ExtensionPtr extension = NULL;
// 
//   int i = 0;
//   while (!extension && i < m_channelExtensions.size())
//   {
//     Channel::ExtensionPtr tmp = m_channelExtensions[i];
//     if (tmp->id() == extensionId)
//       extension = tmp;
//     ++i;
//   }
// 
//   return extension;
// }
// 
// //------------------------------------------------------------------------
// void EspinaFactory::registerSegmentationExtension(Segmentation::InformationExtension extension)
// {
//   Q_ASSERT(m_segmentationExtensions.contains(extension) == false);
//   m_segmentationExtensions << extension;
// }
// 
// //------------------------------------------------------------------------
// void EspinaFactory::unregisterSegmentationExtension(Segmentation::InformationExtension extension)
// {
//   Q_ASSERT(m_segmentationExtensions.contains(extension));
//   m_segmentationExtensions.removeOne(extension);
// }
// 
// //------------------------------------------------------------------------
// Segmentation::InformationExtension EspinaFactory::segmentationExtension(ModelItem::ExtId extensionId) const
// {
//   Segmentation::InformationExtension extension = NULL;
// 
//   int i = 0;
//   while (!extension && i < m_segmentationExtensions.size())
//   {
//     Segmentation::InformationExtension tmp = m_segmentationExtensions[i];
//     if (tmp->id() == extensionId)
//       extension = tmp;
//     ++i;
//   }
// 
//   return extension;
// }
// 
// //------------------------------------------------------------------------
// Segmentation::InformationExtension EspinaFactory::informationProvider(Segmentation::InfoTag tag) const
// {
//   Segmentation::InformationExtension extension = NULL;
// 
//   int i = 0;
//   while (!extension && i < m_segmentationExtensions.size())
//   {
//     Segmentation::InformationExtension tmp = m_segmentationExtensions[i];
//     if (tmp->availableInformations().contains(tag))
//       extension = tmp;
//     ++i;
//   }
// 
//   return extension;
// }
// 
// //------------------------------------------------------------------------
// void EspinaFactory::registerRenderer(IRenderer *renderer)
// {
//   Q_ASSERT(!m_renderers.contains(renderer->name()));
//   m_renderers[renderer->name()] = renderer;
// }
// 
// //------------------------------------------------------------------------
// void EspinaFactory::unregisterRenderer(IRenderer *renderer)
// {
//   Q_ASSERT(m_renderers.contains(renderer->name()));
//   m_renderers.remove(renderer->name());
// }
// 
// //------------------------------------------------------------------------
// FilterSPtr EspinaFactory::createFilter(const QString              &filter,
//                                       const Filter::NamedInputs  &inputs,
//                                       const ModelItem::Arguments &args)
// {
//   Q_ASSERT(m_filterCreators.contains(filter));
//   return m_filterCreators[filter]->createFilter(filter, inputs, args);
// }
// 
// //------------------------------------------------------------------------
// bool EspinaFactory::readFile(const QString &file, const QString &ext, IOErrorHandler *handler)
// {
//   bool success = false;
//   if (m_fileReaders.contains(ext))
//     success = m_fileReaders[ext]->readFile(file, handler);
//   else if (handler)
//     handler->error(QObject::tr("%1 file extension is not supported").arg(ext));
// 
//   return success;
// }
// 
// //------------------------------------------------------------------------
// SegmentationSPtr EspinaFactory::createSegmentation(FilterSPtr        filter,
//                                                         const FilterOutputId &oId)
// {
// //   std::cout << "Factory is going to create a segmentation for vtkObject: " << vtkRef->id().toStdString() << std::endl;
//   SegmentationSPtr seg(new Segmentation(filter, oId));
// //   foreach(SegmentationExtensionPtr ext, m_segExtensions)
// //     seg->addExtension(ext->clone());
// 
//   return seg;
// }
// 