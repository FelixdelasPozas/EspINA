/*
 * AppositionSurfaceCommand.cpp
 *
 *  Created on: Jan 16, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#include "AppositionSurfaceCommand.h"
#include <Filter/AppositionSurfaceFilter.h>
#include <Core/Extensions/AppositionSurfaceExtension.h>

// EspINA
#include <GUI/Model/ModelAdapter.h>
#include <Core/EspinaTypes.h>
#include <Support/Settings/EspinaSettings.h>
#include <Core/Relations.h>
#include <Support/ViewManager.h>

// Qt
#include <QApplication>

namespace EspINA
{
  //-----------------------------------------------------------------------------
  AppositionSurfaceCommand::AppositionSurfaceCommand(const SegmentationAdapterList  inputs,
                                                     const ModelAdapterSPtr         model,
                                                     const ViewManagerSPtr          vm,
                                                     SegmentationAdapterSList &createdSegmentations)
  : m_model      {model}
  , m_viewManager{vm}
  , m_category   {m_model->classification()->category(SAS)}
  {
    Q_ASSERT(m_category);

    for(auto segmentation: inputs)
    {

    }
  }

  //-----------------------------------------------------------------------------
  void AppositionSurfaceCommand::redo()
  {
    for(auto segmentation: m_asSegmentations)
    {

      m_model->addFilter(m_filters[i]);
      m_model->addRelation(m_segmentations[i]->filter(), m_filters[i], AppositionSurfaceFilter::INPUTLINK);

      m_asSegmentations[i]->setTaxonomy(m_taxonomy);
      m_model->addSegmentation(m_asSegmentations[i]);

      m_model->addRelation(m_filters[i]      , m_asSegmentations[i], Filter::CREATELINK);
      m_model->addRelation(m_segmentations[i], m_asSegmentations[i], Relations::LOCATION);
      m_model->addRelation(m_segmentations[i], m_asSegmentations[i], AppositionSurfaceFilter::SAS);
      m_model->addRelation(m_channels[i]     , m_asSegmentations[i], Channel::LINK);
    }
  }
  //-----------------------------------------------------------------------------
  void AppositionSurfaceCommand::undo()
  {
    for(int i = 0; i < m_filters.size(); ++i)
    {
      m_model->removeRelation(m_channels[i]     , m_asSegmentations[i], Channel::LINK);
      m_model->removeRelation(m_segmentations[i], m_asSegmentations[i], AppositionSurfaceFilter::SAS);
      m_model->removeRelation(m_segmentations[i], m_asSegmentations[i], Relations::LOCATION);
      m_model->removeRelation(m_filters[i]      , m_asSegmentations[i], Filter::CREATELINK);
      m_model->removeSegmentation(m_asSegmentations[i]);

      m_model->removeRelation(m_segmentations[i]->filter(), m_filters[i], AppositionSurfaceFilter::INPUTLINK);
      m_model->removeFilter(m_filters[i]);
    }
  }


} /* namespace EspINA */
