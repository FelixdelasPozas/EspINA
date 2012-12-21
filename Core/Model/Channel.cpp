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

#include "Core/Model/Filter.h"
#include "Core/Extensions/ChannelExtension.h"
#include "Core/Extensions/ModelItemExtension.h"
#include "Core/Model/RelationshipGraph.h"
#include "Core/Model/Sample.h"

#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkTIFFImageIO.h>

#include <vtkImageAlgorithm.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <QDebug>
#include <QFileDialog>

using namespace EspINA;

const ModelItem::ArgumentId Channel::ID     = "ID";
const ModelItem::ArgumentId Channel::COLOR  = "Color";
const ModelItem::ArgumentId Channel::VOLUME = "Volume";

const QString Channel::LINK       = "Channel";
const QString Channel::STAINLINK  = "Stain";
const QString Channel::VOLUMELINK = "Volume";

const QString Channel::NAME       = "Name";
const QString Channel::VOLUMETRIC = "Volumetric";

//-----------------------------------------------------------------------------
Channel::Channel(FilterSPtr filter, Filter::OutputId oId)
: m_visible(true)
, m_filter(filter)
{
  memset(m_pos, 0, 3*sizeof(Nm));
  m_args.setOutputId(oId);
}

//-----------------------------------------------------------------------------
Channel::~Channel()
{
  qDebug() << data().toString() << ": Destructor";
  // Extensions may need access channel's information
  deleteExtensions();
}

//------------------------------------------------------------------------
const FilterSPtr Channel::filter() const
{
  return m_filter;
}

//------------------------------------------------------------------------
const Filter::OutputId Channel::outputId() const
{
  return m_args.outputId();
}

//------------------------------------------------------------------------
ChannelVolume::Pointer Channel::volume()
{
  EspinaVolume::Pointer ev = m_filter->volume(m_args.outputId());

  // On invalid cast:
  // The static_pointer_cast will "just do it". This will result in an invalid pointer and will likely cause a crash. The reference count on base will be incremented.
  // The shared_polymorphic_downcast will come the same as a static cast, but will trigger an assertion in the process. The reference count on base will be incremented.
  // The dynamic_pointer_cast will simply come out NULL. The reference count on base will be unchanged.
  return boost::dynamic_pointer_cast<ChannelVolume>(ev);
}

//------------------------------------------------------------------------
const ChannelVolume::Pointer Channel::volume() const
{
  EspinaVolume::Pointer ev = m_filter->volume(m_args.outputId());

  // On invalid cast:
  // The static_pointer_cast will "just do it". This will result in an invalid pointer and will likely cause a crash. The reference count on base will be incremented.
  // The shared_polymorphic_downcast will come the same as a static cast, but will trigger an assertion in the process. The reference count on base will be incremented.
  // The dynamic_pointer_cast will simply come out NULL. The reference count on base will be unchanged.
  return boost::dynamic_pointer_cast<ChannelVolume>(ev);
}


//------------------------------------------------------------------------
void Channel::setPosition(Nm pos[3])
{
  memcpy(m_pos, pos, 3*sizeof(Nm));
}

//------------------------------------------------------------------------
void Channel::position(Nm pos[3])
{
  memcpy(pos, m_pos, 3*sizeof(Nm));
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
  foreach(ModelItemExtensionPtr ext, m_extensions)
  {
    ChannelExtensionPtr channelExt = qSharedPointerDynamicCast<ChannelExtension>(ext);
    Q_ASSERT(!channelExt.isNull());
    QString serializedArgs = channelExt->serialize(); //Independizar los argumentos?
    if (!serializedArgs.isEmpty())
      extensionArgs.append(ext->id()+"=["+serializedArgs+"];");
  }
  if (!extensionArgs.isEmpty())
    m_args[EXTENSIONS] = QString("[%1]").arg(extensionArgs);

  return m_args.serialize();
}

//-----------------------------------------------------------------------------
void Channel::initialize(const Arguments &args)
{
  //qDebug() << "Init" << data().toString() << "with args:" << args;
  foreach(ArgumentId argId, args.keys())
  {
    if (argId != EXTENSIONS)
      m_args[argId] = args[argId];
  }
}

//------------------------------------------------------------------------
void Channel::initializeExtensions(const Arguments &args)
{
//   qDebug() << "Initializing" << data().toString() << "extensions:";
  foreach(ModelItemExtensionPtr ext, m_insertionOrderedExtensions)
  {
    ChannelExtensionPtr channelExt = qSharedPointerCast<ChannelExtension>(ext);
    Q_ASSERT(!channelExt.isNull());

    Arguments extArgs(args.value(channelExt->id(), QString()));
//     qDebug() << channelExt->id();
//     if (!args.isEmpty()) qDebug() << "*" << extArgs;
    channelExt->initialize(extArgs);
  }

}

//------------------------------------------------------------------------
QVariant Channel::information(QString name)
{
  if (name == NAME)
    return data(Qt::DisplayRole);

  return ModelItem::information(name);
}

//------------------------------------------------------------------------
void Channel::addExtension(ChannelExtensionPtr ext)
{
  ModelItem::addExtension(ext);
  ext->setChannel(this);
}


//-----------------------------------------------------------------------------
void Channel::notifyModification(bool force)
{
  m_filter->volume(outputId())->update();
  ModelItem::notifyModification(force);
}

//-----------------------------------------------------------------------------
SampleSPtr Channel::sample()
{
  SharedModelItemList relatedSamples = relatedItems(IN, Channel::STAINLINK);
  SampleSPtr sample;

  if (relatedSamples.size() > 0)
    sample = samplePtr(relatedSamples.first());

  return sample;
}

//TODO 2012-11-28 vtkAlgorithmOutput* Channel::vtkVolume()
// {
//   if (itk2vtk.IsNull())
//   {
// 
//     //qDebug() << " from ITK to VTK (channel)";
//     itk2vtk = itk2vtkFilterType::New();
//     itk2vtk->ReleaseDataFlagOn();
//     itk2vtk->SetInput(m_filter->volume(m_args.outputId()));
//     itk2vtk->Update();
//   }
// 
//   return itk2vtk->GetOutput()->GetProducerPort();
// }
// 

//-----------------------------------------------------------------------------
ChannelPtr EspINA::channelPtr(ModelItemPtr item)
{
  Q_ASSERT(CHANNEL == item->type());
  ChannelPtr ptr = dynamic_cast<ChannelPtr>(item);
  Q_ASSERT(ptr);

  return ptr;
}

//-----------------------------------------------------------------------------
ChannelPtr EspINA::channelPtr(PickableItemPtr item)
{
  Q_ASSERT(CHANNEL == item->type());
  ChannelPtr ptr = dynamic_cast<ChannelPtr>(item);
  Q_ASSERT(ptr);

  return ptr;
}

//-----------------------------------------------------------------------------
SharedChannelPtr EspINA::channelPtr(SharedModelItemPtr& item)
{
  Q_ASSERT(CHANNEL == item->type());
  SharedChannelPtr ptr = qSharedPointerDynamicCast<Channel>(item);
  Q_ASSERT(!ptr.isNull());

  return ptr;
}

//-----------------------------------------------------------------------------
SharedChannelPtr EspINA::channelPtr(SharedPickableItemPtr& item)
{
  Q_ASSERT(CHANNEL == item->type());
  SharedChannelPtr ptr = qSharedPointerDynamicCast<Channel>(item);
  Q_ASSERT(!ptr.isNull());

  return ptr;
}
