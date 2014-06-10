/*
 * AppositionSurfaceFilterInspector.cpp
 *
 *  Created on: Jan 18, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include <Core/Extensions/AppositionSurfaceExtension.h>
#include <Core/Model/EspinaModel.h>
#include "AppositionSurfaceFilterInspector.h"
#include <Core/Model/Segmentation.h>
#include <QDebug>

namespace EspINA
{
  /// Filter Inspector
  //
  //----------------------------------------------------------------------------
  AppositionSurfaceFilterInspector::AppositionSurfaceFilterInspector(FilterSPtr filter, EspinaModel *model)
  : m_filter(filter)
  , m_model(model)
  {
  }
  
  //----------------------------------------------------------------------------
  QWidget *AppositionSurfaceFilterInspector::createWidget(QUndoStack *stack, ViewManager *viewManager)
  {
    return new AppositionSurfaceFilterInspector::Widget(m_filter, m_model);
  }

  /// Filter Inspector Widget
  //
  //----------------------------------------------------------------------------
  AppositionSurfaceFilterInspector::Widget::Widget(FilterSPtr filter, EspinaModel *model)
  {
    setupUi(this);
    m_filter = dynamic_cast<AppositionSurfaceFilter *>(filter.get());
    m_origin->setText(m_filter->getOriginSegmentation());
    m_model = model;

    double area = -1;
    double perimeter = -1;
    double tortuosity = -1;

    if (m_model != NULL)
    {
      ModelItemSList list = m_model->relatedItems(m_filter, RELATION_OUT, Filter::CREATELINK);
      Q_ASSERT(!list.empty());
      SegmentationSPtr seg = segmentationPtr(list.first());

      Segmentation::InformationExtension extension = seg->informationExtension(AppositionSurfaceExtension::ID);

      bool ok = false;
      area = extension->information(AppositionSurfaceExtension::AREA).toDouble(&ok);
      Q_ASSERT(ok == true);
      perimeter = extension->information(AppositionSurfaceExtension::PERIMETER).toDouble(&ok);
      Q_ASSERT(ok == true);
      tortuosity = extension->information(AppositionSurfaceExtension::TORTUOSITY).toDouble(&ok);
      Q_ASSERT(ok == true);
    }


    if (-1 != area)
      m_area->setText(QString::number(area));
    else
      m_area->setText("Not computed");

    if (-1 != perimeter)
      m_perimeter->setText(QString::number(perimeter));
    else
      m_perimeter->setText("Not computed");

    if (-1 != tortuosity)
      m_tortuosity->setText(QString::number(tortuosity));
    else
      m_tortuosity->setText("Not computed");
  }

} /* namespace EspINA */
