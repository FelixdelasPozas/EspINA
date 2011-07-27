/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

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
#include "sample.h"

// DEBUG
#include "espina_debug.h"

// ESPINA
#include "cache/cachedObjectBuilder.h"
#include "filter.h"
#include "data/hash.h"

// ParaQ
#include <pqApplicationCore.h>
#include <pqObjectBuilder.h>
#include <pqOutputPort.h>
#include <pqPipelineSource.h>

#include <vtkSMProxy.h>
#include <vtkPVDataInformation.h>
#include <vtkSMRGBALookupTableProxy.h>
#include <vtkSMProperty.h>
#include <vtkSMProxyProperty.h>
#include <vtkSMPropertyHelper.h>
#include "spatialExtension.h"


using namespace std;

//-----------------------------------------------------------------------------
Sample::~Sample()
{
  int size = m_insertionOrderedExtensions.size()-1;
  for (int i = size; i >= 0; i--)
    delete m_insertionOrderedExtensions[i];
  
  foreach(ISampleExtension *ext, m_pendingExtensions)
    delete ext;
  
  m_extensions.clear();
  m_pendingExtensions.clear();
  m_insertionOrderedExtensions.clear();
  m_representations.clear();
  m_informations.clear();
  
  CachedObjectBuilder::instance()->removeFilter(this->creator());  
}

//-----------------------------------------------------------------------------
QString Sample::getArguments()
{
  double sp[3];
  spacing(sp);
  return EspinaProduct::getArguments().append(
    ESPINA_ARG("Spacing", QString("%1,%2,%3").arg(sp[0]).arg(sp[1]).arg(sp[2]))
    );
}

//-----------------------------------------------------------------------------
QString Sample::label() const
{
  return m_creator->id().split(":")[0];
}


//------------------------------------------------------------------------
QVariant Sample::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
      return label();
    case Qt::DecorationRole:
      return QColor(Qt::blue);
    default:
      return QVariant();
  }
}


//------------------------------------------------------------------------
bool Sample::setData(const QVariant& value, int role)
{
  if (role == Qt::EditRole)
  {
    return true;
  }
  return false;
}


//------------------------------------------------------------------------
void Sample::extent( int* out)
{
  //if (!m_extent)
  //{
    mutex.lock();
    m_creator->pipelineSource()->updatePipeline();;
    m_creator->pipelineSource()->getProxy()->UpdatePropertyInformation();
    vtkPVDataInformation *info = outputPort()->getDataInformation();
    m_extent = info->GetExtent();
    mutex.unlock();
  //}
  memcpy(out,m_extent,6*sizeof(int));
}

//------------------------------------------------------------------------
void Sample::bounds( double* out)
{
  //if (!m_bounds)
  //{
    mutex.lock();
    m_creator->pipelineSource()->updatePipeline();;
    m_creator->pipelineSource()->getProxy()->UpdatePropertyInformation();
    vtkPVDataInformation *info = outputPort()->getDataInformation();
    m_bounds = info->GetBounds();
    mutex.unlock();
  //}
  memcpy(out,m_bounds,6*sizeof(double));
}

//------------------------------------------------------------------------
void Sample::spacing( double* out)
{
  ISampleRepresentation *spatialRep;
  if ((spatialRep =  representation(SpatialExtension::SampleRepresentation::ID)))
  {
    SpatialExtension::SampleRepresentation* rep = 
      dynamic_cast<SpatialExtension::SampleRepresentation*>(spatialRep);
    rep->spacing(out);
  }else
  {
    int e[6];
    double b[6];
    extent(e);
    bounds(b);
    out[0] = b[1] / e[1];
    out[1] = b[3] / e[3];
    out[2] = b[5] / e[5];
  }
//   qDebug() << "Spacing";
//   qDebug() << e[0] << e[1] << e[2] << e[3] << e[4] << e[5];
//   qDebug() << b[0] << b[1] << b[2] << b[3] << b[4] << b[5];
//   qDebug() << out[0] << out[1] << out[2];
}

