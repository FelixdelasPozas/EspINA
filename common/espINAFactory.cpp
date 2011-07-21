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


#include "espINAFactory.h"

// Debug
#include "espina_debug.h"

// EspINA
#include "EspinaPlugin.h"
#include "sample.h"
#include "segmentation.h"


EspINAFactory *EspINAFactory::m_instance = NULL;

EspINAFactory* EspINAFactory::instance()
{
  if (!m_instance)
    m_instance = new EspINAFactory();
  return m_instance;
}

Sample* EspINAFactory::CreateSample(vtkFilter *creator, int portNumber)
{
  std::cout << "Factory is going to create sample: " << creator->id().toStdString() << std::endl;
  Sample *sample = new Sample(creator, portNumber);
  foreach(ISampleExtension *ext, m_sampleExtensions)
  {
    sample->addExtension(ext);
  }
  return sample;
}

void EspINAFactory::addSampleExtension(ISampleExtension* ext)
{
  qDebug() << ext->id() << "registered in Factory";
  m_sampleExtensions.append(ext->clone());
}

Segmentation* EspINAFactory::CreateSegmentation(EspinaFilter *parent, vtkProduct *vtkRef)
{
  std::cout << "Factory is going to create a segmentation for vtkObject: " << vtkRef->id().toStdString() << std::endl;
  Segmentation *seg = new Segmentation(parent, vtkRef->creator(),vtkRef->portNumber());
  foreach(ISegmentationExtension *ext, m_segExtensions)
  {
    seg->addExtension(ext->clone());
  }
  return seg;
}

void EspINAFactory::addSegmentationExtension(ISegmentationExtension* ext)
{
  qDebug() << ext->id() << "registered in Factory";
  m_segExtensions.append(ext->clone());
}

QStringList EspINAFactory::segmentationAvailableInformations()
{
  QStringList informations;
  informations << "Name" << "Taxonomy";
  foreach (ISegmentationExtension *ext, m_segExtensions)
    informations << ext->availableInformations();
  
  return informations;
}


VolumeView* EspINAFactory::CreateVolumeView()
{
  VolumeView *view = new VolumeView();
  foreach(IViewWidget *widget, m_widgets)
  {
    view->addWidget(widget);
  }
  return view;
}

void EspINAFactory::addViewWidget(IViewWidget* widget)
{
  qDebug() << "registered new widget in Factory";
  m_widgets.append(widget/*->clone()*/); //TODO clone method hasn't an implementation
}



