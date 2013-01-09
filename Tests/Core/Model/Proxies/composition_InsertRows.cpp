#include <Core/Model/EspinaModel.h>
#include <Core/Model/Proxies/CompositionProxy.h>
#include <Core/Model/Segmentation.h>

#include <Core/Model/ModelTest.h>
#include <Tests/Core/Model/InsertRowsTest.h>

int composition_InsertRows(int argc, char** argv)
{
  EspINA::EspinaModel model(NULL);
  EspINA::CompositionProxy compositionProxy;
  compositionProxy.setSourceModel(&model);

  ModelTest modelTester(&compositionProxy);

  insertRowsTest(&model);
  insertRowsTest(&model);

  if (model.segmentations().size() != 2)
    return 1;

  return 0;
}
