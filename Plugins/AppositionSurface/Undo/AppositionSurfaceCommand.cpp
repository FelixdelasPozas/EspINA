/*
 * AppositionSurfaceCommand.cpp
 *
 *  Created on: Jan 16, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "AppositionSurfaceCommand.h"
#include <Filter/AppositionSurfaceFilter.h>

// EspINA
#include <Core/Model/EspinaModel.h>
#include <Core/EspinaTypes.h>
#include <Core/EspinaSettings.h>
#include <GUI/ViewManager.h>

// Qt
#include <QApplication>

namespace EspINA
{
  const Filter::FilterType AppositionSurfaceCommand::FILTER_TYPE = "AppositionSurface::AppositionSurfaceFilter";

  //-----------------------------------------------------------------------------
  AppositionSurfaceCommand::AppositionSurfaceCommand(SegmentationList inputs, EspinaModel *model, ViewManager *vm)
  : m_model(model)
  , m_viewManager(vm)
  , m_createPSDTaxonomy(false)
  , m_psdElement(NULL)
  {
    // when this filter is called from the tests there is no qApp, as there isn't a gui
    if (QApplication::instance() != NULL)
      QApplication::setOverrideCursor(Qt::WaitCursor);

    m_psdElement = m_model->taxonomy()->element(QString("PSD"));
    if (NULL == m_psdElement)
      m_createPSDTaxonomy = true;

    foreach(SegmentationPtr seg, inputs)
    {
      Filter::NamedInputs inputs;
      Filter::Arguments args;
      inputs[AppositionSurfaceFilter::INPUTLINK] = seg->filter();
      args[AppositionSurfaceFilter::ORIGIN] = seg->information("Name").toString();
      args[Filter::INPUTS] = Filter::NamedInput(AppositionSurfaceFilter::INPUTLINK, seg->outputId());
      FilterSPtr filter(new AppositionSurfaceFilter(inputs, args, FILTER_TYPE));
      filter->update();

      SegmentationSPtr filterSeg = m_model->factory()->createSegmentation(filter, 0);
      filterSeg->modifiedByUser(userName());

      m_segmentations << filterSeg;
      m_channels      << seg->channel();
      m_samples       << seg->sample();
      m_filters       << filter;
    }

    if (QApplication::instance() != NULL)
      QApplication::restoreOverrideCursor();
  }
  
  //-----------------------------------------------------------------------------
  void AppositionSurfaceCommand::undo()
  {
    for (int i = 0; i < m_segmentations.size(); ++i)
    {
      foreach(Relation relation, m_segmentations[i]->relations())
        m_model->removeRelation(relation.ancestor, relation.succesor, relation.relation);

      foreach(Relation relation, m_filters[i]->relations())
        m_model->removeRelation(relation.ancestor, relation.succesor, relation.relation);
    }

    m_model->removeSegmentation(m_segmentations);
    foreach(FilterSPtr filter, m_filters)
      m_model->removeFilter(filter);

    if (m_createPSDTaxonomy)
      m_model->taxonomy()->deleteElement(m_psdElement);
  }

  //-----------------------------------------------------------------------------
  void AppositionSurfaceCommand::redo()
  {
    if (m_createPSDTaxonomy)
    {
      m_psdElement = m_model->taxonomy()->createElement(QString("PSD"), m_model->taxonomy()->root());
      m_psdElement->setColor(QColor(255,255,0));
    }

    foreach(SegmentationSPtr seg, m_segmentations)
      seg->setTaxonomy(m_psdElement);

    m_model->addSegmentation(m_segmentations);
    m_model->addFilter(m_filters);

    /////////////////////////////////////
    // BUG: taxonomy add element bug here
    /////////////////////////////////////

    for (int i = 0; i < m_segmentations.size(); ++i)
    {
      m_model->addRelation(m_filters[i], m_segmentations[i], Filter::CREATELINK);
      m_model->addRelation(m_samples[i], m_segmentations[i], Sample::WHERE);
      m_model->addRelation(m_channels[i], m_segmentations[i], Channel::LINK);
    }
  }

} /* namespace EspINA */
