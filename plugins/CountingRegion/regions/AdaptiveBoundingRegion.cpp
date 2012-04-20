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


#include "AdaptiveBoundingRegion.h"

#include <common/cache/CachedObjectBuilder.h>
#include <common/model/Channel.h>
#include <vtkRectangularBoundingRegionWidget.h>

#include <pq3DWidget.h>
#include <pq3DWidgetInterface.h>
#include <pqApplicationCore.h>
#include <pqInterfaceTracker.h>
#include <pqPipelineSource.h>
#include <vtkPVXMLElement.h>
#include <vtkSMNewWidgetRepresentationProxy.h>
#include <vtkSMPropertyHelper.h>

class AdaptiveRegionWidget
: public SliceWidget
{
public:
  explicit AdaptiveRegionWidget(pq3DWidget* widget) : SliceWidget(widget){}
  virtual void setSlice(double pos, vtkPVSliceView::VIEW_PLANE plane)
  {
    vtkAbstractWidget *aw = m_widget->getWidgetProxy()->GetWidget();
    vtkRectangularBoundingRegionWidget *regionWidget =
      dynamic_cast<vtkRectangularBoundingRegionWidget*>(aw);
    regionWidget->SetSlice(pos);
    SliceWidget::setSlice(pos, plane);
  }
};

//-----------------------------------------------------------------------------
AdaptiveBoundingRegion::AdaptiveBoundingRegion(CountingRegionSampleExtension *sampleExt,
					       Channel* channel,
					       double inclusion[3],
					       double exclusion[3])
: BoundingRegion(sampleExt, inclusion, exclusion)
{
  // Configuration of Bounding Region interface
  pqFilter::Arguments regionArgs;
  regionArgs << pqFilter::Argument("Input",pqFilter::Argument::INPUT,channel->volume().id());
  QString inclusionArg = QString("%1,%2,%3").arg(left()).arg(top()).arg(upper());
  regionArgs << pqFilter::Argument("InclusionOffset", pqFilter::Argument::DOUBLEVECT, inclusionArg);
  QString exclusionArg = QString("%1,%2,%3").arg(right()).arg(bottom()).arg(lower());
  regionArgs << pqFilter::Argument("ExclusionOffset", pqFilter::Argument::DOUBLEVECT, exclusionArg);

  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  m_boundingRegion = cob->createFilter("filters","AdaptiveBoundingRegion", regionArgs);

  Q_ASSERT(m_boundingRegion);
}

//-----------------------------------------------------------------------------
AdaptiveBoundingRegion::~AdaptiveBoundingRegion()
{
  foreach(pq3DWidget *widget, m_widgets)
  {
    widget->deselect();
    widget->deleteLater();
  }
  m_widgets.clear();

  CachedObjectBuilder *cob = CachedObjectBuilder::instance();
  if (m_boundingRegion)
    cob->removeFilter(m_boundingRegion);
}

//-----------------------------------------------------------------------------
QVariant AdaptiveBoundingRegion::data(int role) const
{
  if (role == Qt::DisplayRole)
  {
    QString repName = QString("Adaptive Region (%1,%2,%3,%4,%5,%6)")
      .arg(left()).arg(top()).arg(upper())
      .arg(right()).arg(bottom()).arg(lower());
    return repName;
  }

  return BoundingRegion::data(role);
}

//-----------------------------------------------------------------------------
pq3DWidget* AdaptiveBoundingRegion::createWidget()
{
  pq3DWidget *widget = createWidget("RectangularBoundingVolume");
  Q_ASSERT(widget);
  // By default ParaView doesn't "Apply" the changes to the widget. So we set
  // up a slot to "Apply" when the interaction ends.
  QObject::connect(widget, SIGNAL(widgetEndInteraction()),
		   widget, SLOT(accept()));
  QObject::connect(widget, SIGNAL(widgetEndInteraction()),
		   this, SLOT(resetWidgets()));

  m_widgets << widget;

  return widget;
}

//-----------------------------------------------------------------------------
SliceWidget* AdaptiveBoundingRegion::createSliceWidget(vtkPVSliceView::VIEW_PLANE plane)
{
  pq3DWidget *widget = createWidget("RectangularBoundingRegion");
  Q_ASSERT(widget);
  // By default ParaView doesn't "Apply" the changes to the widget. So we set
  // up a slot to "Apply" when the interaction ends.
  QObject::connect(widget, SIGNAL(widgetEndInteraction()),
		   widget, SLOT(accept()));
  QObject::connect(widget, SIGNAL(widgetEndInteraction()),
		   this, SLOT(resetWidgets()));

  vtkAbstractWidget *aw = widget->getWidgetProxy()->GetWidget();
  vtkRectangularBoundingRegionWidget *regionWidget =
    dynamic_cast<vtkRectangularBoundingRegionWidget*>(aw);
  Q_ASSERT(regionWidget);
  regionWidget->SetPlane(plane);

  m_widgets << widget;

  return new AdaptiveRegionWidget(widget);
}

//-----------------------------------------------------------------------------
void AdaptiveBoundingRegion::setBounds(double bounds[6])
{
  Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
void AdaptiveBoundingRegion::bounds(double bounds[6])
{
  Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
void AdaptiveBoundingRegion::setEnabled(bool enable)
{
  Q_ASSERT(false);
}

//-----------------------------------------------------------------------------
void AdaptiveBoundingRegion::resetWidgets()
{
  foreach(pq3DWidget *widget, m_widgets)
    widget->reset();
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
  vtkSMPropertyHelper(proxy, "InclusionOffset").Get(m_inclusion, 3);
  vtkSMPropertyHelper(proxy, "ExclusionOffset").Get(m_exclusion, 3);
  emitDataChanged();
}

//-----------------------------------------------------------------------------
pq3DWidget* AdaptiveBoundingRegion::createWidget(QString name)
{
  vtkSMProxy *proxy = m_boundingRegion->pipelineSource()->getProxy();
//   vtkPVXMLElement *hints = proxy->GetHints();
//   Q_ASSERT(hints->GetNumberOfNestedElements() == 1);
//   vtkPVXMLElement* element = hints->GetNestedElement(0);
  QList<pq3DWidgetInterface *> interfaces =
  pqApplicationCore::instance()->interfaceTracker()->interfaces<pq3DWidgetInterface*>();
  pq3DWidget *widget = 0;

  // Create the widget from plugins.
  foreach (pq3DWidgetInterface* iface, interfaces)
  {
    widget = iface->newWidget(name, proxy, proxy);
    if (widget)
    {
//       widget->setHints(element);
      return widget;
    }
  }
  return NULL;
}

