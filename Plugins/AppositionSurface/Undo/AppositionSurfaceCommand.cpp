/*
 * AppositionSurfaceCommand.cpp
 *
 *  Created on: Jan 16, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "AppositionSurfaceCommand.h"
#include <Filter/AppositionSurfaceFilter.h>
#include <GUI/FilterInspector/AppositionSurfaceFilterInspector.h>
#include <Core/Extensions/AppositionSurfaceExtension.h>

// EspINA
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/EspinaModel.h>
#include <Core/EspinaTypes.h>
#include <Core/EspinaSettings.h>
#include <Core/Relations.h>
#include <GUI/ViewManager.h>

// Qt
#include <QApplication>

namespace EspINA
{
  const Filter::FilterType AppositionSurfaceCommand::FILTER_TYPE = "AppositionSurface::AppositionSurfaceFilter";

  //-----------------------------------------------------------------------------
  AppositionSurfaceCommand::AppositionSurfaceCommand(SegmentationList inputs,
                                                     EspinaModel     *model,
                                                     ViewManager     *vm)
  : m_model(model)
  , m_viewManager(vm)
  , m_taxonomy(m_model->taxonomy()->element(SAS))
  {
    Q_ASSERT(m_taxonomy);

    foreach(SegmentationPtr seg, inputs)
    {
      Filter::NamedInputs inputs;
      Filter::Arguments args;
      inputs[AppositionSurfaceFilter::INPUTLINK] = seg->filter();
      args[AppositionSurfaceFilter::ORIGIN] = seg->data().toString();
      args[Filter::INPUTS] = Filter::NamedInput(AppositionSurfaceFilter::INPUTLINK, seg->outputId());

      FilterSPtr filter(new AppositionSurfaceFilter(inputs, args, FILTER_TYPE));
      filter->update();

      Filter::FilterInspectorPtr filterInspector(new AppositionSurfaceFilterInspector(filter));
      filter->setFilterInspector(filterInspector);

      Q_ASSERT(filter->outputs().size() == 1);

      SegmentationSPtr asSegmentation = m_model->factory()->createSegmentation(filter, 0);
      asSegmentation->modifiedByUser(userName());
      asSegmentation->setInputSegmentationDependent(true);
      asSegmentation->setNumber(seg->number());

      m_samples         << seg->sample();
      m_channels        << seg->channel();
      m_filters         << filter;
      m_segmentations   << model->findSegmentation(seg);
      m_asSegmentations << asSegmentation;
    }
  }

  //-----------------------------------------------------------------------------
  void AppositionSurfaceCommand::redo()
  {
    for(int i = 0; i < m_filters.size(); ++i)
    {
      m_model->addFilter(m_filters[i]);
      m_model->addRelation(m_channels[i]->filter(), m_filters[i], AppositionSurfaceFilter::INPUTLINK);

      m_asSegmentations[i]->setTaxonomy(m_taxonomy);
      m_model->addSegmentation(m_asSegmentations[i]);

      m_model->addRelation(m_filters[i]      , m_asSegmentations[i], Filter::CREATELINK);
      m_model->addRelation(m_segmentations[i], m_asSegmentations[i], Relations::LOCATION);
      m_model->addRelation(m_channels[i]     , m_asSegmentations[i], Channel::LINK);
    }
  }
  //-----------------------------------------------------------------------------
  void AppositionSurfaceCommand::undo()
  {
    for(int i = 0; i < m_filters.size(); ++i)
    {
      m_model->removeRelation(m_channels[i]     , m_asSegmentations[i], Channel::LINK);
      m_model->removeRelation(m_segmentations[i], m_asSegmentations[i], Relations::LOCATION);
      m_model->removeRelation(m_filters[i]      , m_asSegmentations[i], Filter::CREATELINK);
      m_model->removeSegmentation(m_asSegmentations[i]);

      m_model->removeRelation(m_channels[i]->filter(), m_filters[i], AppositionSurfaceFilter::INPUTLINK);
      m_model->removeFilter(m_filters[i]);
    }
  }


} /* namespace EspINA */
