/*
 * AppositionSurfaceCommand.h
 *
 *  Created on: Jan 16, 2013
 *      Author: F�lix de las Pozas �lvarez
 */

#ifndef APPOSITIONSURFACECOMMAND_H_
#define APPOSITIONSURFACECOMMAND_H_

#include "AppositionSurfacePlugin_Export.h"

// EspINA
#include <Core/EspinaTypes.h>
#include <GUI/Model/FilterAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <Support/ViewManager.h>

// Qt
#include <QUndoCommand>

namespace EspINA
{
  class AppositionSurfacePlugin_EXPORT AppositionSurfaceCommand
  : public QUndoCommand
  {
    public:
      explicit AppositionSurfaceCommand(const SegmentationAdapterList  inputs,
                                        const ModelAdapterSPtr         model,
                                        const ViewManagerSPtr          vm,
                                        SegmentationAdapterSList &createdSegmentations);
      virtual ~AppositionSurfaceCommand() {};

      virtual void redo();
      virtual void undo();

    private:
      ModelAdapterSPtr        m_model;
      ViewManagerSPtr         m_viewManager;
      CategoryAdapterSPtr     m_category;
      SegmentationAdapterList m_asSegmentations;
  };

} /* namespace EspINA */
#endif /* APPOSITIONSURFACECOMMAND_H_ */
