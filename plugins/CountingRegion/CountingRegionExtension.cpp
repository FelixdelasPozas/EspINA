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


#include "CountingRegionExtension.h"

// Debug
#include "espina_debug.h"

#include "filter.h"
#include "sample.h"
#include "segmentation.h"
#include "cache/cachedObjectBuilder.h"

#include <pqPipelineSource.h>
#include <pqOutputPort.h>
#include <vtkSMOutputPort.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkSMInputProperty.h>
#include <vtkSMStringVectorProperty.h>
#include <vtkSMPropertyHelper.h>
#include <pqDisplayPolicy.h>
#include <pqApplicationCore.h>
#include <pqPipelineRepresentation.h>
#include <pqObjectBuilder.h>
#include <pq3DWidget.h>
#include <vtkBoxWidget2.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <pqScalarsToColors.h>
#include <vtkSMPVRepresentationProxy.h>
#include "vtkRectangularBoundingRegionWidget.h"
#include <crosshairExtension.h>
#include <pqView.h>
#include <vtkWidgetRepresentation.h>
#include <labelMapExtension.h>

//!-----------------------------------------------------------------------
//! COUNTING REGION SEGMENTATION EXTENSION
//!-----------------------------------------------------------------------
//! Information Provided:
//! - Discarted
//------------------------------------------------------------------------
CountingRegion::SegmentationExtension::SegmentationExtension()
: m_discarted(NULL)
{
  m_availableInformations << "Discarted";
}

//------------------------------------------------------------------------
CountingRegion::SegmentationExtension::~SegmentationExtension()
{
  if (m_discarted)
  {
    EXTENSION_DEBUG("Deleted " << ID << " from " << m_seg->id());
    CachedObjectBuilder *cob = CachedObjectBuilder::instance();
    cob->removeFilter(m_discarted);
    m_discarted = NULL;
  }
}

//------------------------------------------------------------------------
ExtensionId CountingRegion::SegmentationExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
void CountingRegion::SegmentationExtension::initialize(Segmentation* seg)
{
  m_seg = seg;
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  
  m_seg->creator()->pipelineSource()->updatePipeline();
  vtkFilter::Arguments countingArgs;
  countingArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_seg->id()));
  m_discarted = cob->createFilter("filters","CountingRegion", countingArgs);
  assert(m_discarted);
  
  SampleExtension *sampleExt = dynamic_cast<SampleExtension *>(m_seg->origin()->extension(CountingRegion::ID));
  updateRegions(sampleExt->regions());
}

