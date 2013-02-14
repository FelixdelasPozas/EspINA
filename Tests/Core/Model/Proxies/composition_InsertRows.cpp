#include <Core/Model/EspinaModel.h>
#include <Core/Model/Proxies/RelationProxy.h>
#include <Core/Model/Segmentation.h>

#include <Core/Model/ModelTest.h>
#include <Core/Relations.h>
#include <Tests/Core/Model/InsertRowsTest.h>

int composition_InsertRows(int argc, char** argv)
{
  EspINA::EspinaModel model(NULL);
  EspINA::RelationProxy compositionProxy;
  compositionProxy.setRelation(EspINA::Relations::COMPOSITION);
  compositionProxy.setSourceModel(&model);

  ModelTest modelTester(&compositionProxy);

  insertRowsTest(&model);
  insertRowsTest(&model);

  if (model.segmentations().size() != 2)
    return 1;

  return 0;
}
