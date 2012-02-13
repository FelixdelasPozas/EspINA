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
#include "model/Channel.h"

#include "processing/pqData.h"
#include "processing/pqFilter.h"

#include <QDebug>

#include <pqOutputPort.h>
#include <pqPipelineSource.h>
#include <vtkPVDataInformation.h>
#include <vtkSMProxy.h>
#include <cache/CachedObjectBuilder.h>

//-----------------------------------------------------------------------------
Channel::Channel(pqData data)
: m_data(data)
, m_color(-1.0)
, m_opacity(1.0)
{
  bzero(m_bounds,6*sizeof(double));
  bzero(m_extent,6*sizeof(int));
  bzero(m_spacing,3*sizeof(double));
  m_bounds[1] = m_extent[1] = -1;
  bzero(m_pos,3*sizeof(int));

  qDebug() << "Created Channel" << m_data.id();
}

//-----------------------------------------------------------------------------
Channel::~Channel()
{
  qDebug() << "Destroyed Channel" << m_data.id();

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
void Channel::setPosition(int pos[3])
{
  memcpy(m_pos, pos, 3*sizeof(int));
}

//------------------------------------------------------------------------
void Channel::position(int pos[3])
{
  memcpy(pos, m_pos, 3*sizeof(int));
}

//------------------------------------------------------------------------
void Channel::setColor(double color)
{
  m_color = color;
}

//------------------------------------------------------------------------
double Channel::color() const
{
  return m_color;
}

//------------------------------------------------------------------------
void Channel::setOpacity(double opacity)
{
  m_opacity = opacity;
}


//------------------------------------------------------------------------
QVariant Channel::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      return m_data.id().section(':',0,-2);
//       return label();
//     case Qt::DecorationRole:
//       return QColor(Qt::blue);
    default:
      return QVariant();
  }
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