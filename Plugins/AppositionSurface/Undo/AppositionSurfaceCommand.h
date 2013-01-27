/*
 * AppositionSurfaceCommand.h
 *
 *  Created on: Jan 16, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef APPOSITIONSURFACECOMMAND_H_
#define APPOSITIONSURFACECOMMAND_H_

// EspINA
#include <Core/Model/Filter.h>
#include <Core/EspinaTypes.h>
#include <Core/Model/Segmentation.h>

// Qt
#include <QUndoCommand>

namespace EspINA
{
  class EspinaModel;
  class ViewManager;

  class AppositionSurfaceCommand
  : public QUndoCommand
  {
    public:
      static const Filter::FilterType FILTER_TYPE;

    public:
      explicit AppositionSurfaceCommand(SegmentationList inputs, EspinaModel *model, ViewManager *vm);
      virtual ~AppositionSurfaceCommand() {};

      virtual void redo();
      virtual void undo();

    private:
      EspinaModel *m_model;
      ViewManager *m_viewManager;

      TaxonomyElementSPtr m_taxonomy;
      SampleSPtrList      m_samples;
      ChannelSList        m_channels;
      FilterSPtrList      m_filters;
      SegmentationSList   m_segmentations;
  };

} /* namespace EspINA */
#endif /* APPOSITIONSURFACECOMMAND_H_ */
