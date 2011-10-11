/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "SegmentationSelectionExtension.h"

// Debug
#include "espina_debug.h"

// EspINA
#include "cachedObjectBuilder.h"
#include "segmentation.h"

#include <vtkSMPropertyHelper.h>
#include <pqPipelineSource.h>
#include <vtkSMProxy.h>

const ExtensionId SegmentationSelectionExtension::ID = "SegmentationSelectionExtension";


//-----------------------------------------------------------------------------
SegmentationSelectionExtension::SegmentationSelectionExtension()
: m_selection(NULL)
{

}

//-----------------------------------------------------------------------------
SegmentationSelectionExtension::~SegmentationSelectionExtension()
{
  if (m_selection)
  {
    EXTENSION_DEBUG("Deleted " << ID << " Extension from " << m_seg->id());
    CachedObjectBuilder *cob = CachedObjectBuilder::instance();
    cob->removeFilter(m_selection);
    m_selection = NULL;
  }
}

//-----------------------------------------------------------------------------
ExtensionId SegmentationSelectionExtension::id()
{
  return ID;
}

//-----------------------------------------------------------------------------
void SegmentationSelectionExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  vtkFilter::Arguments selectionArgs;
  selectionArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_seg->id()));
  m_selection = cob->createFilter("filters","SegmentationSelection", selectionArgs);
  assert(m_selection);
}

//-----------------------------------------------------------------------------
ISegmentationRepresentation* SegmentationSelectionExtension::representation(QString rep)
{
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//-----------------------------------------------------------------------------
QVariant SegmentationSelectionExtension::information(QString info)
{
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//-----------------------------------------------------------------------------
bool SegmentationSelectionExtension::isSegmentationPixel(int x, int y, int z)
{
  int pixel[3] = {x,y,z};
  
  return isSegmentationPixel(pixel);
}

//-----------------------------------------------------------------------------
bool SegmentationSelectionExtension::isSegmentationPixel(int pixel[3])
{
  vtkSMPropertyHelper(m_selection->pipelineSource()->getProxy(),"CheckPixel").Set(pixel,3);
  m_selection->pipelineSource()->getProxy()->UpdateVTKObjects();
  int value;
  m_selection->pipelineSource()->updatePipeline();
  m_selection->pipelineSource()->getProxy()->UpdatePropertyInformation();
  vtkSMPropertyHelper(m_selection->pipelineSource()->getProxy(),"PixelValue").Get(&value,1);
  
  return value > 0;
}



//-----------------------------------------------------------------------------
ISegmentationExtension* SegmentationSelectionExtension::clone()
{
  return new SegmentationSelectionExtension();
}