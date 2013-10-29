/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor<jpena@cesvima.upm.es>

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
#include "ChannelAdapter.h"

using namespace EspINA;

ChannelAdapter::ChannelAdapter(ChannelSPtr channel)
: ViewItemAdapter(channel)
{

}

ChannelAdapter::~ChannelAdapter()
{

}



void ChannelAdapter::addExtension(ChannelExtensionSPtr extension)
{

}

Bounds ChannelAdapter::bounds() const
{

}

double ChannelAdapter::brightness() const
{

}

QVariant ChannelAdapter::data(int role) const
{

}

double ChannelAdapter::contrast() const
{

}

void ChannelAdapter::deleteExtension(ChannelExtensionSPtr extension)
{

}

bool ChannelAdapter::hasExtension(const ChannelExtension::Type& type) const
{

}

double ChannelAdapter::opacity() const
{

}

double ChannelAdapter::hue() const
{

}

double ChannelAdapter::saturation() const
{

}

void ChannelAdapter::setBrightness(double brightness)
{

}

void ChannelAdapter::setHue(double hue)
{

}

void ChannelAdapter::setOpacity(double opacity)
{

}


void ChannelAdapter::setPosition(Nm point[3])
{

}


ChannelExtensionSPtr ChannelAdapter::extension(const ChannelExtension::Type& type)
{

}

void ChannelAdapter::position(Nm point[3])
{

}

void ChannelAdapter::setContrast(double contrast)
{

}

bool ChannelAdapter::setData(const QVariant& value, int role)
{

}