//------------------------------------------------------------------------
ISegmentationRepresentation* CountingRegion::SegmentationExtension::representation(QString rep)
{
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QVariant CountingRegion::SegmentationExtension::information(QString info)
{
  if (info == "Discarted")
  {
    int isDiscarted = 0;
    m_discarted->pipelineSource()->updatePipeline();
    vtkSMPropertyHelper(m_discarted->pipelineSource()->getProxy(),"Discarted").UpdateValueFromServer();
    vtkSMPropertyHelper(m_discarted->pipelineSource()->getProxy(),"Discarted").Get(&isDiscarted,1);
    m_seg->setVisible(!isDiscarted);
    m_seg->notifyInternalUpdate();
    return (bool)isDiscarted;
  }
  
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//------------------------------------------------------------------------
void CountingRegion::SegmentationExtension::updateRegions(QMap<int, BoundingRegion *>& regions)
{
  EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
  EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
  
  vtkSMProperty *p;
  
  vtkstd::vector<vtkSMProxy *> inputs;
  vtkstd::vector<unsigned int> ports;
  
  // First input is the segmentation object
  inputs.push_back(m_seg->creator()->pipelineSource()->getProxy());
  ports.push_back(0);
    
  foreach(BoundingRegion *region, regions)
  {
    region->pipelineSource()->updatePipeline();
    inputs.push_back(region->pipelineSource()->getProxy());
    ports.push_back(0);
  }
    
  p = m_discarted->pipelineSource()->getProxy()->GetProperty("Input");
  vtkSMInputProperty *inputRegions = vtkSMInputProperty::SafeDownCast(p);
  assert(inputRegions);
  if (inputRegions)
  {
    inputRegions->SetProxies(static_cast<unsigned int>(inputs.size())
    , &inputs[0]
    , &ports[0]);
  }
  
  m_discarted->pipelineSource()->updatePipeline();
  
  int isDiscarted = 0;
  vtkSMPropertyHelper(m_discarted->pipelineSource()->getProxy(),"Discarted").UpdateValueFromServer();
  vtkSMPropertyHelper(m_discarted->pipelineSource()->getProxy(),"Discarted").Get(&isDiscarted,1);
  m_seg->setVisible(!isDiscarted);
  EXTENSION_DEBUG("Counting Region Extension request Segmentation Update");
  m_seg->notifyInternalUpdate();
}

//------------------------------------------------------------------------
ISegmentationExtension* CountingRegion::SegmentationExtension::clone()
{
  return new SegmentationExtension();
}


//!-----------------------------------------------------------------------
//! BOUNDING REGION SAMPLE REPRESENTATION
//!-----------------------------------------------------------------------
//! Base class for counting region's Bounding Region representations

const ISampleRepresentation::RepresentationId CountingRegion::BoundingRegion::ID  = "CountingRegion";

int CountingRegion::BoundingRegion::newId = 0;

//------------------------------------------------------------------------
CountingRegion::BoundingRegion::BoundingRegion(Sample* sample)
: ISampleRepresentation(sample)
, m_boundigRegion(NULL)
{
  m_regionId = newId++;
  
  // Create a standard item model to represent the region
  QStandardItem * name = new QStandardItem("Bounding Region");
  QStandardItem * id = new QStandardItem(QString::number(m_regionId));
  QStandardItem * renderInXY = new QStandardItem();
  renderInXY->setData(true,Qt::CheckStateRole);
  renderInXY->setCheckState(Qt::Checked);
  renderInXY->setFlags(renderInXY->flags() |  Qt::ItemIsUserCheckable| Qt::ItemIsEditable);
  QStandardItem * renderInYZ = new QStandardItem();
  renderInYZ->setData(true,Qt::CheckStateRole);
  renderInYZ->setCheckState(Qt::Checked);
  renderInYZ->setFlags(renderInXY->flags());
  QStandardItem * renderInXZ = new QStandardItem();
  renderInXZ->setData(true,Qt::CheckStateRole);
  renderInXZ->setCheckState(Qt::Checked);
  renderInXZ->setFlags(renderInXY->flags());
  QStandardItem * renderIn3D = new QStandardItem();
  renderIn3D->setData(true,Qt::CheckStateRole);
  renderIn3D->setCheckState(Qt::Checked);
  renderIn3D->setFlags(renderInXY->flags());
  QStandardItem * description = new QStandardItem("");
  m_modelInfo << name << id << renderInXY << renderInYZ << renderInXZ << renderIn3D << description;
}

//------------------------------------------------------------------------
CountingRegion::BoundingRegion::~BoundingRegion()
{
  if (m_boundigRegion)
  {
    EXTENSION_DEBUG("Deleted " << " bounded area " << " from " << m_sample->id());
    CachedObjectBuilder *cob = CachedObjectBuilder::instance();
    cob->removeFilter(m_boundigRegion);
    m_boundigRegion = NULL;
  }

}

//------------------------------------------------------------------------
QString CountingRegion::BoundingRegion::id()
{
  return ID;
}

//------------------------------------------------------------------------
void CountingRegion::BoundingRegion::render(pqView* view, ViewType type)
{

}

//------------------------------------------------------------------------
pqPipelineSource* CountingRegion::BoundingRegion::pipelineSource()
{
  assert(m_boundigRegion);
  return  m_boundigRegion->pipelineSource();
}

//------------------------------------------------------------------------
int CountingRegion::BoundingRegion::totalVolume()
{
  int vol;
  m_boundigRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundigRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"TotalVolume").Get(&vol,1);
  return vol;
}

//------------------------------------------------------------------------
int CountingRegion::BoundingRegion::inclusionVolume()
{
  int vol;
  m_boundigRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundigRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"InclusionVolume").Get(&vol,1);
  return vol;
}


//------------------------------------------------------------------------
int CountingRegion::BoundingRegion::exclusionVolume()
{
  int vol;
  m_boundigRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundigRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"ExclusionVolume").Get(&vol,1);
  return vol;
}

//------------------------------------------------------------------------
QString CountingRegion::BoundingRegion::getArguments()
{
  return QString("%1,%2,%3,%4,%5,%6;")
    .arg(m_inclusion[0]).arg(m_exclusion[0])
    .arg(m_inclusion[1]).arg(m_exclusion[1])
    .arg(m_inclusion[2]).arg(m_exclusion[2]);
}

