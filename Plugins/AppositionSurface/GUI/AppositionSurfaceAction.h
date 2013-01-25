/*
 * AppositionSurfaceAction.h
 *
 *  Created on: Jan 16, 2013
 *      Author: Félix de las Pozas Álvarez
 */

#ifndef APPOSITIONSURFACEACTION_H_
#define APPOSITIONSURFACEACTION_H_

// Qt
#include <QAction>
#include <QUndoStack>

namespace EspINA
{
  class ViewManager;
  class EspinaModel;

  class AppositionSurfaceAction
  : public QAction
  {
      Q_OBJECT
    public:
      explicit AppositionSurfaceAction(ViewManager *vm,
                                       QUndoStack  *undo,
                                       EspinaModel *model,
                                       QObject     *parent = 0);
      virtual ~AppositionSurfaceAction();

    public slots:
      void computeASurfaces();

    private:
      ViewManager *m_viewManager;
      QUndoStack  *m_undoStack;
      EspinaModel *m_model;
  };

} /* namespace EspINA */
#endif /* APPOSITIONSURFACEACTION_H_ */
