/*
 * loadModelFromSegFile.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// Espina
#include "common/model/EspinaModel.h"
#include "common/IO/EspinaIO.h"

// Qt
#include <QString>
#include <QFileInfo>
#include <QDir>

int loadModelFromSegFile(int argc, char** argv)
{
  QString filename = QString("../../test images/test1.seg");
  QFileInfo file(filename);

  ViewManager* vm = new ViewManager();
  QUndoStack *undo = new QUndoStack;
  EspinaFactory *factory = new EspinaFactory();
  EspinaModel *model = new EspinaModel(factory);
  SeedGrowSegmentation *seedFilterCreator = new SeedGrowSegmentation(model, undo, vm);
  factory->registerFilter(seedFilterCreator, SeedGrowSegmentationFilter::TYPE);
  QDir tmpDir = QDir(argv[1]);

  // check model
  if(EspinaIO::loadSegFile(file, model, tmpDir) == EspinaIO::SUCCESS)
    if ((model != NULL) && (model->segmentations().size() == 64) && (model->channels().size() == 1))
      return 1;

  return 0;
}


