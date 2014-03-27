/*
 * AppositionSurfaceAction.h
 *
 *  Created on: Jan 16, 2013
 *      Author: Felix de las Pozas Alvarez
 */

#ifndef APPOSITIONSURFACEACTION_H_
#define APPOSITIONSURFACEACTION_H_

#include "AppositionSurfacePlugin_Export.h"

// Qt
#include <QAction>
#include <QUndoStack>

namespace EspINA
{
  class ViewManager;
  class ModelAdapter;

  class AppositionSurfacePlugin_EXPORT AppositionSurfaceAction
  : public QAction
  {
      Q_OBJECT
    public:
      explicit AppositionSurfaceAction(ViewManager *vm,
                                       QUndoStack  *undo,
                                       ModelAdapter *model,
                                       QObject     *parent = 0);
      virtual ~AppositionSurfaceAction();

    public slots:
      void computeASurfaces();

    private:
      ViewManager *m_viewManager;
      QUndoStack  *m_undoStack;
      ModelAdapter *m_model;
  };

} /* namespace EspINA */
#endif /* APPOSITIONSURFACEACTION_H_ */
