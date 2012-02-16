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


#include "EspinaFactory.h"


//------------------------------------------------------------------------
EspinaFactory *EspinaFactory::m_instance = NULL;

//------------------------------------------------------------------------
EspinaFactory* EspinaFactory::instance()
{
  if (!m_instance)
    m_instance = new EspinaFactory();
  return m_instance;
}

//------------------------------------------------------------------------
void EspinaFactory::registerFilter(const QString filter, FilterFactory* factory)
{
  Q_ASSERT(m_filterFactory.contains(filter) == false);
  m_filterFactory[filter] = factory;
}


//------------------------------------------------------------------------
SamplePtr EspinaFactory::createSample(const QString id, const QString args)
{
  if (args.isNull())
    return SamplePtr(new Sample(id));
  else
    return SamplePtr(new Sample(id, args));
}

//------------------------------------------------------------------------
FilterPtr EspinaFactory::createFilter(const QString filter, const QString args)
{
  Q_ASSERT(m_filterFactory.contains(filter));

  return m_filterFactory[filter]->createFilter(filter, args);
}

//------------------------------------------------------------------------
SegmentationPtr EspinaFactory::createSegmentation(Filter* parent, pqData data)
{
//   std::cout << "Factory is going to create a segmentation for vtkObject: " << vtkRef->id().toStdString() << std::endl;
  SegmentationPtr seg(new Segmentation(parent, data));
//   foreach(ISegmentationExtension *ext, m_segExtensions)
//   {
//     seg->addExtension(ext->clone());
//   }
//   return seg;
  return seg;
}



// Sample* EspinaFactory::CreateSample(vtkFilter* creator, int portNumber, const QString& path)
// {
// //   std::cout << "Factory is going to create sample: " << creator->id().toStdString() << std::endl;
//   Sample *sample = new Sample(creator, portNumber, path);
//   foreach(ISampleExtension *ext, m_sampleExtensions)
//   {
//     sample->addExtension(ext->clone());
//   }
//   return sample;
// }
// 
// void EspinaFactory::addSampleExtension(ISampleExtension* ext)
// {
// //   qDebug() << ext->id() << "registered in Sample Factory";
//   m_sampleExtensions.append(ext->clone());
// }
// 
// void EspinaFactory::addSegmentationExtension(ISegmentationExtension* ext)
// {
// //   qDebug() << ext->id() << "registered in Segmentation Factory";
//   m_segExtensions.append(ext->clone());
// }
// 
// QStringList EspinaFactory::segmentationAvailableInformations()
// {
//   QStringList informations;
//   informations << "Name" << "Taxonomy";
//   foreach (ISegmentationExtension *ext, m_segExtensions)
//     informations << ext->availableInformations();
//   
//   return informations;
// }
// 
// 
// VolumeView* EspinaFactory::CreateVolumeView()
// {
//   VolumeView *view = new VolumeView();
//   foreach(IViewWidget *widget, m_widgets)
//   {
//     view->addWidget(widget);
//   }
//   return view;
// }
// 
// void EspinaFactory::addViewWidget(IViewWidget* widget)
// {
// //   qDebug() << "registered new widget in Factory";
//   m_widgets.append(widget/*->clone()*/); //TODO clone method hasn't an implementation
// }
// 


