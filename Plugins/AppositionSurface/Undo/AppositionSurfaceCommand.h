/*
 * AppositionSurfaceCommand.h
 *
 *  Created on: Jan 16, 2013
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef APPOSITIONSURFACECOMMAND_H_
#define APPOSITIONSURFACECOMMAND_H_

#include "AppositionSurfacePlugin_Export.h"

// EspINA
#include <Core/Model/Filter.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/Segmentation.h>

// Qt
#include <QUndoCommand>

namespace EspINA
{
  class ModelAdapter;
  class ViewManager;

  class AppositionSurfacePlugin_EXPORT AppositionSurfaceCommand
  : public QUndoCommand
  {
    public:
      static const Filter::FilterType FILTER_TYPE;

    public:
      explicit AppositionSurfaceCommand(SegmentationList   inputs,
                                        ModelAdapter       *model,
                                        ViewManager       *vm,
                                        SegmentationSList &createdSegmentations);
      virtual ~AppositionSurfaceCommand() {};

      virtual void redo();
      virtual void undo();

    private:
      ModelAdapter *m_model;
      ViewManager *m_viewManager;

      TaxonomyElementSPtr m_taxonomy;
      SampleSList      m_samples;
      ChannelSList        m_channels;
      FilterSList      m_filters;
      SegmentationSList   m_segmentations;
      SegmentationSList   m_asSegmentations;
  };

} /* namespace EspINA */
#endif /* APPOSITIONSURFACECOMMAND_H_ */
