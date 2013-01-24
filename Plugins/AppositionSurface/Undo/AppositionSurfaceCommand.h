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
      AppositionSurfaceCommand(SegmentationList inputs, EspinaModel *model, ViewManager *vm);
      virtual ~AppositionSurfaceCommand() {};

      virtual void redo();
      virtual void undo();

    private:
      EspinaModel *m_model;
      ViewManager *m_viewManager;
      bool         m_createPSDTaxonomy;

      SegmentationSList   m_segmentations;
      ChannelSList        m_channels;
      SampleSPtrList      m_samples;
      FilterSPtrList      m_filters;
      TaxonomyElementSPtr m_psdElement;
  };

} /* namespace EspINA */
#endif /* APPOSITIONSURFACECOMMAND_H_ */
