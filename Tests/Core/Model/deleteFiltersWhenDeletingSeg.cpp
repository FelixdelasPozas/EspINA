/*
 * deleteFiltersWhenDeletingSeg.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// Espina
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/ModelItem.h>
#include <Core/Model/RelationshipGraph.h>
#include <Core/IO/SegFileReader.h>
#include <Undo/RemoveSegmentation.h>
#include <Undo/FillHolesCommand.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Core/Filters/SeedGrowSegmentationFilter.h>
#include <App/Undo/SeedGrowSegmentationCommand.h>
#include <GUI/ViewManager.h>

// Qt
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QUndoStack>
#include <QUndoCommand>

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

int deleteFiltersWhenDeletingSeg(int argc, char** argv)
{
  return 1;

  // test fails to compile due dependecies with fillholescommand (needs GUI linked)
//  QString filename1 = QString(argv[1]) + QString("test1.seg");
//  QFileInfo file(filename1);
//
//  EspinaFactory *factory = new EspinaFactory();
//  EspinaModel *model = new EspinaModel(factory);
//  QUndoStack *undo = new QUndoStack();
//  SeedGrowSegmentationCreator creator;
//  factory->registerFilter(&creator, TEST_FILTER_TYPE);
//
//  if (EspinaIO::loadSegFile(file, model, QDir::current()) != EspinaIO::SUCCESS)
//  {
//    qDebug() << "couldn't load test file";
//    return 1;
//  }
//
//  SegmentationPtr seg = model->segmentations().at(0).data();
//  SegmentationList list;
//  list.append(seg);
//
//  // make it a multi-filter segmentation
//  FilterSPtr filter1 = seg->filter();
//  undo->push(new FillHolesCommand(list, model, NULL));
//  FilterSPtr filter2 = seg->filter();
//  Q_ASSERT(model->filters().contains(filter1) && model->filters().contains(filter2));
//
//  // remove and test
//  int numSegmentations = model->segmentations().size();
//  undo->push(new RemoveSegmentation(list, model));
//
//  if (model->filters().contains(filter1) || model->filters().contains(filter2)
//      || model->segmentations().size() != numSegmentations - 1)
//  {
//    qDebug() << "segmentation and filters hasn't been correctly deleted";
//    return 1;
//  }
//
//  return 0;
}
