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

#include "Filter.h"
#include "EspinaRegions.h"
#include "common/extensions/ChannelExtension.h"
#include "common/extensions/ModelItemExtension.h"
#include "common/model/RelationshipGraph.h"
#include "common/model/Representation.h"
#include "common/model/Sample.h"

#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkTIFFImageIO.h>

#include <vtkImageAlgorithm.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <QDebug>
#include <QFileDialog>


const ModelItem::ArgumentId Channel::ID     = "ID";
const ModelItem::ArgumentId Channel::COLOR  = "Color";
const ModelItem::ArgumentId Channel::VOLUME = "Volume";

const QString Channel::LINK       = "Channel";
const QString Channel::STAINLINK  = "Stain";
const QString Channel::VOLUMELINK = "Volume";

const QString Channel::NAME       = "Name";
const QString Channel::VOLUMETRIC = "Volumetric";

//-----------------------------------------------------------------------------
Channel::Channel(Filter* filter, OutputNumber outputNumber)
: m_visible(true)
, m_filter(filter)
{
  memset(m_pos, 0, 3*sizeof(Nm));
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
EspinaVolume *Channel::itkVolume()
{
  return m_filter->output(m_args.outputNumber());
}

//------------------------------------------------------------------------
// EspinaVolume::IndexType Channel::index(Nm x, Nm y, Nm z)
// {
//   EspinaVolume::IndexType res;
//   res[0] = x / volume()->GetSpacing()[0];
//   res[1] = y / volume()->GetSpacing()[1];
//   res[2] = z / volume()->GetSpacing()[2];
//   return res;
// }

//------------------------------------------------------------------------
void Channel::extent(int out[6])
{
  VolumeExtent(itkVolume(), out);
}

//------------------------------------------------------------------------
void Channel::bounds(double out[6])
{
  VolumeBounds(itkVolume(), out);
}

//------------------------------------------------------------------------
void Channel::spacing(double out[3])
{
  EspinaVolume::SpacingType spacing;
  spacing = itkVolume()->GetSpacing();
  for (int i=0; i<3; i++)
    out[i] = spacing[i];
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
  foreach(ModelItemExtension *ext, m_extensions)
  {
    ChannelExtension *channelExt = dynamic_cast<ChannelExtension *>(ext);
    Q_ASSERT(channelExt);
    QString serializedArgs = channelExt->serialize(); //Independizar los argumentos?
    if (!serializedArgs.isEmpty())
      extensionArgs.append(ext->id()+"=["+serializedArgs+"];");
  }
  if (!extensionArgs.isEmpty())
    m_args[EXTENSIONS] = QString("[%1]").arg(extensionArgs);

  return m_args.serialize();
}

//-----------------------------------------------------------------------------
void Channel::initialize(ModelItem::Arguments args)
{
  //qDebug() << "Init" << data().toString() << "with args:" << args;
  foreach(ArgumentId argId, args.keys())
  {
    if (argId != EXTENSIONS)
      m_args[argId] = args[argId];
  }
}

//------------------------------------------------------------------------
void Channel::initializeExtensions(ModelItem::Arguments args)
{
//   qDebug() << "Initializing" << data().toString() << "extensions:";
  foreach(ModelItemExtension *ext, m_insertionOrderedExtensions)
  {
    ChannelExtension *channelExt = dynamic_cast<ChannelExtension *>(ext);
    Q_ASSERT(channelExt);
    ModelItem::Arguments extArgs(args.value(channelExt->id(), QString()));
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

void Channel::addExtension(ChannelExtension* ext)
{
  ModelItem::addExtension(ext);
  ext->setChannel(this);
}


//-----------------------------------------------------------------------------
void Channel::notifyModification(bool force)
{
  itk2vtk->Update();
  ModelItem::notifyModification(force);
}

//-----------------------------------------------------------------------------
Sample *Channel::sample()
{
  ModelItem::Vector relatedSamples = relatedItems(ModelItem::IN, Channel::STAINLINK);
  Q_ASSERT(relatedSamples.size() == 1);
  return dynamic_cast<Sample *>(relatedSamples[0]);
}

vtkAlgorithmOutput* Channel::vtkVolume()
{
  if (itk2vtk.IsNull())
  {

    //qDebug() << " from ITK to VTK (channel)";
    itk2vtk = itk2vtkFilterType::New();
    itk2vtk->ReleaseDataFlagOn();
    itk2vtk->SetInput(m_filter->output(m_args.outputNumber()));
    itk2vtk->Update();
  }

  return itk2vtk->GetOutput()->GetProducerPort();
}