//------------------------------------------------------------------------
const QList< QStandardItem* > CountingRegion::BoundingRegion::getModelItem()
{
  QList<QStandardItem *> cr;
  QString repName = QString("%1 (%2,%3,%4,%5,%6,%7)") 
  .arg(id())
  .arg(m_inclusion[0]).arg(m_inclusion[1]).arg(m_inclusion[2])
  .arg(m_exclusion[0]).arg(m_exclusion[1]).arg(m_exclusion[2]);
  // Create a standard item model to represent the region
  QStandardItem * name = new QStandardItem(repName); 
  QStandardItem * id = new QStandardItem(QString::number(m_regionId));
  QStandardItem * renderInXY = new QStandardItem();
  renderInXY->setData(true,Qt::CheckStateRole);
  renderInXY->setCheckState(Qt::Checked);
  renderInXY->setFlags(renderInXY->flags() |  Qt::ItemIsUserCheckable| Qt::ItemIsEditable);
  QStandardItem * renderInYZ = new QStandardItem();
  renderInYZ->setData(true,Qt::CheckStateRole);
  renderInYZ->setCheckState(Qt::Checked);
  renderInYZ->setFlags(renderInXY->flags());
  QStandardItem * renderInXZ = new QStandardItem();
  renderInXZ->setData(true,Qt::CheckStateRole);
  renderInXZ->setCheckState(Qt::Checked);
  renderInXZ->setFlags(renderInXY->flags());
  QStandardItem * renderIn3D = new QStandardItem();
  renderIn3D->setData(true,Qt::CheckStateRole);
  renderIn3D->setCheckState(Qt::Checked);
  renderIn3D->setFlags(renderInXY->flags());
  QStandardItem * description = new QStandardItem("");

  cr <<name << id << renderInXY << renderInYZ << renderInXZ << renderIn3D << description;
  
  return cr;
}


//!-----------------------------------------------------------------------
//! RECTANGULAR REGION SAMPLE REPRESENTATION
//!-----------------------------------------------------------------------
//! Represent a Bounding Region applied to the sample

const ISampleRepresentation::RepresentationId RectangularRegion::ID  = "RectangularRegion";

//------------------------------------------------------------------------
RectangularRegion::RectangularRegion(Sample* sample,
  int left, int top, int upper,
  int right, int bottom, int lower,
  QList<QStandardItem *> &info) 
: BoundingRegion(sample)
{
  EXTENSION_DEBUG("Rectangular Region: (" <<  
  left << "," <<
  top << "," <<
  upper << "," <<
  right << "," <<
  bottom << "," <<
  lower << ") Initialized");
  
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  m_inclusion[0] = left;  m_exclusion[0] = right;
  m_inclusion[1] = top;   m_exclusion[1] = bottom;
  m_inclusion[2] = upper; m_exclusion[2] = lower;
  
  double spacing[3]; 
  m_sample->spacing(spacing);
  int extent[6];
  m_sample->extent(extent);
  
  
  // Configuration of Bounding Region interface
  vtkFilter::Arguments regionArgs;
  regionArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,""));
  regionArgs.push_back(vtkFilter::Argument("Extent",vtkFilter::INTVECT, QString("%1,%2,%3,%4,%5,%6")
		  .arg(extent[0]).arg(extent[1]).arg(extent[2])
		  .arg(extent[3]).arg(extent[4]).arg(extent[5])));
  regionArgs.push_back(vtkFilter::Argument("Spacing",vtkFilter::DOUBLEVECT, QString("%1,%2,%3")
					   .arg(spacing[0]).arg(spacing[1]).arg(spacing[2])));
  regionArgs.push_back(vtkFilter::Argument("Inclusion",vtkFilter::INTVECT, QString("%1,%2,%3")
					   .arg(left).arg(top).arg(upper)));
  regionArgs.push_back(vtkFilter::Argument("Exclusion",vtkFilter::INTVECT, QString("%1,%2,%3")
					   .arg(right).arg(bottom).arg(lower)));
  m_boundigRegion = cob->createFilter("filters","RectangularBoundingRegion", regionArgs);
  
  if (!m_boundigRegion)
  {
    qDebug() << "Couldn't create Bounding Region Filter";
    assert(false);
  }
  
  m_boundigRegion->pipelineSource()->updatePipeline();
  for (int i=0; i<4; i++)
  {
    QList<pq3DWidget *> widgets =  pq3DWidget::createWidgets(m_boundigRegion->pipelineSource()->getProxy(), m_boundigRegion->pipelineSource()->getProxy());
    m_widget[i] = widgets.first();
    QObject::connect(m_widget[i], SIGNAL(widgetEndInteraction()),
		     m_widget[i], SLOT(accept()));
    QObject::connect(m_widget[i], SIGNAL(widgetEndInteraction()),
		     this, SLOT(reset()));
//     QObject::connect(widgets[i], SIGNAL(widgetEndInteraction()),
// 		     this, SLOT(modifyVOI()));
  }
  
  QString repName = QString("Rectangular Region (%1,%2,%3,%4,%5,%6)") 
  .arg(left).arg(top).arg(upper).arg(right).arg(bottom).arg(lower);
  
  m_modelInfo[0]->setData(repName,Qt::DisplayRole);
  
  info = m_modelInfo;
}

