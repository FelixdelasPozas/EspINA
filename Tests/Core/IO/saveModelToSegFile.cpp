/*
 * saveModelToSegFile.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// Espina
#include "common/model/EspinaModel.h"
#include "common/model/EspinaFactory.h"
#include "common/IO/EspinaIO.h"
#include "common/gui/ViewManager.h"
#include "frontend/toolbar/seedgrow/SeedGrowSegmentation.h"

// Qt
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QUndoStack>

int saveModelToSegFile(int argc, char** argv)
{
  QString filename1 = QString("../../test images/test1.seg");
  QFileInfo file(filename1);

  ViewManager* vm = new ViewManager();
  QUndoStack *undo = new QUndoStack;
  EspinaFactory *factory = new EspinaFactory();
  EspinaModel *model = new EspinaModel(factory);
  SeedGrowSegmentation *seedFilterCreator = new SeedGrowSegmentation(model, undo, vm);
  factory->registerFilter(seedFilterCreator, SeedGrowSegmentationFilter::TYPE);
  QDir fileDir(QString(argv[1]));

  if(EspinaIO::loadSegFile(file, model, fileDir) == EspinaIO::SUCCESS)
  {
    QString filename2 = QString(argv[1]) + QString("../../test images/test2.seg");
    QFileInfo file2(filename2);
    if (EspinaIO::saveSegFile(file2, model) == EspinaIO::SUCCESS)
      return 1;
  }

  return 0;
}
