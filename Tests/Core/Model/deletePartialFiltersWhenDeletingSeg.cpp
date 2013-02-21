/*
 * deleteFiltersWhenDeletingSeg-Partial.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// Espina
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>
#include <Core/Model/ModelItem.h>
#include <Core/Model/RelationshipGraph.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/IO/EspinaIO.h>
#include <Undo/RemoveSegmentation.h>
#include <Core/EspinaRegion.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Filters/SeedGrowSegmentationFilter.h>
#include <App/Undo/SeedGrowSegmentationCommand.h>

// Qt
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QUndoStack>
#include <QUndoCommand>

// vtk
#include <vtkPlane.h>
#include <vtkImageStencilData.h>
#include <vtkImplicitFunctionToImageStencil.h>

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


int deletePartialFiltersWhenDeletingSeg(int argc, char** argv)
{
  QString filename1 = QString(argv[1]) + QString("test1.seg");
  QFileInfo file(filename1);

  EspinaFactory *factory = new EspinaFactory();
  EspinaModel   *model   = new EspinaModel(factory);
  SeedGrowSegmentationCreator creator;
  factory->registerFilter(&creator, TEST_FILTER_TYPE);

  if (EspinaIO::loadSegFile(file, model) != EspinaIO::SUCCESS)
  {
    qDebug() << "couldn't load test file";
    return 1;
  }

  SegmentationPtr seg = model->segmentations().at(0).data();
  FilterSPtr filter1 = seg->filter();

  double bounds[6];
  seg->volume()->bounds(bounds);
  double center[3] = { (bounds[0]+bounds[1])/2, (bounds[2]+bounds[3])/2, (bounds[4]+bounds[5])/2 };

  vtkPlane *plane = vtkPlane::New();
  plane->SetOrigin(center);
  plane->SetNormal(0,1,0);


  itkVolumeType::PointType origin = seg->volume()->toITK()->GetOrigin();
  itkVolumeType::SpacingType spacing = seg->volume()->toITK()->GetSpacing();
  int segExtent[6];
  seg->volume()->extent(segExtent);

  vtkImplicitFunctionToImageStencil *plane2stencil = vtkImplicitFunctionToImageStencil::New();
  plane2stencil->SetInput(plane);
  plane2stencil->SetOutputOrigin(0, 0, 0);
  plane2stencil->SetOutputSpacing(spacing[0], spacing[1], spacing[2]);
  plane2stencil->SetOutputWholeExtent(segExtent);
  plane2stencil->Update();

  vtkImageStencilData *stencil = vtkImageStencilData::New();
  stencil = plane2stencil->GetOutput();

  Filter::NamedInputs inputs;
  Filter::Arguments   args;

//  inputs[SplitFilter::INPUTLINK] = seg->filter();
//  args[Filter::INPUTS] = args.namedInput(SplitFilter::INPUTLINK, seg->outputId());
//
//  SplitFilter *splitter = new SplitFilter(inputs, args);
//  splitter->setStencil(stencil);
//  splitter->update();
//  Q_ASSERT(splitter->numberOutputs() == 2);
//  Segmentation  *splitSeg[2];
//  EspinaFactory *factory = model->factory();
//  splitSeg[0] = factory->createSegmentation(splitter, 0);
//  splitSeg[1] = factory->createSegmentation(splitter, 1);
//  undo->push(new SplitUndoCommand(seg, splitter, splitSeg, model));
//
//  // make the one we will delete a multiple filter segmentation
//  undo->push(new EditorToolBar::CODECommand(splitSeg[0], EditorToolBar::CODECommand::CLOSE, 3, model));
//  Filter *filter2 = seg->filter();
//
//  // remove and test
//  SegmentationList list;
//  list.append(seg);
//  undo->push(new RemoveSegmentation(list, model));
//  if (model->filters().contains(filter1) && model->filters().contains(splitter) && !model->filters().contains(filter2))
//    return 1;

  return 0;
}


