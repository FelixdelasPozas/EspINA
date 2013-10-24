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
#include "Channel.h"

#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkTIFFImageIO.h>

#include <vtkImageAlgorithm.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <QDebug>
#include <QFileDialog>

using namespace EspINA;

const double MIN_BRIGHTNESS = -1.0;
const double MAX_BRIGHTNESS =  1.0;

const double MIN_CONTRAST = 0.0;
const double MAX_CONTRAST = 2.0;

const double MIN_HUE =  0.0;
const double MAX_HUE =  1.0;
const double NO_HUE  = -1.0;

const double MIN_OPACITY  =  0.0;
const double MAX_OPACITY  =  1.0;
const double AUTO_OPACITY = -1.0;

const double MIN_SATURATION = 0.0;
const double MAX_SATURATION = 1.0;

//------------------------------------------------------------------------
Channel::Channel(FilterSPtr filter, Output::Id output) 
: ViewItem(filter, output)
, m_brightness{0.0}
, m_contrast{1.0}
, m_hue{NO_HUE}
, m_opacity{AUTO_OPACITY}
, m_saturation{0.0}
{
}

//------------------------------------------------------------------------
Channel::~Channel()
{
}

//------------------------------------------------------------------------
void Channel::changeOutput(OutputSPtr output)
{

}

//------------------------------------------------------------------------
void Channel::restoreState(const State& state)
{

}

//------------------------------------------------------------------------
void Channel::saveState(State& state) const
{

}

//------------------------------------------------------------------------
void Channel::addExtension(ChannelExtensionSPtr extension)
{

}

void Channel::deleteExtension(ChannelExtensionSPtr extension)
{

}

ChannelExtensionSPtr Channel::extension(const ChannelExtension::Type& type)
{

}

bool Channel::hasExtension(const ChannelExtension::Type& type) const
{

}

void Channel::initializeExtensions()
{

}
void Channel::invalidateExtensions()
{

}

//------------------------------------------------------------------------
Snapshot Channel::saveSnapshot() const
{

}

//------------------------------------------------------------------------
void Channel::unload()
{

}


//------------------------------------------------------------------------
void Channel::position(Nm point[3])
{

}

//------------------------------------------------------------------------
void Channel::setPosition(Nm point[3])
{

}

//------------------------------------------------------------------------
void Channel::setHue(double hue)
{
  if (hue < MIN_HUE && hue != NO_HUE)
  {
    m_hue = MIN_HUE;
  } else if (hue > MAX_HUE) 
  {
    m_hue = MAX_HUE;
  } else 
  {
    m_hue = hue;    
  }
}

//------------------------------------------------------------------------
void Channel::setOpacity(double opacity)
{
  if (opacity < MIN_OPACITY && opacity != AUTO_OPACITY)
  {
    m_opacity = MIN_OPACITY;
  } else if (opacity > MAX_OPACITY) 
  {
    m_opacity = MAX_OPACITY;
  } else 
  {
    m_opacity = opacity;    
  }
}

//------------------------------------------------------------------------
void Channel::setSaturation(double saturation)
{
  if (saturation < MIN_SATURATION)
  {
    m_saturation = MIN_SATURATION;
  } else if (saturation > MAX_SATURATION) 
  {
    m_saturation = MAX_SATURATION;
  } else 
  {
    m_saturation = saturation;    
  }
}

//------------------------------------------------------------------------
void Channel::setContrast(double contrast)
{
  if (contrast < MIN_CONTRAST)
  {
    m_contrast = MIN_CONTRAST;
  } else if (contrast > MAX_CONTRAST) 
  {
    m_contrast = MAX_CONTRAST;
  } else 
  {
    m_contrast = contrast;    
  }
}

//------------------------------------------------------------------------
void Channel::setBrightness(double brightness)
{
  if (brightness < MIN_BRIGHTNESS)
  {
    m_brightness = MIN_BRIGHTNESS;
  } else if (brightness > MAX_BRIGHTNESS) 
  {
    m_brightness = MAX_BRIGHTNESS;
  } else 
  {
    m_brightness = brightness;    
  }
}

//------------------------------------------------------------------------
Bounds Channel::bounds() const
{

}


// static const State::Id ID;
// static const State::Id HUE;
// static const State::Id OPACITY;
// static const State::Id SATURATION;
// static const State::Id CONTRAST;
// static const State::Id BRIGHTNESS;
// static const State::Id VOLUME;
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

// NOTE: Serialization format
//     class CArguments
//     : public State
//     {
//     public:
//       explicit CArguments() : m_outputId(0) {}
//       explicit CArguments(const Arguments &args)
//       : Arguments(args), m_outputId(0) {}
// 
//       /// Channel's hue. Value in range (0,1) U (-1), latter meaning not stained
//       void setHue(double hue)
//       {
//         (*this)[HUE] = QString::number(hue);
//       }
// 
//       double hue() const
//       {
//         return (*this)[HUE].toFloat();
//       }
// 
//       /// Channel's opacity. Value in range (0,1) U (-1), latter meaning automatically managed
//       void setOpacity(double opacity)
//       {
//         (*this)[OPACITY] = QString::number(opacity);
//       }
// 
//       double opacity() const
//       {
//         return (*this)[OPACITY].toFloat();
//       }
// 
//       /// Channel's saturation. Value in range (0,1).
//       void setSaturation(double saturation)
//       {
//         (*this)[SATURATION] = QString::number(saturation);
//       }
// 
//       double saturation() const
//       {
//         return (*this)[SATURATION].toFloat();
//       }
// 
//       /// Channel's contrast. Value in range (0,2).
//       void setContrast(double contrast)
//       {
//         (*this)[CONTRAST] = QString::number(contrast);
//       }
// 
//       double contrast() const
//       {
//         return (*this)[CONTRAST].toFloat();
//       }
// 
//       /// Channel's brightness. Value in range (-1,1).
//       void setBrightness(double brightness)
//       {
//         (*this)[BRIGHTNESS] = QString::number(brightness);
//       }
// 
//       double brightness() const
//       {
//         return (*this)[BRIGHTNESS].toFloat();
//       }
// 
//       void setOutputId(FilterOutputId oId)
//       {
//         (*this)[VOLUME] = QString("%1_%2")
//         .arg(VOLUME_LINK)
//         .arg(oId);
//         m_outputId = oId;
//       }
// 
//       FilterOutputId outputId() const
//       {
//         return m_outputId;
//       }
// 
//     private:
//       FilterOutputId m_outputId;
//     };