//------------------------------------------------------------------------
RectangularRegion::~RectangularRegion()
{
  for(int i=0; i<4; i++)
  {
    if (m_widget[i])
    {
      m_widget[i]->deselect();
      delete m_widget[i];
      m_widget[i] = NULL;
    }
  }
}

//------------------------------------------------------------------------
void RectangularRegion::render(pqView* view, ViewType type)
{
//   if (type != VIEW_PLANE_XY)
//     return;
  if (m_widget[type]->view() != view)
    m_widget[type]->setView(view);

  if (m_modelInfo[2+type]->data(Qt::CheckStateRole) == Qt::Checked)
    m_widget[type]->select();
  else
    m_widget[type]->deselect();
  
  vtkRectangularBoundingRegionWidget *regionwidget = dynamic_cast<vtkRectangularBoundingRegionWidget*>(m_widget[type]->getWidgetProxy()->GetWidget());
  assert(regionwidget);
  CrosshairExtension::SampleRepresentation *samRep = dynamic_cast<CrosshairExtension::SampleRepresentation *>(m_sample->representation("Crosshairs"));
  
//   m_widget[type]->reset();
  double spacing[3];
  m_sample->spacing(spacing);
  regionwidget->SetViewType(type);
  if (type != VIEW_3D)
  {
//     int normalDir = (type + 2) % 3;
    regionwidget->SetSlice(samRep->slice(type),spacing);
  }
  
//   m_widget[type]->setWidgetVisible(true);
//   m_widget[type]->accept();
}

//------------------------------------------------------------------------
void RectangularRegion::clear(pqView* view, ViewType type)
{
  if (m_widget[type])
    m_widget[type]->deselect();
}



//------------------------------------------------------------------------
void RectangularRegion::setInclusive(int left, int top, int upper)
{

}

//------------------------------------------------------------------------
void RectangularRegion::setExclusive(int right, int bottom, int lower)
{

}

//------------------------------------------------------------------------
QString RectangularRegion::description()
{
  QString desc("Type: Rectangular Region\n"
	       "Volume Information:\n"
	       "  Total Volume:\n"
	       "    %1 px\n"
	       "    %2 %3\n"
	       "  Inclusion Volume:\n"
	       "    %4 px\n"
	       "    %5 %3\n"
	       "  Exclusion Volume:\n"
	       "    %6 px\n"
	       "    %7 %3\n"
	      );
  
  int extent[6];
  m_sample->extent(extent);
  double spacing[3];
  m_sample->spacing(spacing);
  double volPixel = spacing[0]*spacing[1]*spacing[2]; //Volume of 1 pixel
  unsigned int totalVolInPixel = totalVolume();//(extent[1]-extent[0]+1)*(extent[3]-extent[2]+1)*(extent[5]-extent[4]+1);
  double totalVolInUnits = totalVolInPixel*volPixel;
  unsigned int inclusionVolInPixel = inclusionVolume();
  double inclusionVolInUnits = inclusionVolInPixel*volPixel;
  unsigned int exclusionVolInPixel = exclusionVolume();
  double exclusionVolInUnits = exclusionVolInPixel*volPixel;
  desc = desc.arg(totalVolInPixel,0).arg(totalVolInUnits,0,'f',2).arg(m_sample->units());
  desc = desc.arg(inclusionVolInPixel,0).arg(inclusionVolInUnits,0,'f',2);
  desc = desc.arg(exclusionVolInPixel,0).arg(exclusionVolInUnits,0,'f',2);
  return desc;
}


