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
#include "common/model/Channel.h"

#include "common/extensions/ChannelExtension.h"
#include "common/extensions/ModelItemExtension.h"
#include "common/model/Sample.h"
#include "common/model/RelationshipGraph.h"
#include "common/model/Representation.h"
#include "Filter.h"
#include <vtkImageAlgorithm.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <QDebug>
#include <QFileDialog>

#include "EspinaTypes.h"
#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkTIFFImageIO.h>

const ModelItem::ArgumentId Channel::ID     = ArgumentId("ID",     true);
const ModelItem::ArgumentId Channel::COLOR  = ArgumentId("Color",  true);
const ModelItem::ArgumentId Channel::VOLUME = ArgumentId("Volume", true);

const QString Channel::STAINLINK  = "stain";
const QString Channel::VOLUMELINK = "volume";

const QString Channel::NAME       = "Name";
const QString Channel::VOLUMETRIC = "Volumetric";

//-----------------------------------------------------------------------------
Channel::Channel(Filter* filter, OutputNumber outputNumber)
: m_visible(true)
, m_filter(filter)
{
  m_args.setOutputNumber(outputNumber);
}

//-----------------------------------------------------------------------------
Channel::~Channel()
{
}

//------------------------------------------------------------------------
Filter* Channel::filter()
{
  return m_filter;
}

//------------------------------------------------------------------------
OutputNumber Channel::outputNumber()
{
  return m_args.outputNumber();
}

//------------------------------------------------------------------------
EspinaVolume *Channel::volume()
{
  return m_filter->output(m_args.outputNumber());
}

//------------------------------------------------------------------------
void Channel::extent(int out[6])
{
  VolumeExtent(volume(), out);
  //memcpy(out,m_extent,6*sizeof(int));
}

//------------------------------------------------------------------------
void Channel::bounds(double out[6])
{
  VolumeBounds(volume(), out);
  //memcpy(val,m_bounds,6*sizeof(double));
}

//------------------------------------------------------------------------
void Channel::spacing(double out[3])
{
  EspinaVolume::SpacingType spacing;
  spacing = volume()->GetSpacing();
  for (int i=0; i<3; i++)
    out[i] = spacing[i];
}

//------------------------------------------------------------------------
void Channel::setPosition(double pos[3])
{
  memcpy(m_pos, pos, 3*sizeof(double));
}

//------------------------------------------------------------------------
void Channel::position(double pos[3])
{
  memcpy(pos, m_pos, 3*sizeof(double));
}

//------------------------------------------------------------------------
void Channel::setColor(const double color)
{
  m_args.setColor(color);
}

//------------------------------------------------------------------------
double Channel::color() const
{
  return m_args.color();
}

//------------------------------------------------------------------------
QStringList Channel::availableInformations() const
{
  QStringList informations;
  informations << NAME;
  informations << ModelItem::availableInformations();

  return informations;
}

//------------------------------------------------------------------------
QStringList Channel::availableRepresentations() const
{
  QStringList representations;
  representations << VOLUME;
  representations << ModelItem::availableInformations();

  return representations;
}

//------------------------------------------------------------------------
QVariant Channel::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      return m_args[ID];
    case Qt::CheckStateRole:
      return m_visible?Qt::Checked: Qt::Unchecked;
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
bool Channel::setData(const QVariant& value, int role)
{
  switch (role)
  {
    case Qt::EditRole:
      return true;
    case Qt::CheckStateRole:
      setVisible(value.toBool());
      return true;
    default:
      return false;
  }
}

