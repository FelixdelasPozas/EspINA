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

#include <QDebug>

#include <pqOutputPort.h>
#include <pqPipelineSource.h>
#include <pqFileDialog.h>
#include <vtkPVDataInformation.h>
#include <vtkSMProxy.h>
#include <vtkSMReaderFactory.h>
#include <vtkSMProxyManager.h>
#include <pqActiveObjects.h>


const ModelItem::ArgumentId Channel::ID = ArgumentId("Id", true);
const ModelItem::ArgumentId Channel::PATH = ArgumentId("Path", true);
const ModelItem::ArgumentId Channel::COLOR = ArgumentId("Color", true);

const QString Channel::NAME   = "Name";
const QString Channel::VOLUME = "Volumetric";

//-----------------------------------------------------------------------------
Channel::Channel(const QString file, pqData data)
: m_data(data)
, m_visible(true)
{
//   qDebug() << "Creating channel from data";
  memset(m_bounds, 0, 6*sizeof(double));
  memset(m_extent, 0, 6*sizeof(int));
  memset(m_spacing, 0, 3*sizeof(double));
  m_bounds[1] = m_extent[1] = -1;
  memset(m_pos, 0, 3*sizeof(double));
  m_args[ID] = Argument(File::extendedName(file));
  m_args[PATH] = Argument(file);
  m_args.setColor(-1.0);
  m_isSelected = false;

  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  pqFilter::Arguments infoArgs;
  infoArgs << pqFilter::Argument("Input", pqFilter::Argument::INPUT, m_data.id());
  double imgBounds[6], imgSpacing[3];
  int imgExtent[6];
  m_data.pipelineSource()->updatePipeline();
  vtkPVDataInformation *info = outputPort()->getDataInformation();
  info->GetExtent(imgExtent);
  info->GetBounds(imgBounds);
  imgSpacing[0] = imgBounds[1] / imgExtent[1];
  imgSpacing[1] = imgBounds[3] / imgExtent[3];
  imgSpacing[2] = imgBounds[5] / imgExtent[5];
  QString spacingArg = QString("%1,%2,%3").arg(imgSpacing[0]).arg(imgSpacing[1]).arg(imgSpacing[2]);
  infoArgs << pqFilter::Argument("Spacing", pqFilter::Argument::DOUBLEVECT, spacingArg);
  m_spacingFilter = cob->createFilter("filters", "InformationChanger", infoArgs);
  m_spacingFilter->pipelineSource()->updatePipeline();
  Q_ASSERT(m_spacingFilter->getNumberOfData() == 1);
  m_representations[VOLUME] = new Representation(m_spacingFilter->data(0));
  m_data = m_spacingFilter->data(0);
}

//-----------------------------------------------------------------------------
Channel::Channel(const QString file, const Arguments args)
: m_visible(true) //TODO: Should be persistent?
, m_args(args)
{
  //qDebug() << "Creating channel from args" << file << args;
  memset(m_bounds, 0, 6*sizeof(double));
  memset(m_extent, 0, 6*sizeof(int));
  memset(m_spacing, 0, 3*sizeof(double));
  m_bounds[1] = m_extent[1] = -1;
  memset(m_pos, 0, 3*sizeof(double));

//   QStringList input = m_args[ID].split(":");
  m_args[ID] = Argument(File::extendedName(file));
  if (QFile::exists(file))
  {
    m_args[PATH] = Argument(file);
  } else if (!QFile::exists(m_args[PATH]))
  {
    pqServer *server = pqActiveObjects::instance().activeServer();
    vtkSMReaderFactory *readerFactory =
    vtkSMProxyManager::GetProxyManager()->GetReaderFactory();
    QString filters = readerFactory->GetSupportedFileTypes(server->session());
    filters.replace("Meta Image Files", "Channel Files");
    pqFileDialog fileDialog(server,
			    0,
			    QString(), QString(), filters);
    fileDialog.setObjectName("SelectChannelFile");
    fileDialog.setFileMode(pqFileDialog::ExistingFiles);
    fileDialog.setWindowTitle(QString("Select file for %1:").arg(m_args[ID]));

    Q_ASSERT(fileDialog.exec() == QDialog::Accepted);

    m_args[PATH] = fileDialog.getSelectedFiles().first();
  }

  qDebug() << m_args[PATH];
  int port = 0;//input.last().toInt();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  pqFilter *channelReader = cob->loadFile(m_args[PATH]);
  Q_ASSERT(port < channelReader->getNumberOfData());
  m_data = pqData(channelReader, port);

  pqFilter::Arguments infoArgs;
  infoArgs << pqFilter::Argument("Input", pqFilter::Argument::INPUT, m_data.id());
  double imgBounds[6], imgSpacing[3];
  int imgExtent[6];
  m_data.pipelineSource()->updatePipeline();
  vtkPVDataInformation *info = outputPort()->getDataInformation();
  info->GetExtent(imgExtent);
  info->GetBounds(imgBounds);
  imgSpacing[0] = imgBounds[1] / imgExtent[1];
  imgSpacing[1] = imgBounds[3] / imgExtent[3];
  imgSpacing[2] = imgBounds[5] / imgExtent[5];
  QString spacingArg = QString("%1,%2,%3").arg(imgSpacing[0]).arg(imgSpacing[1]).arg(imgSpacing[2]);
  infoArgs << pqFilter::Argument("Spacing", pqFilter::Argument::DOUBLEVECT, spacingArg);
  m_spacingFilter = cob->createFilter("filters", "InformationChanger", infoArgs);
  m_spacingFilter->pipelineSource()->updatePipeline();
  Q_ASSERT(m_spacingFilter->getNumberOfData() == 1);
  m_representations[VOLUME] = new Representation(m_spacingFilter->data(0));
  m_data = m_spacingFilter->data(0);
}

