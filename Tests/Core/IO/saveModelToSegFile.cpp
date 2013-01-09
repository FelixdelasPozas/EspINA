/*
 * saveModelToSegFile.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// Espina
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/IO/EspinaIO.h>
#include <GUI/ViewManager.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Filters/SeedGrowSegmentationFilter.h>
#include <App/Undo/SeedGrowSegmentationCommand.h>

// Qt
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QUndoStack>

using namespace EspINA;

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

int saveModelToSegFile(int argc, char** argv)
{
  QString filename1 = QString(argv[1]) + QString("test1.seg");
  QFileInfo file(filename1);

  EspinaFactory *factory = new EspinaFactory();
  EspinaModel *model = new EspinaModel(factory);
  SeedGrowSegmentationCreator seedFilterCreator;
  factory->registerFilter(&seedFilterCreator, TEST_FILTER_TYPE);

  if (EspinaIO::loadSegFile(file, model, QDir::current()) != EspinaIO::SUCCESS)
    return 1;

  QString filename2 = QString("test2.seg");
  QFileInfo file2(filename2);

  if (EspinaIO::saveSegFile(file2, model) != EspinaIO::SUCCESS)
    return 1;

  if (std::system(NULL))
    std::system("rm -f test2.seg");

  return 0;
}
