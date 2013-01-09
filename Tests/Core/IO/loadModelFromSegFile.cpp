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
#include <App/Undo/SeedGrowSegmentationCommand.h>

// Qt
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

using namespace EspINA;

// must match seg file Filter::FilterType
const Filter::FilterType TEST_FILTER_TYPE = "SeedGrowSegmentation::SeedGrowSegmentationFilter";

class SeedGrowSegmentationCreator
: public IFilterCreator
{
public:
  virtual ~SeedGrowSegmentationCreator(){}
  virtual FilterSPtr createFilter(const QString& filter, const Filter::NamedInputs& inputs, const ModelItem::Arguments& args)
  {
    return FilterSPtr(new SeedGrowSegmentationFilter(inputs, args, TEST_FILTER_TYPE));
  }
};

int loadModelFromSegFile(int argc, char** argv)
{
  QString filename = QString(argv[1]) + QString("test1.seg");
  QFileInfo file(filename);

  EspinaFactory *factory = new EspinaFactory();
  EspinaModel *model = new EspinaModel(factory);

  SeedGrowSegmentationCreator creator;
  factory->registerFilter(&creator, TEST_FILTER_TYPE);

  // check model
  if(EspinaIO::loadSegFile(file, model, QDir::current()) != EspinaIO::SUCCESS)
  {
    qDebug() << "couldn't load test file";
    return 1;
  }

  if (model)
  {
    if ((model->segmentations().size() != 64) || (model->channels().size() != 1))
    {
      qDebug() << "wrong number of segmentations or channels";
      return 1;
    }
  }

  return 0;
}


