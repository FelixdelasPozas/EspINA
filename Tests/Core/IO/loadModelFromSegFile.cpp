/*
 * loadModelFromSegFile.cpp
 *
 *  Created on: Nov 21, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// Espina
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/IO/SegFileReader.h>
#include <Core/Interfaces/IFilterCreator.h>
#include <Core/Filters/SeedGrowSegmentationFilter.h>
#include <Core/Filters/ChannelReader.h>
#include <App/Undo/SeedGrowSegmentationCommand.h>

// Qt
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QDebug>

using namespace EspINA;

// must match seg file Filter::FilterType
const Filter::FilterType TEST_FILTER_TYPE = "SeedGrowSegmentation::SeedGrowSegmentationFilter";
const QString CHANNELREADER_TYPE = "Channel Reader";

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

class ChannelReaderCreator
: public IFilterCreator
{
  public:
    virtual ~ChannelReaderCreator() {};
    virtual FilterSPtr createFilter(const QString& filter, const Filter::NamedInputs& inputs, const ModelItem::Arguments& args)
    {
      return FilterSPtr(new ChannelReader(inputs, args, CHANNELREADER_TYPE, NULL));
    }
};

int loadModelFromSegFile(int argc, char** argv)
{
  QString filename = QString(argv[1]) + QString("test1.seg");
  QFileInfo file(filename);
  Q_ASSERT(file.exists());

  EspinaFactory *factory = new EspinaFactory();
  EspinaModel *model = new EspinaModel(factory);

  SeedGrowSegmentationCreator creator;
  factory->registerFilter(&creator, TEST_FILTER_TYPE);

  ChannelReaderCreator channelCreator;
  factory->registerFilter(&channelCreator, CHANNELREADER_TYPE);

  // check model
  if(SegFileReader::loadSegFile(file, model) != IOErrorHandler::SUCCESS)
  {
    qDebug() << "couldn't load test file";
    return 1;
  }

  if (model)
  {
    if ((model->segmentations().size() != 1) || (model->channels().size() != 1))
    {
      qDebug() << "wrong number of segmentations or channels";
      return 1;
    }
  }

  return 0;
}


