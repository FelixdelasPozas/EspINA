#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>

#include "InsertRowsTest.h"
#include <Core/Model/ModelTest.h>

int model_InsertRows(int argc, char** argv)
{
  EspINA::EspinaModel model(NULL);
  ModelTest modelTester(&model);

  insertRowsTest(&model);

  if (model.segmentations().size() != 1)
    return 1;

  return 0;
}