//-----------------------------------------------------------------------------
void Sample::setSpacing(double x, double y, double z)
{
  SpatialExtension::SampleRepresentation* rep = 
    dynamic_cast<SpatialExtension::SampleRepresentation*>(m_representations[SpatialExtension::SampleRepresentation::ID]);
  double spacing[3];
  rep->spacing(spacing);
  if(spacing[0] != x || spacing[1] != y || spacing[2] != z)
  {
    assert(m_segs.empty());
    qDebug() << m_extensions.keys();
    rep->setSpacing(x, y, z);
  }
}

//-----------------------------------------------------------------------------
void Sample::addSegmentation(Segmentation* seg)
{
  m_segs.push_back(seg);
  foreach(ISampleRepresentation::RepresentationId rep, availableRepresentations())
  {
    representation(rep)->requestUpdate();
  }
}

//------------------------------------------------------------------------
void Sample::removeSegmentation(Segmentation* seg)
{
  m_segs.removeOne(seg);
  foreach(ISampleRepresentation::RepresentationId rep, availableRepresentations())
  {
    representation(rep)->requestUpdate();
  }
}

//------------------------------------------------------------------------
void Sample::addExtension(ISampleExtension* ext)
{
  EXTENSION_DEBUG("Added " << ext->id() << " to sample " << id());
  if (m_extensions.contains(ext->id()))
  {
     qWarning() << "Sample: Extension already registered";
     assert(false);
  }
  
  bool hasDependencies = true;
  foreach(QString reqExtId, ext->dependencies())
    hasDependencies = hasDependencies && m_extensions.contains(reqExtId);
  
  if (hasDependencies)
  {
    m_extensions.insert(ext->id(),ext);
    m_insertionOrderedExtensions.push_back(ext);
    foreach(ISampleRepresentation::RepresentationId rep, ext->availableRepresentations())
      m_representations.insert(rep, ext);
    foreach(QString info, ext->availableInformations())
    {
      m_informations.insert(info, ext);
      EXTENSION_DEBUG("New Information: " << info);
    }
    // Try to satisfy pending extensions
    foreach(ISampleExtension *pending, m_pendingExtensions)
      addExtension(pending);
  } 
  else
  {
    if (!m_pendingExtensions.contains(ext->id()))
      m_pendingExtensions.insert(ext->id(),ext);
  }
}

//------------------------------------------------------------------------
ISampleExtension* Sample::extension(ExtensionId extId)
{
  assert(m_extensions.contains(extId));
  return m_extensions[extId];
}

//------------------------------------------------------------------------
QStringList Sample::availableRepresentations()
{
  QStringList represnetations;
  foreach (ISampleExtension *ext, m_insertionOrderedExtensions)
    represnetations << ext->availableRepresentations();
  
  return represnetations;
}

//------------------------------------------------------------------------
ISampleRepresentation* Sample::representation(QString rep)
{
  if (!m_representations.contains(rep))
  {
    // Update extensions
    foreach(ISampleExtension *ext, m_insertionOrderedExtensions)
    {
      foreach(ISampleRepresentation::RepresentationId rep, ext->availableRepresentations())
	m_representations.insert(rep, ext);
    }
    if (!m_representations.contains(rep))
    {
      qWarning() << "FATAL ERROR: Representation not available after update";
      assert(false);
    }
  }
  return m_representations[rep]->representation(rep);
}

//------------------------------------------------------------------------
QStringList Sample::availableInformations()
{
  QStringList informations;
  informations << "Name";
  foreach (ISampleExtension *ext, m_insertionOrderedExtensions)
    informations << ext->availableInformations();
  
  return informations;
}

//------------------------------------------------------------------------
QVariant Sample::information(QString info)
{
  if (info == "Name")
    return data(Qt::DisplayRole);
    
  return m_informations[info]->information(info);
}

//------------------------------------------------------------------------
void Sample::initialize()
{
  foreach(ISampleExtension *ext, m_insertionOrderedExtensions)
    ext->initialize(this);
}