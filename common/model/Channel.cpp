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

#include "common/cache/CachedObjectBuilder.h"
#include "common/extensions/ChannelExtension.h"
#include "common/extensions/ModelItemExtension.h"
#include "common/model/Sample.h"
#include "common/model/RelationshipGraph.h"
#include "common/model/Representation.h"
#include "common/processing/pqData.h"
#include "common/processing/pqFilter.h"
#include <vtkImageAlgorithm.h>
#include <vtkDataObject.h>
#include <vtkImageData.h>

#include <QDebug>
#include <QFileDialog>

#include "EspinaTypes.h"
#include <itkImageFileReader.h>
#include <itkMetaImageIO.h>
#include <itkTIFFImageIO.h>

const ModelItem::ArgumentId Channel::ID = ArgumentId("Id", true);
const ModelItem::ArgumentId Channel::PATH = ArgumentId("Path", true);
const ModelItem::ArgumentId Channel::COLOR = ArgumentId("Color", true);
const ModelItem::ArgumentId Channel::SPACING = ArgumentId("Spacing", true);

const QString Channel::NAME   = "Name";
const QString Channel::VOLUME = "Volumetric";

//-----------------------------------------------------------------------------
// Channel::Channel(const QString file, pqData data)
// : m_data(data)
// , m_visible(true)
// {
// //   qDebug() << "Creating channel from data";
//   memset(m_bounds, 0, 6*sizeof(double));
//   memset(m_extent, 0, 6*sizeof(int));
//   memset(m_spacing, 0, 3*sizeof(double));
//   m_bounds[1] = m_extent[1] = -1;
//   memset(m_pos, 0, 3*sizeof(double));
//   m_args[ID] = Argument(File::extendedName(file));
//   m_args[PATH] = Argument(file);
//   m_args.setColor(-1.0);
//   m_isSelected = false;
// 
//   CachedObjectBuilder *cob = CachedObjectBuilder::instance();
//   pqFilter::Arguments infoArgs;
//   infoArgs << pqFilter::Argument("Input", pqFilter::Argument::INPUT, m_data.id());
//   double imgBounds[6], imgSpacing[3];
//   int imgExtent[6];
//   m_data.algorithm()->Update();
//   Q_ASSERT(false);
// //TODO:   vtkPVDataInformation *info = m_data.algorithm()->GetOutputPortInformation();
// //   info->GetExtent(imgExtent);
// //   info->GetBounds(imgBounds);
// //   imgSpacing[0] = imgBounds[1] / imgExtent[1];
// //   imgSpacing[1] = imgBounds[3] / imgExtent[3];
// //   imgSpacing[2] = imgBounds[5] / imgExtent[5];
// //   QString spacingArg = QString("%1,%2,%3").arg(imgSpacing[0]).arg(imgSpacing[1]).arg(imgSpacing[2]);
// //   infoArgs << pqFilter::Argument("Spacing", pqFilter::Argument::DOUBLEVECT, spacingArg);
// //   m_spacingFilter = cob->createFilter("filters", "InformationChanger", infoArgs);
// //   m_spacingFilter->pipelineSource()->updatePipeline();
// //   Q_ASSERT(m_spacingFilter->getNumberOfData() == 1);
// //   m_representations[VOLUME] = new Representation(m_spacingFilter->data(0));
// //   m_data = m_spacingFilter->data(0);
// }

//-----------------------------------------------------------------------------
Channel::Channel(const QFileInfo file, const Arguments args)
: m_visible(true)
, m_args(args)
{
  qDebug() << "Creating channel from args" << file.path() << args;
  memset(m_bounds, 0, 6*sizeof(double));
  memset(m_extent, 0, 6*sizeof(int));
  memset(m_pos, 0, 3*sizeof(double));

  m_args[ID] = Argument(file.fileName());
  qDebug() << m_args;
  qDebug() << m_args[PATH];
  if (QFile::exists(file.absoluteFilePath()))
  {
    m_args[PATH] = Argument(file.absoluteFilePath());
  } else if (!QFile::exists(m_args[PATH]))
  {
    QString filters; //TODO: Get previous extension
    QFileDialog fileDialog(0,
			  QString(),
			  QString(),
			  filters);
    fileDialog.setObjectName("SelectChannelFile");
    fileDialog.setFileMode(QFileDialog::ExistingFiles);
    fileDialog.setWindowTitle(QString("Select file for %1:").arg(m_args[ID]));

    Q_ASSERT(fileDialog.exec() == QDialog::Accepted);

    m_args[PATH] = fileDialog.selectedFiles().first();
  }

  double spacing[3];
  m_args.spacing(spacing);

  typedef itk::ImageFileReader<EspinaVolume> EspinaVolumeReader;
  reader = EspinaVolumeReader::New();

  QString ext = file.suffix();
  if ("mhd" == ext || "mha" == ext)
    io = itk::MetaImageIO::New();
  else if ("tif" == ext || "tiff" == ext)
    io = itk::TIFFImageIO::New();
  else
  {
    qWarning() << QString("Cached Object Builder: Couldn't load file %1."
    "Extension not supported")
    .arg(file.absoluteFilePath());
    Q_ASSERT(false);
  }
  io->SetFileName(m_args[PATH].toAscii());
  if (spacing[0] > 0)
  {
    for(int i=0; i<3; i++)
      io->SetSpacing(i, spacing[i]);
  }
  reader->SetImageIO(io);
  reader->SetFileName(m_args[PATH].toStdString());
  reader->Update();
  for(int i=0; i<3; i++)
    spacing[i] = io->GetSpacing(i);
  m_args.setSpacing(spacing);

  m_volume = reader->GetOutput();
  EspinaVolume::RegionType region = m_volume->GetRequestedRegion();
  for(int i=0; i<3; i++)
  {
    int min = 2*i, max = 2*i+1;
    m_extent[min] = region.GetIndex(i);
    m_extent[max] = m_extent[min] + region.GetSize(i) - 1;
    m_bounds[min] = m_extent[min]*spacing[i];
    m_bounds[max] = m_extent[max]*spacing[i];
  }

  qDebug() << "Converting from ITK to VTK (channel)";
  itk2vtk = itk2vtkFilterType::New();
  itk2vtk->ReleaseDataFlagOn();
  itk2vtk->SetInput(m_volume);
  itk2vtk->Update();
}

//-----------------------------------------------------------------------------
Channel::~Channel()
{
  m_volume->Delete();
}


//------------------------------------------------------------------------
EspinaVolume *Channel::volume()
{
  return m_volume;
}


//------------------------------------------------------------------------
void Channel::extent(int *out)
{
  memcpy(out,m_extent,6*sizeof(int));
}

void Channel::bounds(double val[3])
//------------------------------------------------------------------------
{
  memcpy(val,m_bounds,6*sizeof(double));
}

//------------------------------------------------------------------------
void Channel::spacing(double val[3])
{
  m_args.spacing(val);
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
  ModelItem::Vector relatedSamples = relatedItems(ModelItem::IN, "mark");
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
    return itk2vtk->GetOutput()->GetProducerPort();
}