//------------------------------------------------------------------------
void RectangularRegion::reset()
{
  double spacing[3];
  m_sample->spacing(spacing);
  for (int i=0; i<4; i++)
    if (m_widget[i])
    {
      m_widget[i]->reset();
//       if (i < 3)//NOTE: To force repaint on other views in case
// 		// the counting region visibility changes    
//       {
// 	  vtkRectangularBoundingRegionWidget *regionwidget = dynamic_cast<vtkRectangularBoundingRegionWidget*>(m_widget[i]->getWidgetProxy()->GetWidget());
// 	  assert(regionwidget);
// 	  CrosshairExtension::SampleRepresentation *samRep = dynamic_cast<CrosshairExtension::SampleRepresentation *>(m_sample->representation("Crosshairs"));
//   
// // 	  int normalDir = (i + 2) % 3;
// 	  regionwidget->SetSlice(samRep->slice(ViewType(i)),spacing);
//       }
    }
    vtkSMPropertyHelper(m_boundigRegion->pipelineSource()->getProxy(),"Inclusion").Get(m_inclusion,3);
    vtkSMPropertyHelper(m_boundigRegion->pipelineSource()->getProxy(),"Exclusion").Get(m_exclusion,3);
    QString repName = QString("Rectangular Region (%1,%2,%3,%4,%5,%6)") 
    .arg(m_inclusion[0])
    .arg(m_exclusion[0])
    .arg(m_inclusion[1])
    .arg(m_exclusion[1])
    .arg(m_inclusion[2])
    .arg(m_exclusion[2]);
    
    m_modelInfo[0]->setData(repName,Qt::DisplayRole);
   
 emit regionChanged(this);
}

//!-----------------------------------------------------------------------
//! ADAPTIVE REGION SAMPLE REPRESENTATION
//!-----------------------------------------------------------------------
//! Represent a Bounding Region applied to the sample
//------------------------------------------------------------------------
const ISampleRepresentation::RepresentationId AdaptiveRegion::ID  = "AdaptiveRegion";

AdaptiveRegion::AdaptiveRegion(Sample* sample, int left, int top, int upper,
			       int right, int bottom, int lower,
			       QList<QStandardItem *> &info
			      )
: BoundingRegion(sample)
{
  EXTENSION_DEBUG("Adaptive Region: (" <<  
  left << "," <<
  top << "," <<
  upper << "," <<
  right << "," <<
  bottom << "," <<
  lower << ") Initialized");
  
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  m_inclusion[0] = left;  m_exclusion[0] = right;
  m_inclusion[1] = top;   m_exclusion[1] = bottom;
  m_inclusion[2] = upper; m_exclusion[2] = lower;
  
  // Configuration of Bounding Region interface
  vtkFilter::Arguments regionArgs;
  regionArgs.push_back(vtkFilter::Argument("Input",vtkFilter::INPUT,m_sample->id()));
  regionArgs.push_back(vtkFilter::Argument("Inclusion",vtkFilter::INTVECT,QString("%1,%2,%3").arg(left).arg(top).arg(upper)));
  regionArgs.push_back(vtkFilter::Argument("Exclusion",vtkFilter::INTVECT,QString("%1,%2,%3").arg(right).arg(bottom).arg(lower)));
  m_boundigRegion = cob->createFilter("filters","AdaptiveBoundingRegion", regionArgs);
  
  if (!m_boundigRegion)
  {
    qDebug() << "Couldn't create Bounding Region Filter";
    assert(false);
  }
    
  m_boundigRegion->pipelineSource()->updatePipeline();
  for (int i=0; i<4; i++)
  {
    QList<pq3DWidget *> widgets =  pq3DWidget::createWidgets(m_boundigRegion->pipelineSource()->getProxy(), m_boundigRegion->pipelineSource()->getProxy());
    m_widget[i] = widgets.first();
    QObject::connect(m_widget[i], SIGNAL(widgetEndInteraction()),
		     m_widget[i], SLOT(accept()));
    QObject::connect(m_widget[i], SIGNAL(widgetEndInteraction()),
		     this, SLOT(reset()));
    
  }
  info = m_modelInfo;
}


//------------------------------------------------------------------------
AdaptiveRegion::~AdaptiveRegion()
{
  for(int i=0; i<4; i++)
  {
    if (m_widget[i])
    {
      m_widget[i]->deselect();
      delete m_widget[i];
      m_widget[i] = NULL;
    }
  }
}

