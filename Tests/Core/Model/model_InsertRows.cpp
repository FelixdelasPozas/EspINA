#include <Core/Model/EspinaModel.h>
#include <Core/Model/Segmentation.h>

#include "InsertRowsTest.h"
#include <Core/Model/ModelTest.h>

int model_InsertRows(int argc, char** argv)
{
  EspinaModel model(NULL);
  ModelTest modelTester(&model);

  insertRowsTest(&model);

  return 0;
}
