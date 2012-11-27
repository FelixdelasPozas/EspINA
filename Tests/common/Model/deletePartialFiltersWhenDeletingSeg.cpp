/*
 * deleteFiltersWhenDeletingSeg-Partial.cpp
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
#include "common/EspinaRegions.h"
#include "common/editor/split/SplitFilter.h"
#include "frontend/toolbar/editor/split/SplitUndoCommand.h"
#include "common/model/EspinaFactory.h"

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

int deletePartialFiltersWhenDeletingSeg(int argc, char** argv)
{
  QString filename1 = QString(argv[1]) + QString("../../test images/test1.seg");
  QFileInfo file(filename1);
  EspinaModel *model = NULL;
  QDir fileDir = QString(argv[1]) + QString("../../test images/");
  EspinaIO::loadSegFile(file, model, fileDir);

  QUndoStack *undo = new QUndoStack();

  Segmentation *seg = model->segmentations().at(0);
  Filter *filter1 = seg->filter();
  double bounds[6];
  VolumeBounds(seg->itkVolume(), bounds);
  double center[3] = { (bounds[0]+bounds[1])/2, (bounds[2]+bounds[3])/2, (bounds[4]+bounds[5])/2 };

  vtkPlane *plane = vtkPlane::New();
  plane->SetOrigin(center);
  plane->SetNormal(0,1,0);

  EspinaVolume::PointType origin = seg->itkVolume()->GetOrigin();
  EspinaVolume::SpacingType spacing = seg->itkVolume()->GetSpacing();
  int segExtent[6];
  VolumeExtent(seg->itkVolume(), segExtent);

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

  inputs[SplitFilter::INPUTLINK] = seg->filter();
  args[Filter::INPUTS] = args.namedInput(SplitFilter::INPUTLINK, seg->outputNumber());

  SplitFilter *splitter = new SplitFilter(inputs, args);
  splitter->setStencil(stencil);
  splitter->update();
  Q_ASSERT(splitter->numberOutputs() == 2);
  Segmentation  *splitSeg[2];
  EspinaFactory *factory = model->factory();
  splitSeg[0] = factory->createSegmentation(splitter, 0);
  splitSeg[1] = factory->createSegmentation(splitter, 1);
  undo->push(new SplitUndoCommand(seg, splitter, splitSeg, model));

  // make the one we will delete a multiple filter segmentation
  undo->push(new EditorToolBar::CODECommand(splitSeg[0], EditorToolBar::CODECommand::CLOSE, 3, model));
  Filter *filter2 = seg->filter();

  // remove and test
  RemoveSegmentation(seg);
  if (model->filters().contains(filter1) && model->filters().contains(splitter) && !model->filters().contains(filter2))
    return 1;

  return 0;
}