void AdaptiveRegion::render(pqView* view, ViewType type)
{
//   pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
//   pqDataRepresentation *dr = dp->setRepresentationVisibility(pipelineSource()->getOutputPort(0),view,true);
//   pqPipelineRepresentation *rep = qobject_cast<pqPipelineRepresentation *>(dr);
//   rep->setRepresentation(vtkSMPVRepresentationProxy::WIREFRAME);
//   vtkSMProxy * lut = rep->getLookupTableProxy();
//   double colors[8] = {0,1,0,0,255,0,1,0};
//   vtkSMPropertyHelper(lut,"RGBPoints").Set(colors,8);
//   lut->UpdateVTKObjects();
//   
//   
//   double opacity = 0.5;
//   vtkSMPropertyHelper(rep->getProxy(),"Opacity").Set(opacity);
//   rep->getProxy()->UpdateVTKObjects();
// vtkRectangularBoundingRegionWidget *regionwidget = dynamic_cast<vtkRectangularBoundingRegionWidget*>(m_widget[type]->getWidgetProxy()->GetWidget());
// assert(regionwidget);
// CrosshairExtension::SampleRepresentation *samRep = dynamic_cast<CrosshairExtension::SampleRepresentation *>(m_sample->representation("Crosshairs"));
// regionwidget->SetSlice(samRep->slice(type));
// regionwidget->SetViewType(type);
// m_widget[type]->setView(view);
// m_widget[type]->select();
// m_widget[type]->setVisible(m_visible[type]);
  if (m_widget[type]->view() != view)
    m_widget[type]->setView(view);

//   if (m_visible[type])
  if (m_modelInfo[2+type]->data(Qt::CheckStateRole) == Qt::Checked)
    m_widget[type]->select();
  else
    m_widget[type]->deselect();
  
  vtkRectangularBoundingRegionWidget *regionwidget = dynamic_cast<vtkRectangularBoundingRegionWidget*>(m_widget[type]->getWidgetProxy()->GetWidget());
  assert(regionwidget);
  CrosshairExtension::SampleRepresentation *samRep = dynamic_cast<CrosshairExtension::SampleRepresentation *>(m_sample->representation("Crosshairs"));
  double spacing[3];
  m_sample->spacing(spacing);
  regionwidget->SetViewType(type);
  if (type != VIEW_3D)
  {
//     int normalDir = (type + 2) % 3;
    regionwidget->SetSlice(samRep->slice(type),spacing);
  }
}

//------------------------------------------------------------------------
void AdaptiveRegion::clear(pqView* view, ViewType type)
{
  if (m_widget[type])
    m_widget[type]->deselect();
}


//------------------------------------------------------------------------
int AdaptiveRegion::totalAdaptiveVolume()
{
  int vol;
  m_boundigRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundigRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"TotalAdaptiveVolume").Get(&vol,1);
  return vol;
}

//------------------------------------------------------------------------
int AdaptiveRegion::exclusionAdaptiveVolume()
{
  int vol;
  m_boundigRegion->pipelineSource()->updatePipeline();
  vtkSMProxy *proxy = m_boundigRegion->pipelineSource()->getProxy();
  proxy->UpdatePropertyInformation();
  vtkSMPropertyHelper(proxy,"ExclusionAdaptiveVolume").Get(&vol,1);
  return vol;
}

//------------------------------------------------------------------------
void AdaptiveRegion::setInclusive(int left, int top, int upper)
{

}

//------------------------------------------------------------------------
void AdaptiveRegion::setExclusive(int right, int bottom, int lower)
{

}

//------------------------------------------------------------------------
QString AdaptiveRegion::description()
{
  QString desc("Type: Adaptive Region\n"
	       "Volume Information:\n"
	       "  Total Volume:\n"
	       "    %1 px\n"
	       "    %2 %3\n"
	       "  Total Adaptive Volume:\n"
	       "    %4 px\n"
	       "    %5 %3\n"
	       "  Inclusion Volume:\n"
	       "    %6 px\n"
	       "    %7 %3\n"
	       "  Exclusion Volume:\n"
	       "    %8 px\n"
	       "    %9 %3\n"
	       "  Exclusion Adaptive Volume:\n"
	       "    %10 px\n"
	       "    %11 %3\n"
	      );
  
  int extent[6];
  m_sample->extent(extent);
  double spacing[3];
  m_sample->spacing(spacing);
  double volPixel = spacing[0]*spacing[1]*spacing[2]; //Volume of 1 pixel
  
  unsigned int volumeInPixel;
  double volumeInUnits;
  
  volumeInPixel = totalVolume();
  volumeInUnits = volumeInPixel*volPixel;
  desc = desc.arg(volumeInPixel,0).arg(volumeInUnits,0,'f',2).arg(m_sample->units());

  volumeInPixel = totalAdaptiveVolume();
  volumeInUnits = volumeInPixel*volPixel;
  desc = desc.arg(volumeInPixel,0).arg(volumeInUnits,0,'f',2);
  
  volumeInPixel = inclusionVolume();
  volumeInUnits = volumeInPixel*volPixel;
  desc = desc.arg(volumeInPixel,0).arg(volumeInUnits,0,'f',2);
  
  volumeInPixel = exclusionVolume();
  volumeInUnits = volumeInPixel*volPixel;
  desc = desc.arg(volumeInPixel,0).arg(volumeInUnits,0,'f',2);
  
  volumeInPixel = exclusionAdaptiveVolume();
  volumeInUnits = volumeInPixel*volPixel;
  desc = desc.arg(volumeInPixel,0).arg(volumeInUnits,0,'f',2);
  
  return desc;
}