//-----------------------------------------------------------------------------
Channel::~Channel()
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  cob->removeFilter(m_data.source());
/*  int size = m_insertionOrderedExtensions.size()-1;
  for (int i = size; i >= 0; i--)
    delete m_insertionOrderedExtensions[i];

  foreach(IChannelExtension *ext, m_pendingExtensions)
    delete ext;

  m_extensions.clear();
  m_pendingExtensions.clear();
  m_insertionOrderedExtensions.clear();
  m_representations.clear();
  m_informations.clear();

  CachedObjectBuilder::instance()->removeFilter(this->creator()); */ 
}

//-----------------------------------------------------------------------------
// QString Channel::getArguments() const
// {
//   double sp[3];
//   Channel *nonconst = const_cast<Channel *>(this); //Cast away constness
//   nonconst->spacing(sp);
//   QString args = EspinaProduct::getArguments();
//   args.append(
//     ESPINA_ARG("Spacing", QString("%1,%2,%3")
//       .arg(sp[0]).arg(sp[1]).arg(sp[2]))
//   );
//   
//   foreach(IChannelExtension *ext, m_extensions)
//   {
//     QString extArgs = ext->getArguments();
//     if (!extArgs.isEmpty())
//       args.append(ESPINA_ARG(ext->id(),"["+extArgs+"]"));
//   }
//   
//   return args;
// }

//-----------------------------------------------------------------------------
// QString Channel::label() const
// {
//   return m_path + "/" + m_creator->id().split(":")[0];
// }



//------------------------------------------------------------------------
// bool Channel::setData(const QVariant& value, int role)
// {
//   if (role == Qt::EditRole)
//   {
//     return true;
//   }
//   return false;
// }

//------------------------------------------------------------------------
pqOutputPort* Channel::outputPort()
{
  return m_data.outputPort();
}


//------------------------------------------------------------------------
void Channel::extent( int* out)
{
  if (m_extent[1] < m_extent[0])
  {
    m_data.pipelineSource()->updatePipeline();
    vtkPVDataInformation *info = outputPort()->getDataInformation();
    info->GetExtent(m_extent);
  }
  memcpy(out,m_extent,6*sizeof(int));
}

void Channel::bounds(double val[3])
//------------------------------------------------------------------------
{
  if (m_bounds[1] < m_bounds[0])
  {
    m_data.pipelineSource()->updatePipeline();
    vtkPVDataInformation *info = outputPort()->getDataInformation();
    info->GetBounds(m_bounds);
  }
  memcpy(val,m_bounds,6*sizeof(double));
}

//------------------------------------------------------------------------
void Channel::spacing(double val[3])
{
/*   IChannelRepresentation *spatialRep;
//   if ((spatialRep =  representation(SpatialExtension::ChannelRepresentation::ID)))
//   {
//     SpatialExtension::ChannelRepresentation* rep =
//       dynamic_cast<SpatialExtension::ChannelRepresentation*>(spatialRep);
//     rep->spacing(out);
//   }else
//   {
//   int e[6];
//   double b[6];
*/
  if (m_extent[1] < m_extent[0])
    extent(m_extent);
  if (m_bounds[1] < m_bounds[0])
    bounds(m_bounds);

  m_spacing[0] = m_bounds[1] / m_extent[1];
  m_spacing[1] = m_bounds[3] / m_extent[3];
  m_spacing[2] = m_bounds[5] / m_extent[5];

  memcpy(val,m_spacing,3*sizeof(double));
//   }
//   qDebug() << "Spacing";
//   qDebug() << e[0] << e[1] << e[2] << e[3] << e[4] << e[5];
//   qDebug() << b[0] << b[1] << b[2] << b[3] << b[4] << b[5];
//   qDebug() << val[0] << val[1] << val[2];
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
//     case Qt::DecorationRole:
//       return QColor(Qt::blue);
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


// //-----------------------------------------------------------------------------
// void Channel::setSpacing(double x, double y, double z)
// {
//   SpatialExtension::ChannelRepresentation* rep =
//     dynamic_cast<SpatialExtension::ChannelRepresentation*>(representation(SpatialExtension::ChannelRepresentation::ID));
//   double spacing[3];
//   rep->spacing(spacing);
//   if(spacing[0] != x || spacing[1] != y || spacing[2] != z)
//   {
//     assert(m_segs.empty());
//     qDebug() << m_extensions.keys();
//     rep->setSpacing(x, y, z);
//   }
// }
// 
// //-----------------------------------------------------------------------------
// void Channel::addSegmentation(Segmentation* seg)
// {
//   m_segs.push_back(seg);
//   foreach(IChannelRepresentation::RepresentationId rep, availableRepresentations())
//   {
//     representation(rep)->requestUpdate();
//   }
// }
// 
// //------------------------------------------------------------------------
// void Channel::removeSegmentation(Segmentation* seg)
// {
//   m_segs.removeOne(seg);
//   foreach(IChannelRepresentation::RepresentationId rep, availableRepresentations())
//   {
//     representation(rep)->requestUpdate();
//   }
// }
// 
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