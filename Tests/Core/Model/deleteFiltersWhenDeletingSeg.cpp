/*
 * deleteFiltersWhenDeletingSeg.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// Espina
#include "common/model/EspinaModel.h"
#include "common/model/Segmentation.h"
#include "common/model/ModelItem.h"
#include "common/model/RelationshipGraph.h"
#include "common/IO/EspinaIO.h"
#include "common/undo/RemoveSegmentation.h"
#include "frontend/toolbar/editor/EditorToolBar.h"

// Qt
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QUndoStack>
#include <QUndoCommand>

int deleteFiltersWhenDeletingSeg(int argc, char** argv)
{
  QString filename1 = QString(argv[1]) + QString("../../test images/test1.seg");
  QFileInfo file(filename1);
  EspinaModel *model = NULL;
  QDir fileDir = QString(argv[1]) + QString("../../test images/");
  EspinaIO::loadSegFile(file, model, fileDir);
  QUndoStack *undo = new QUndoStack();
  Segmentation *seg = model->segmentations().at(0);

  // make it a multi-filter segmentation
  Filter *filter1 = seg->filter();
  undo->push(new EditorToolBar::CODECommand(seg, EditorToolBar::CODECommand::CLOSE, 3, model));
  Filter *filter2 = seg->filter();

  // remove and test
  RemoveSegmentation(seg);
  if (!model->filters().contains(filter1) && !model->filters().contains(filter2))
    return 1;

  return 0;
}