void AdaptiveRegion::reset()
{
  double spacing[3];
  m_sample->spacing(spacing);
  for (int i=0; i<4; i++)
    if (m_widget[i])
    {
      m_widget[i]->reset();
      if (i < 3)//NOTE: To force repaint on other views in case
		// the counting region visibility changes    
      {
	  vtkRectangularBoundingRegionWidget *regionwidget = dynamic_cast<vtkRectangularBoundingRegionWidget*>(m_widget[i]->getWidgetProxy()->GetWidget());
	  assert(regionwidget);
	  CrosshairExtension::SampleRepresentation *samRep = dynamic_cast<CrosshairExtension::SampleRepresentation *>(m_sample->representation("Crosshairs"));
  
// 	  int normalDir = (i + 2) % 3;
	  regionwidget->SetSlice(samRep->slice(ViewType(i)),spacing);
      }
    }
    vtkSMPropertyHelper(m_boundigRegion->pipelineSource()->getProxy(),"Inclusion").Get(m_inclusion,3);
    vtkSMPropertyHelper(m_boundigRegion->pipelineSource()->getProxy(),"Exclusion").Get(m_exclusion,3);
    QString repName = QString("Adaptive Region (%1,%2,%3,%4,%5,%6)") 
    .arg(m_inclusion[0])
    .arg(m_exclusion[0])
    .arg(m_inclusion[1])
    .arg(m_exclusion[1])
    .arg(m_inclusion[2])
    .arg(m_exclusion[2]);
    
    m_modelInfo.first()->setData(repName,Qt::DisplayRole);
   
 emit regionChanged(this);
}




//!-----------------------------------------------------------------------
//! CROSSHAIR EXTENSION
//!-----------------------------------------------------------------------
//! Provides:
//! - CountingRegion Representation

//------------------------------------------------------------------------
CountingRegion::SampleExtension::SampleExtension()
: m_numRepresentations(0)
{
}

//------------------------------------------------------------------------
CountingRegion::SampleExtension::~SampleExtension()
{
  foreach(BoundingRegion *region, m_regions)
    delete region;
  
  m_regions.clear();
}

//------------------------------------------------------------------------
void CountingRegion::SampleExtension::initialize(Sample* sample)
{
  EXTENSION_DEBUG(ID << " Initialized");
  m_sample = sample;
}

//------------------------------------------------------------------------
ISampleRepresentation* CountingRegion::SampleExtension::representation(QString rep)
{
  int id = rep.split(" ")[2].toInt();
  if (m_regions.contains(id))
    return m_regions[id];
  
  qWarning() << ID << ":" << rep << " is not provided";
  assert(false);
  return NULL;
}

//------------------------------------------------------------------------
QStringList CountingRegion::SampleExtension::availableRepresentations()
{
  QStringList reps;
  foreach(int id, m_regions.keys())
  {
    reps << QString("Counting Region %1").arg(id);
  }
  return reps;
}


//------------------------------------------------------------------------
QVariant CountingRegion::SampleExtension::information(QString info)
{
  qWarning() << ID << ":"  << info << " is not provided";
  assert(false);
  return QVariant();
}

//------------------------------------------------------------------------
void CountingRegion::SampleExtension::setArguments(QString args)
{
  QStringList regions = args.split(";");
  
  foreach (QString region, regions)
  {
//     std::cout << "Number of regions: " << m_regions.size() << std::endl;
    if (region.isEmpty())
      continue;
    
    QString type = region.section('=',0,0);
    QStringList margins = region.section('=',-1).split(',');
    int inclusion[3], exclusion[3];
    QList<QStandardItem *> row;
    for (int i=0; i<3; i++)
    {
      inclusion[i] = margins[2*i].toInt();
      exclusion[i] = margins[2*i+1].toInt();
    }
    if (type == RectangularRegion::ID)
      createRectangularRegion(inclusion[0],inclusion[1],inclusion[2],
	exclusion[0], exclusion[1], exclusion[2], row);
    else if (type == AdaptiveRegion::ID)
      createAdaptiveRegion(inclusion[0],inclusion[1],inclusion[2],
	exclusion[0], exclusion[1], exclusion[2], row);
  }
//   std::cout << "Number of regions: " << m_regions.size() << std::endl;
  emit regionsModified(this);
}

