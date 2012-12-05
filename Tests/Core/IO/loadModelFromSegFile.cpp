/*
 * loadModelFromSegFile.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// Espina
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/IO/EspinaIO.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Filters/SeedGrowSegmentationFilter.h>

// Qt
#include <QString>
#include <QFileInfo>
#include <QDir>

class SeedGrowSegmentationCreator
: public IFilterCreator
{
public:
  virtual ~SeedGrowSegmentationCreator(){}
  virtual Filter* createFilter(const QString& filter, const Filter::NamedInputs& inputs, const ModelItem::Arguments& args)
  {
    return new SeedGrowSegmentationFilter(inputs, args);
  }
};

int loadModelFromSegFile(int argc, char** argv)
{
  QString filename = QString("../../test images/test1.seg");
  QFileInfo file(filename);

  EspinaFactory *factory = new EspinaFactory();
  EspinaModel *model = new EspinaModel(factory);

  SeedGrowSegmentationCreator creator;
  factory->registerFilter(&creator, SeedGrowSegmentationFilter::TYPE);

  QDir tmpDir = QDir(argv[1]);
  // check model
  if(EspinaIO::loadSegFile(file, model, tmpDir) != EspinaIO::SUCCESS)
    return 1;

  if (model)
    if ((model->segmentations().size() != 64) || (model->channels().size() != 1))
      return 1;

  return 0;
}


