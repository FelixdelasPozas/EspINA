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


#include "FreeFormSource.h"
#include <cache/CachedObjectBuilder.h>
#include <model/EspinaFactory.h>
#include <vtkSliceView.h>
#include <EspinaCore.h>
#include <EspinaView.h>

#include <QDateTime>
#include <QDebug>

typedef ModelItem::ArgumentId ArgumentId;

// We use timestamp as ID
const ArgumentId ID = ArgumentId("ID", true);
const ArgumentId SPACING = ArgumentId("SPACING", true);

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(double spacing[3])
: m_hasPixels(false)
{
//   CachedObjectBuilder *cob = CachedObjectBuilder::instance();
// 
//   //WARNING: Efecto 3000!
//   m_args[ID] = QDateTime::currentDateTime().toString("ddMMyyhhmmss");
//   //qDebug() << m_args[ID] << id();
// 
//   pqFilter::Arguments args;
//   m_args[SPACING] = QString("%1,%2,%3")
//                      .arg(spacing[0]).arg(spacing[1]).arg(spacing[2]);
// // 		     .arg(extent[3]).arg(extent[4]).arg(extent[5]);
// 
//   args << pqFilter::Argument("Id", pqFilter::Argument::UNKOWN, m_args[ID]);
//   args << pqFilter::Argument("Spacing",pqFilter::Argument::DOUBLEVECT, m_args[SPACING]);
//   m_source = cob->createFilter("sources","FreeFormSource", args, false, true);
//   Q_ASSERT(m_source->getNumberOfData() == 1);
// 
//   if (!m_seg)
//     m_seg = EspinaFactory::instance()->createSegmentation(this, 0);
}

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(ModelItem::Arguments args)
: m_args(args)
, m_hasPixels(false)
{
//   CachedObjectBuilder *cob = CachedObjectBuilder::instance();
// 
//   QString segId = id() + "_0";
//   if ((m_source = cob->loadFile(segId)) == NULL)
//   {
//     pqFilter::Arguments sourceArgs;
//     sourceArgs << pqFilter::Argument("Spacing",pqFilter::Argument::DOUBLEVECT, m_args[SPACING]);
//     m_source = cob->createFilter("sources","FreeFormSource", sourceArgs);
//     Q_ASSERT(m_source->getNumberOfData() == 1);
//   }
// 
//   Q_ASSERT(m_source);
//   if (!m_seg)
//     m_seg = EspinaFactory::instance()->createSegmentation(this, 0);
}

//-----------------------------------------------------------------------------
FreeFormSource::~FreeFormSource()
{

}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(vtkSliceView::VIEW_PLANE plane,
			 QVector3D center, int radius)
{
  Q_ASSERT(false);
//   int points[5] = {
//     center.x(), center.y(), center.z(),
//     radius, plane};
//   vtkSMProxy *proxy = m_source->pipelineSource()->getProxy();
//   vtkSMPropertyHelper(proxy, "Draw").Set(points, 5);
//   proxy->UpdateVTKObjects();
// //   m_source->pipelineSource()->updatePipeline();
//   double cross[3];
//   //TODO: Find a clearer way to get the segmentation rendered
//   EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
//   view->center(cross);
//   view->setCenter(cross[0], cross[1], cross[2], true);
//   view->setCenter(cross[0], cross[1], cross[2], true);
//   view->forceRender();
  m_hasPixels = true;
}

//-----------------------------------------------------------------------------
void FreeFormSource::erase(vtkSliceView::VIEW_PLANE plane,
			  QVector3D center, int radius)
{
  Q_ASSERT(false);
//   if (!m_hasPixels)
//     return;
// 
//   int points[5] = {
//     center.x(), center.y(), center.z(),
//     radius, plane};
//   vtkSMProxy *proxy = m_source->pipelineSource()->getProxy();
//   vtkSMPropertyHelper(proxy, "Erase").Set(points, 5);
//   proxy->UpdateVTKObjects();
// //   m_source->pipelineSource()->updatePipeline();
//   double cross[3];
//   //TODO: Find a clearer way to get the segmentation rendered
//   EspinaView *view = EspinaCore::instance()->viewManger()->currentView();
//   view->center(cross);
//   view->setCenter(cross[0], cross[1], cross[2], true);
//   view->setCenter(cross[0], cross[1], cross[2], true);
//   view->forceRender();
}

//-----------------------------------------------------------------------------
QString FreeFormSource::id() const
{
  return m_args.hash();
}

//-----------------------------------------------------------------------------
QVariant FreeFormSource::data(int role) const
{
  if (role == Qt::DisplayRole)
    return FFS;
  else
    return QVariant();
}

//-----------------------------------------------------------------------------
QString FreeFormSource::serialize() const
{
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
int FreeFormSource::numberOutputs() const
{
//   return m_source && m_seg && m_hasPixels?1:0;
}

//-----------------------------------------------------------------------------
EspinaVolume* FreeFormSource::output(int i) const
{
  //Q_ASSERT(m_source->getNumberOfData() > 0);
}

//-----------------------------------------------------------------------------
QWidget* FreeFormSource::createConfigurationWidget()
{
  return NULL;
}