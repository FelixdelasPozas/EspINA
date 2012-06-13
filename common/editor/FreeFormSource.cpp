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
#include <vtkPVSliceView.h>
#include <pqPipelineSource.h>
#include <vtkSMPropertyHelper.h>
#include <vtkSMProxy.h>

typedef ModelItem::ArgumentId ArgumentId;

const ArgumentId ID = ArgumentId("ID", true);

unsigned int FreeFormSource::m_count = 0;

//-----------------------------------------------------------------------------
FreeFormSource::FreeFormSource(double spacing[3])
: m_source(NULL)
, m_seg(NULL)
, m_id(m_count++)
{
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  m_args[ID] = QString::number(m_id);
  pqFilter::Arguments args;
  QString SpacingArg = QString("%1,%2,%3")
                      .arg(spacing[0]).arg(spacing[1]).arg(spacing[2]);
// 		     .arg(extent[3]).arg(extent[4]).arg(extent[5]);

  args << pqFilter::Argument("Spacing",pqFilter::Argument::DOUBLEVECT, SpacingArg);
  m_source = cob->createFilter("sources","FreeFormSource", args, false, true);
  Q_ASSERT(m_source->getNumberOfData() == 1);

  if (!m_seg)
    m_seg = EspinaFactory::instance()->createSegmentation(this, 0, m_source->data(0));
}

//-----------------------------------------------------------------------------
FreeFormSource::~FreeFormSource()
{

}

//-----------------------------------------------------------------------------
void FreeFormSource::draw(QVector3D center, int radius)
{
  int points[5] = {
    center.x(), center.y(), center.z(),
    radius, vtkPVSliceView::AXIAL};
  vtkSMProxy *proxy = m_source->pipelineSource()->getProxy();
  vtkSMPropertyHelper(proxy, "Draw").Set(points, 5);
  proxy->UpdateVTKObjects();
  m_source->pipelineSource()->updatePipeline();
}

//-----------------------------------------------------------------------------
void FreeFormSource::erase(QVector3D center, int radius)
{
  int points[5] = {
    center.x(), center.y(), center.z(),
    radius, vtkPVSliceView::AXIAL};
  vtkSMProxy *proxy = m_source->pipelineSource()->getProxy();
  vtkSMPropertyHelper(proxy, "Erase").Set(points, 5);
  proxy->UpdateVTKObjects();
  m_source->pipelineSource()->updatePipeline();
}

//-----------------------------------------------------------------------------
QString FreeFormSource::id() const
{
  return m_args.hash();
}

//-----------------------------------------------------------------------------
QVariant FreeFormSource::data(int role) const
{
  return QVariant();
}

//-----------------------------------------------------------------------------
QString FreeFormSource::serialize() const
{
  return m_args.serialize();
}

//-----------------------------------------------------------------------------
int FreeFormSource::numProducts() const
{
  return m_source?1:0;
}

//-----------------------------------------------------------------------------
Segmentation* FreeFormSource::product(int index) const
{
  Q_ASSERT(m_source->getNumberOfData() > 0);
  return m_seg;
}

//-----------------------------------------------------------------------------
pqData FreeFormSource::preview()
{
  return pqData(NULL, -1);
}

//-----------------------------------------------------------------------------
QWidget* FreeFormSource::createConfigurationWidget()
{
  return NULL;
}