//------------------------------------------------------------------------
QString CountingRegion::SampleExtension::getArguments()
{
  QString args;
  
  foreach(BoundingRegion *r, m_regions)
  {
    args.append(r->id()+"="+r->getArguments());
  }
  
  return args;
}


//------------------------------------------------------------------------
void CountingRegion::SampleExtension::createAdaptiveRegion(int left, int top, int upper,
							      int right, int bottom, int lower,
							      QList<QStandardItem *> &info)
{
  assert(m_sample);
  AdaptiveRegion *region = new AdaptiveRegion(m_sample, left, top, upper,
					      right, bottom, lower, info);
  assert(region);
  info.last()->setData(region->description(),Qt::DisplayRole);
  connect(region,SIGNAL(regionChanged(BoundingRegion *)),this,SLOT(updateSegmentations(BoundingRegion *)));
  
  
//   for(ViewType view = VIEW_PLANE_FIRST; view <= VIEW_3D; view = ViewType(view+1))
//     region->setViewVisibility(view,true);
  
  QString repName = QString("Adaptive Region (%1,%2,%3,%4,%5,%6)") 
    .arg(left).arg(top).arg(upper).arg(right).arg(bottom).arg(lower);
  
  info.first()->setData(repName,Qt::DisplayRole);
  
  if (!m_regions.contains(region->regionId()))
  {
    m_regions[region->regionId()] = region;
    m_numRepresentations++;
    
    foreach(Segmentation *seg, m_sample->segmentations())
    {
      SegmentationExtension *ext = dynamic_cast<SegmentationExtension *>(seg->extension(CountingRegion::ID));
      if (!ext)
      {
	qDebug() << "Failed to load Counting Brick Extension on " << seg->id();
	assert(false);
      }
      ext->updateRegions(m_regions);
    }
  }
  sample()->notifyInternalUpdate();
  emit regionsModified(this);
}


//------------------------------------------------------------------------
void CountingRegion::SampleExtension::createRectangularRegion(int left, int top, int upper,
								 int right, int bottom, int lower,
								 QList<QStandardItem *> &info)
{
  RectangularRegion *region = new RectangularRegion(m_sample, left, top, upper,
						    right, bottom, lower, info);
  assert(region);
  info.last()->setData(region->description(),Qt::DisplayRole);
  connect(region,SIGNAL(regionChanged(BoundingRegion *)),this,SLOT(updateSegmentations(BoundingRegion *)));
  
  QStandardItem *regionItem = info.first();
  QString repName = info.first()->data(Qt::DisplayRole).toString();
  
  if (!m_regions.contains(region->regionId()))
  {
    m_regions[region->regionId()] = region;
    m_numRepresentations++;
    
    foreach(Segmentation *seg, m_sample->segmentations())
    {
      SegmentationExtension *ext = dynamic_cast<SegmentationExtension *>(seg->extension(CountingRegion::ID));
      if (!ext)
      {
	qDebug() << "Failed to load Counting Brick Extension on " << seg->id();
	assert(false);
      }
      ext->updateRegions(m_regions);
    }
  }
  sample()->notifyInternalUpdate();
  emit regionsModified(this);
}

//------------------------------------------------------------------------
void CountingRegion::SampleExtension::removeRegion(int regionId)
{
  if (m_regions.contains(regionId))
  {
    BoundingRegion *region = m_regions[regionId];
    m_regions.remove(regionId);
    m_numRepresentations--;
    
    foreach(Segmentation *seg, m_sample->segmentations())
    {
      SegmentationExtension *ext = dynamic_cast<SegmentationExtension *>(seg->extension(CountingRegion::ID));
      if (!ext)
      {
	qDebug() << "Failed to load Counting Brick Extension on " << seg->id();
	assert(false);
      }
      ext->updateRegions(m_regions);
    }
    delete region;
  }
  emit regionsModified(this);
}


//------------------------------------------------------------------------
ISampleExtension* CountingRegion::SampleExtension::clone()
{
  return new SampleExtension();
}

//------------------------------------------------------------------------
void CountingRegion::SampleExtension::updateSegmentations(CountingRegion::BoundingRegion* reg)
{
  foreach(Segmentation *seg, m_sample->segmentations())
  {
    SegmentationExtension *ext = dynamic_cast<SegmentationExtension *>(seg->extension(CountingRegion::ID));
    if (!ext)
    {
      qDebug() << "Failed to load Counting Brick Extension on " << seg->id();
      assert(false);
    }
    ext->information("Discarted");
  }
  m_sample->representation(LabelMapExtension::SampleRepresentation::ID)->requestUpdate(true);
  emit regionsModified(this);
}