void ChannelAdapter::setSaturation(double saturation)
{

}
// #include "Core/Model/EspinaFactory.h"
// #include "Core/Model/Filter.h"
// #include "Core/Extensions/ChannelExtension.h"
// #include "Core/Extensions/ModelItemExtension.h"
// #include "Core/Model/RelationshipGraph.h"
// #include "Core/Model/Sample.h"
// #include "EspinaModel.h"
// 
// #include <itkImageFileReader.h>
// #include <itkMetaImageIO.h>
// #include <itkTIFFImageIO.h>
// 
// #include <vtkImageAlgorithm.h>
// #include <vtkDataObject.h>
// #include <vtkImageData.h>
// 
// #include <QDebug>
// #include <QFileDialog>
// 
// using namespace EspINA;
// 
// const ModelItem::ArgumentId Channel::ID         = "ID";
// const ModelItem::ArgumentId Channel::HUE        = "Hue";
// const ModelItem::ArgumentId Channel::OPACITY    = "Opacity";
// const ModelItem::ArgumentId Channel::SATURATION = "Saturation";
// const ModelItem::ArgumentId Channel::CONTRAST   = "Contrast";
// const ModelItem::ArgumentId Channel::BRIGHTNESS = "Brightness";
// const ModelItem::ArgumentId Channel::VOLUME     = "Volume";
// 
// const QString Channel::LINK       = "Channel";
// const QString Channel::STAIN_LINK  = "Stain";
// const QString Channel::VOLUME_LINK = "Volume";
// 
// const QString Channel::NAME       = "Name";
// const QString Channel::VOLUMETRIC = "Volumetric";
// 
// //-----------------------------------------------------------------------------
// Channel::Channel(FilterSPtr filter, FilterOutputId oId)
// : PickableItem()
// , m_visible(true)
// , m_filter(filter)
// {
//   memset(m_pos, 0, 3*sizeof(Nm));
//   m_args.setOutputId(oId);
// }
// 
// //-----------------------------------------------------------------------------
// Channel::~Channel()
// {
//   //qDebug() << data().toString() << ": Destructor";
//   // Extensions may need access channel's information
//   foreach(ExtensionPtr extension, m_extensions)
//   {
//     delete extension;
//   }
// }
// 
// //------------------------------------------------------------------------
// const FilterSPtr Channel::filter() const
// {
//   return m_filter;
// }
// 
// //------------------------------------------------------------------------
// const FilterOutputId Channel::outputId() const
// {
//   return m_args.outputId();
// }
// 
// //------------------------------------------------------------------------
// ChannelVolumeSPtr Channel::volume()
// {
//   return static_cast<const Channel *>(this)->volume();
// }
// 
// //------------------------------------------------------------------------
// const ChannelVolumeSPtr Channel::volume() const
// {
//   return channelVolume(output());
// }
// 
// 
// //------------------------------------------------------------------------
// void Channel::setPosition(Nm pos[3])
// {
//   memcpy(m_pos, pos, 3*sizeof(Nm));
// }
// 
// //------------------------------------------------------------------------
// void Channel::position(Nm pos[3])
// {
//   memcpy(pos, m_pos, 3*sizeof(Nm));
// }
// 
// //------------------------------------------------------------------------
// void Channel::setHue(const double hue)
// {
//   m_args.setHue(hue);
// }
// 
// //------------------------------------------------------------------------
// double Channel::hue() const
// {
//   return m_args.hue();
// }
// 
// //------------------------------------------------------------------------
// void Channel::setOpacity(const double opacity)
// {
//   m_args.setOpacity(opacity);
// }
// 
// //------------------------------------------------------------------------
// double Channel::opacity() const
// {
//   return m_args.opacity();
// }
// 
// //------------------------------------------------------------------------
// void Channel::setSaturation(const double saturation)
// {
//   m_args.setSaturation(saturation);
// }
// 
// //------------------------------------------------------------------------
// double Channel::saturation() const
// {
//   return m_args.saturation();
// }
// 
// //------------------------------------------------------------------------
// void Channel::setBrightness(const double brightness)
// {
//   m_args.setBrightness(brightness);
// }
// 
// //------------------------------------------------------------------------
// double Channel::brightness() const
// {
//   return m_args.brightness();
// }
// 
// //------------------------------------------------------------------------
// void Channel::setContrast(const double contrast)
// {
//   m_args.setContrast(contrast);
// }
// 
// //------------------------------------------------------------------------
// double Channel::contrast() const
// {
//   return m_args.contrast();
// }
// 
// //------------------------------------------------------------------------
// QVariant Channel::data(int role) const
// {
//   switch (role)
//   {
//     case Qt::DisplayRole:
//       return m_args[ID];
//     case Qt::CheckStateRole:
//       return m_visible?Qt::Checked: Qt::Unchecked;
//     default:
//       return QVariant();
//   }
// }
// 
// //------------------------------------------------------------------------
// bool Channel::setData(const QVariant& value, int role)
// {
//   switch (role)
//   {
//     case Qt::EditRole:
//       return true;
//     case Qt::CheckStateRole:
//       setVisible(value.toBool());
//       return true;
//     default:
//       return false;
//   }
// }
// 
// //------------------------------------------------------------------------
// QString Channel::serialize() const
// {
//   return m_args.serialize();
// }
// 
// //-----------------------------------------------------------------------------
// void Channel::initialize(const Arguments &args)
// {
//   //qDebug() << "Init" << data().toString() << "with args:" << args;
//   foreach(ArgumentId argId, args.keys())
//   {
//     // changes in seg file format
//     if (QString("Color").compare(argId) == 0)
//     {
//       m_args[HUE] = args[argId];
// 
//       setOpacity(-1.0);
//       if (hue() != -1.0)
//         setSaturation(1.0);
//       else
//         setSaturation(0.0);
//       setContrast(1.0);
//       setBrightness(0.0);
//       continue;
//     }
// 
//     if (argId != EXTENSIONS)
//       m_args[argId] = args[argId];
//   }
// }
// 
// //------------------------------------------------------------------------
// void Channel::initializeExtensions()
// {
// //   qDebug() << "Initializing" << data().toString() << "extensions:";
//   foreach(Channel::ExtensionPtr channelExtension, m_extensions)
//   {
//     channelExtension->initialize();
//   }
// }
// 
// //------------------------------------------------------------------------
// void Channel::invalidateExtensions()
// {
//   foreach(Channel::ExtensionPtr ext, m_extensions)
//   {
//     ext->invalidate();
//   }
// }
// 
// //------------------------------------------------------------------------
// void Channel::addExtension(Channel::ExtensionPtr extension)
// {
//   if (m_extensions.contains(extension->id()))
//   {
//     qWarning() << "Extension already registered";
//     Q_ASSERT(false);
//   }
// 
//   foreach(ModelItem::ExtId requiredExtensionId, extension->dependencies())
//   {
//     ExtensionPtr requiredExtension = Channel::extension(requiredExtensionId);
//     if (!requiredExtension)
//     {
//       EspinaFactory *factory = m_model->factory();
//       ExtensionPtr prototype = factory->channelExtension(requiredExtensionId);
//       if (!prototype)
//       {
//         qWarning() << "Failed to load extension's dependency" << requiredExtensionId;
//         Q_ASSERT(false);
//       }
// 
//       addExtension(prototype->clone());
//     }
//   }
// 
//   extension->setChannel(this);
//   m_extensions[extension->id()] = extension;
// }
// 
// //------------------------------------------------------------------------
// void Channel::deleteExtension(Channel::ExtensionPtr extension)
// {
//   ExtId id = extension->id();
//   if (m_extensions.contains(id))
//   {
//     delete m_extensions[id];
//     m_extensions.remove(id);
//   }
// }
// 
// 
// //-----------------------------------------------------------------------------
// Channel::ExtensionPtr Channel::extension(ModelItem::ExtId extensionId)
// {
//   return m_extensions.value(extensionId, NULL);
// }
// 
// //-----------------------------------------------------------------------------
// SampleSPtr Channel::sample()
// {
//   ModelItemSList relatedSamples = relatedItems(RELATION_IN, Channel::STAIN_LINK);
//   SampleSPtr sample;
// 
//   if (relatedSamples.size() > 0)
//     sample = samplePtr(relatedSamples.first());
// 
//   return sample;
// }
// 
// //-----------------------------------------------------------------------------
// ChannelPtr EspINA::channelPtr(ModelItemPtr item)
// {
//   Q_ASSERT(CHANNEL == item->type());
//   ChannelPtr ptr = dynamic_cast<ChannelPtr>(item);
//   Q_ASSERT(ptr);
// 
//   return ptr;
// }
// 
// //-----------------------------------------------------------------------------
// ChannelPtr EspINA::channelPtr(PickableItemPtr item)
// {
//   Q_ASSERT(CHANNEL == item->type());
//   ChannelPtr ptr = dynamic_cast<ChannelPtr>(item);
//   Q_ASSERT(ptr);
// 
//   return ptr;
// }
// 
// //-----------------------------------------------------------------------------
// ChannelSPtr EspINA::channelPtr(ModelItemSPtr& item)
// {
//   Q_ASSERT(CHANNEL == item->type());
//   ChannelSPtr ptr = boost::dynamic_pointer_cast<Channel>(item);
//   Q_ASSERT(ptr != NULL);
// 
//   return ptr;
// }
// 
// //-----------------------------------------------------------------------------
// ChannelSPtr EspINA::channelPtr(PickableItemSPtr& item)
// {
//   Q_ASSERT(CHANNEL == item->type());
//   ChannelSPtr ptr = boost::dynamic_pointer_cast<Channel>(item);
//   Q_ASSERT(ptr != NULL);
// 
//   return ptr;
// }
// 