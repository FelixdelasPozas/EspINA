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
#include "segmentation.h"

// Debug
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
#include "sample.h"


using namespace std;

int Segmentation::s_newId = 1;

//-----------------------------------------------------------------------------
Segmentation::Segmentation(EspinaFilter* parent, vtkFilter* creator, int portNumber)
: EspinaProduct(parent,creator, portNumber)
{
  m_id = s_newId++;
}

//------------------------------------------------------------------------
Segmentation::~Segmentation()
{
  int size = m_insertionOrderedExtensions.size()-1;
  for (int i = size; i >= 0; i--)
    delete m_insertionOrderedExtensions[i];
  
  foreach(ISegmentationExtension *ext, m_pendingExtensions)
    delete ext;
  
  m_extensions.clear();
  m_pendingExtensions.clear();
  m_insertionOrderedExtensions.clear();
  m_representations.clear();
  m_informations.clear();
}

//------------------------------------------------------------------------
QVariant Segmentation::data(int role) const
{
  switch (role)
  {
    case Qt::DisplayRole:
    //case Qt::EditRole:
      return label();
    case Qt::DecorationRole:
    {
      QPixmap segIcon(3,16);
      segIcon.fill(m_taxonomy->getColor());
      return segIcon;
    }
    case Qt::ToolTipRole:
      return QString(
	"<b>Name:</b> %1<br>"
	"<b>Taxonomy:</b> %2<br>"
	"<b>Sample:</b> %3<br>"
	"<b>Created by:</b><br>"
	"%4"
      )
      .arg(label())
      .arg(m_taxonomy->getName())
      .arg(origin()->label())
      .arg(m_parent->getFilterArguments())
      ;
    case Qt::CheckStateRole:
      return visible()?Qt::Checked:Qt::Unchecked;
    default:
      return QVariant();
  }
}

//------------------------------------------------------------------------
void Segmentation::color(double* rgba)
{
  QColor color = m_taxonomy->getColor();
  rgba[0] = color.red()/255.0;
  rgba[1] = color.green()/255.0;
  rgba[2] = color.blue()/255.0;
  rgba[3] = 1;
}


//------------------------------------------------------------------------
bool Segmentation::setData(const QVariant& value, int role)
{
  switch (role)
  {
    case Qt::EditRole:
      return true;
    case Qt::CheckStateRole:
      setVisible(value.toBool());
      return true;
    //case Qt::DecorationRole:
      //m_repMap["01_Color"]->requestUpdate();
      //return true;
    default:
      return false;
  }
}

//------------------------------------------------------------------------
void Segmentation::addExtension(ISegmentationExtension* ext)
{
  if (m_extensions.contains(ext->id()))
  {
     qWarning() << "Segmentation: Extension already registered";
     assert(false);
  }
  
  bool hasDependencies = true;
  foreach(QString reqExtId, ext->dependencies())
    hasDependencies = hasDependencies && m_extensions.contains(reqExtId);
  
  if (hasDependencies)
  {
    m_extensions.insert(ext->id(),ext);
    m_insertionOrderedExtensions.push_back(ext);
    foreach(ISegmentationRepresentation::RepresentationId rep, ext->availableRepresentations())
      m_representations.insert(rep, ext);
    foreach(QString info, ext->availableInformations())
    {
      m_informations.insert(info, ext);
      EXTENSION_DEBUG("New Information: " << info);
    }
    // Try to satisfy pending extensions
    foreach(ISegmentationExtension *pending, m_pendingExtensions)
      addExtension(pending);
  } 
  else
  {
    if (!m_pendingExtensions.contains(ext->id()))
      m_pendingExtensions.insert(ext->id(),ext);
  }
}

//------------------------------------------------------------------------
ISegmentationExtension *Segmentation::extension(ExtensionId extId)
{
  assert(m_extensions.contains(extId));
  return m_extensions[extId];
}

//------------------------------------------------------------------------
QStringList Segmentation::availableRepresentations()
{
  QStringList represnetations;
  foreach (ISegmentationExtension *ext, m_insertionOrderedExtensions)
    represnetations << ext->availableRepresentations();
  
  return represnetations;
}

//------------------------------------------------------------------------
ISegmentationRepresentation* Segmentation::representation(QString rep)
{
  return m_representations[rep]->representation(rep);
}

//------------------------------------------------------------------------
QStringList Segmentation::availableInformations()
{
  QStringList informations;
  informations << "Name" << "Taxonomy";
  foreach (ISegmentationExtension *ext, m_insertionOrderedExtensions)
    informations << ext->availableInformations();
  
  return informations;
}

//------------------------------------------------------------------------
QVariant Segmentation::information(QString info)
{
  if (info == "Name")
    return data(Qt::DisplayRole);
  if (info == "Taxonomy")
    return m_taxonomy->getName();
    
  return m_informations[info]->information(info);
}

//------------------------------------------------------------------------
//! TODO: Review where extensions should be initialized: at creation
//! or when adding them to EspINA
void Segmentation::initialize()
{
  foreach(ISegmentationExtension *ext, m_extensions)
    ext->initialize(this);
}

//------------------------------------------------------------------------
void Segmentation::notifyInternalUpdate()
{
  emit updated(this);
}
