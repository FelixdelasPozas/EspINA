/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#include "CountingRegionSegmentationExtension.h"

//#include "CountingRegionSampleExtension.h"

#include <common/model/Segmentation.h>
#include <common/model/Sample.h>
//#include <regions/BoundingRegion.h>

#include <QDebug>

const ModelItemExtension::ExtId CountingRegionSegmentationExtension::ID = "CountingRegionExtension";

const ModelItemExtension::InfoTag CountingRegionSegmentationExtension::DISCARTED = "Discarted";

//------------------------------------------------------------------------
CountingRegionSegmentationExtension::CountingRegionSegmentationExtension()
{
  m_availableInformations << DISCARTED;
}

//------------------------------------------------------------------------
CountingRegionSegmentationExtension::~CountingRegionSegmentationExtension()
{
}

//------------------------------------------------------------------------
ModelItemExtension::ExtId CountingRegionSegmentationExtension::id()
{
  return ID;
}

//------------------------------------------------------------------------
void CountingRegionSegmentationExtension::initialize(Segmentation* seg)
{
  m_seg = seg;

//   m_seg->volume().pipelineSource()->updatePipeline();
//   pqFilter::Arguments countingArgs;
//   countingArgs << pqFilter::Argument("Input",pqFilter::Argument::INPUT, m_seg->volume().id());
//   m_countingRegion = cob->createFilter("filters", "CountingRegion", countingArgs);
//   Q_ASSERT(m_countingRegion);
// 
//   ModelItem::Vector samples = m_seg->relatedItems(ModelItem::IN, "where");
//   //Q_ASSERT(!samples.isEmpty());
//   if (!samples.isEmpty())
//   {
//     Sample *sample = dynamic_cast<Sample *>(samples.first());
//     ModelItemExtension *ext = sample->extension(CountingRegionSampleExtension::ID);
//     Q_ASSERT(ext);
//     CountingRegionSampleExtension *sampleExt =
//     dynamic_cast<CountingRegionSampleExtension *>(ext);

//     updateRegions(sampleExt->regions());
}

//------------------------------------------------------------------------
QVariant CountingRegionSegmentationExtension::information(ModelItemExtension::InfoTag tag) const
{
  if (DISCARTED == tag)
  {
    bool isDiscarted = false;
    if (m_init)
    {
      //       m_countingRegion->pipelineSource()->updatePipeline();
      //       vtkSMProxy *proxy = m_countingRegion->pipelineSource()->getProxy();
      //       vtkSMPropertyHelper(proxy,"Discarted").UpdateValueFromServer();
      //       vtkSMPropertyHelper(proxy,"Discarted").Get(&isDiscarted,1);

      //       ModelItemExtension *ext = m_seg->extension("MarginsExtension");
      //       if (ext)
      //       {
	// 	bool ok;
	// 	double margin;
	// 	QStringList marginInfoList = ext->availableInformations();
	// 	int i = 0;
	// 	while(i < marginInfoList.size() && !isDiscarted)
	// 	{
	  // 	  margin = ext->information(marginInfoList[i++]).toDouble(&ok);
	  // 	  if (ok && margin < 1)
	  // 	    isDiscarted = true;
	  // 	}
	  //       }
    }
    m_seg->setVisible(!isDiscarted);
    return isDiscarted;
  }

  qWarning() << ID << ":"  << tag << " is not provided";
  Q_ASSERT(false);
  return QVariant();
}

//------------------------------------------------------------------------
SegmentationRepresentation* CountingRegionSegmentationExtension::representation(QString rep)
{
  qWarning() << ID << ":" << rep << " is not provided";
  Q_ASSERT(false);
  return NULL;
}


//------------------------------------------------------------------------
void CountingRegionSegmentationExtension::updateRegions(QList< BoundingRegion* > regions)
{
  qDebug() << m_seg->data().toString() << "upating bounding regions";
//   EXTENSION_DEBUG("Updating " << m_seg->id() << " bounding regions...");
//   EXTENSION_DEBUG("\tNumber of regions applied:" << regions.size());
//   m_init = !regions.isEmpty();
//   if (!m_init)
//     return;
// 
//   vtkSMProperty *p;
// 
//   vtkstd::vector<vtkSMProxy *> inputs;
//   vtkstd::vector<unsigned int> ports;
// 
//   // First input is the segmentation object
//   inputs.push_back(m_seg->volume().pipelineSource()->getProxy());
//   ports.push_back(0);
// 
//   foreach(BoundingRegion *region, regions)
//   {
//     region->region().pipelineSource()->updatePipeline();
//     inputs.push_back(region->region().pipelineSource()->getProxy());
//     ports.push_back(0);
//     connect(region, SIGNAL(modified(BoundingRegion *)),
// 	    this, SLOT(regionUpdated(BoundingRegion*)));
//   }
// 
//   vtkSMProxy *proxy =  m_countingRegion->pipelineSource()->getProxy();
//   p = proxy->GetProperty("Input");
//   vtkSMInputProperty *inputRegions = vtkSMInputProperty::SafeDownCast(p);
//   Q_ASSERT(inputRegions);
//   if (inputRegions)
//   {
//     inputRegions->SetProxies(static_cast<unsigned int>(inputs.size())
//     , &inputs[0]
//     , &ports[0]);
//   }
// 
//   m_countingRegion->pipelineSource()->updatePipeline();
// 
//   int isDiscarted = 0;
//   vtkSMPropertyHelper(proxy, "Discarted").UpdateValueFromServer();
//   vtkSMPropertyHelper(proxy, "Discarted").Get(&isDiscarted,1);
//   if (m_seg->visible() != !isDiscarted)
//   {
//     m_seg->setVisible(!isDiscarted);
//     m_seg->notifyModification();
//   }
// //   EXTENSION_DEBUG("Counting Region Extension request Segmentation Update");
}

//------------------------------------------------------------------------
SegmentationExtension* CountingRegionSegmentationExtension::clone()
{
  return new CountingRegionSegmentationExtension();
}

//------------------------------------------------------------------------
void CountingRegionSegmentationExtension::regionUpdated(BoundingRegion* region)
{
//   m_countingRegion->pipelineSource()->updatePipeline();
// 
//   int isDiscarted = 0;
//   vtkSMProxy *proxy =  m_countingRegion->pipelineSource()->getProxy();
//   vtkSMPropertyHelper(proxy, "Discarted").UpdateValueFromServer();
//   vtkSMPropertyHelper(proxy, "Discarted").Get(&isDiscarted,1);
//   if (m_seg->visible() != !isDiscarted)
//   {
//     m_seg->setVisible(!isDiscarted);
//     m_seg->notifyModification();
//   }
}