//------------------------------------------------------------------------
QString Channel::serialize() const
{
  QString extensionArgs;
  foreach(ModelItemExtension *ext, m_extensions)
  {
    ChannelExtension *channelExt = dynamic_cast<ChannelExtension *>(ext);
    Q_ASSERT(channelExt);
    QString serializedArgs = channelExt->serialize(); //Independizar los argumentos?
    if (!serializedArgs.isEmpty())
      extensionArgs.append(ext->id()+"=["+serializedArgs+"];");
  }
  if (!extensionArgs.isEmpty())
  {
    m_args[EXTENSIONS] = QString("[%1]").arg(extensionArgs);
  }
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
void Channel::initialize(ModelItem::Arguments args)
{
  qDebug() << "Init channel with args:" << args;
  foreach(ArgumentId argId, args.keys())
  {
    if (argId != EXTENSIONS)
    {
      m_args[argId] = args[argId];
    }
  }
  ModelItem::Arguments extArgs(args[EXTENSIONS]);
  foreach(ModelItemExtension *ext, m_extensions)
  {
    ChannelExtension *channelExt = dynamic_cast<ChannelExtension *>(ext);
    Q_ASSERT(channelExt);
//     qDebug() << extArgs;
    ArgumentId argId(channelExt->id(), false);
    ModelItem::Arguments cArgs(extArgs.value(argId, QString()));
    channelExt->initialize(this, cArgs);
  }
}

//------------------------------------------------------------------------
QVariant Channel::information(QString name)
{
  if (name == NAME)
    return data(Qt::DisplayRole);

  return ModelItem::information(name);
}

void Channel::addExtension(ChannelExtension* ext)
{
  ModelItem::addExtension(ext);
}


//-----------------------------------------------------------------------------
Sample *Channel::sample()
{
  ModelItem::Vector relatedSamples = relatedItems(ModelItem::IN, Channel::STAINLINK);
  Q_ASSERT(relatedSamples.size() == 1);
  return dynamic_cast<Sample *>(relatedSamples[0]);
}


// //------------------------------------------------------------------------
// void Channel::addExtension(IChannelExtension* ext)
// {
//   EXTENSION_DEBUG("Added " << ext->id() << " to sample " << id());
//   if (m_extensions.contains(ext->id()))
//   {
//      qWarning() << "Channel: Extension already registered";
//      assert(false);
//   }
//   
//   bool hasDependencies = true;
//   foreach(QString reqExtId, ext->dependencies())
//     hasDependencies = hasDependencies && m_extensions.contains(reqExtId);
//   
//   if (hasDependencies)
//   {
//     m_extensions.insert(ext->id(),ext);
//     m_insertionOrderedExtensions.push_back(ext);
//     foreach(IChannelRepresentation::RepresentationId rep, ext->availableRepresentations())
//       m_representations.insert(rep, ext);
//     foreach(QString info, ext->availableInformations())
//     {
//       m_informations.insert(info, ext);
//       EXTENSION_DEBUG("New Information: " << info);
//     }
//     // Try to satisfy pending extensions
//     foreach(IChannelExtension *pending, m_pendingExtensions)
//       addExtension(pending);
//   } 
//   else
//   {
//     if (!m_pendingExtensions.contains(ext->id()))
//       m_pendingExtensions.insert(ext->id(),ext);
//   }
// }
// 
// //------------------------------------------------------------------------
// IChannelExtension* Channel::extension(ExtensionId extId)
// {
//   if (m_extensions.contains(extId))
//     return m_extensions[extId];
//   else
//     return NULL;
// }
// 
// //------------------------------------------------------------------------
// QStringList Channel::availableRepresentations()
// {
//   QStringList represnetations;
//   foreach (IChannelExtension *ext, m_insertionOrderedExtensions)
//     represnetations << ext->availableRepresentations();
//   
//   return represnetations;
// }
// 
// //------------------------------------------------------------------------
// IChannelRepresentation* Channel::representation(QString rep)
// {
//   if (!m_representations.contains(rep))
//   {
//     // Update extensions
//     foreach(IChannelExtension *ext, m_insertionOrderedExtensions)
//     {
//       foreach(IChannelRepresentation::RepresentationId rep, ext->availableRepresentations())
// 	m_representations.insert(rep, ext);
//     }
//     if (!m_representations.contains(rep))
//     {
//       qWarning() << "FATAL ERROR: Representation not available after update";
//       assert(false);
//     }
//   }
//   return m_representations[rep]->representation(rep);
// }
// 
// //------------------------------------------------------------------------
// QStringList Channel::availableInformations()
// {
//   QStringList informations;
//   informations << "Name";
//   foreach (IChannelExtension *ext, m_insertionOrderedExtensions)
//     informations << ext->availableInformations();
//   
//   return informations;
// }
// 
// //------------------------------------------------------------------------
// QVariant Channel::information(QString info)
// {
//   if (info == "Name")
//     return data(Qt::DisplayRole);
//     
//   return m_informations[info]->information(info);
// }
// 
// //------------------------------------------------------------------------
// void Channel::initialize()
// {
//   foreach(IChannelExtension *ext, m_insertionOrderedExtensions)
//     ext->initialize(this);
// }

vtkAlgorithmOutput* Channel::image()
{
  if (itk2vtk.IsNull())
  {

    qDebug() << "Converting from ITK to VTK (channel)";
    itk2vtk = itk2vtkFilterType::New();
    itk2vtk->ReleaseDataFlagOn();
    itk2vtk->SetInput(m_filter->output(m_args.outputNumber()));
    itk2vtk->Update();
  }

  return itk2vtk->GetOutput()->GetProducerPort();
}
