/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "SeedGrowSegmentationCommand.h"
#include <FilterInspectors/SeedGrowSegmentation/SGSFilterInspector.h>

#include <Filters/SeedGrowSegmentationFilter.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/EspinaSettings.h>
#include <QMessageBox>

using namespace EspINA;

const QString SGS_VOI = "SGS VOI";

//-----------------------------------------------------------------------------
SeedGrowSegmentationCommand::SeedGrowSegmentationCommand(ChannelPtr               channel,
                                                         itkVolumeType::IndexType seed,
                                                         int                      voiExtent[6],
                                                         int                      lowerThreshold,
                                                         int                      upperThreshold,
                                                         bool                     applyClosing,
                                                         TaxonomyElementPtr       taxonomy,
                                                         EspinaModel          *model,
                                                         QUndoCommand *           parent)
: QUndoCommand(parent)
, m_model   (model)
, m_sample  (channel->sample())
, m_channel (m_model->findChannel(channel))
, m_taxonomy(m_model->findTaxonomyElement(taxonomy))
{
  Filter::NamedInputs inputs;
  Filter::Arguments   args;

  SeedGrowSegmentationFilter::Parameters params(args);
  params.setSeed(seed);
  params.setLowerThreshold(lowerThreshold);
  params.setUpperThreshold(upperThreshold);
  params.setVOI(voiExtent);
  params.setCloseValue(applyClosing);

  inputs[SeedGrowSegmentationFilter::INPUTLINK] = channel->filter();
  args[Filter::INPUTS] = Filter::NamedInput(SeedGrowSegmentationFilter::INPUTLINK, channel->outputId());

  SeedGrowSegmentationFilter *sgsFilter = new SeedGrowSegmentationFilter(inputs, args);
  sgsFilter->update();
  Q_ASSERT(sgsFilter->outputs().size() == 1);

  Filter::FilterInspectorPtr inspector(new SGSFilterInspector(sgsFilter));
  sgsFilter->setFilterInspector(inspector);

  m_filter = FilterSPtr(sgsFilter);

  m_segmentation = m_model->factory()->createSegmentation(m_filter, 0);
  m_segmentation->setTaxonomy(m_taxonomy);

  int segExtent[6];
  m_segmentation->volume()->extent(segExtent);

  bool incompleteSeg = false;
  for (int i=0, j=1; i<6; i+=2, j+=2)
  {
    if (segExtent[i] <= voiExtent[i] || voiExtent[j] <= segExtent[j])
      incompleteSeg = true;
  }

  if (incompleteSeg)
  {
    QMessageBox warning;
    warning.setIcon(QMessageBox::Warning);
    warning.setWindowTitle(QObject::tr("Seed Grow Segmentation Filter Information"));
    warning.setText(QObject::tr("New segmentation may be incomplete due to VOI restriction."));
    warning.exec();
    QString condition = QObject::tr("Touch VOI");
    m_segmentation->addCondition(SGS_VOI, ":/espina/voi.svg", condition);
  }

  m_segmentation->modifiedByUser(userName());
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationCommand::~SeedGrowSegmentationCommand()
{
  qDebug() << ">>>>    Destroying SeedGrowSegmentationCommand   <<<<<<<";
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationCommand::redo()
{
  m_model->addFilter(m_filter);
  m_model->addRelation(m_channel->filter(), m_filter, SeedGrowSegmentationFilter::INPUTLINK);

  m_segmentation->setTaxonomy(m_taxonomy);
  m_model->addSegmentation(m_segmentation);
  m_model->addRelation(m_filter , m_segmentation, Filter::CREATELINK);
  m_model->addRelation(m_sample , m_segmentation, Sample::WHERE     );
  m_model->addRelation(m_channel, m_segmentation, Channel::LINK     );

  m_segmentation->initializeExtensions();
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationCommand::undo()
{
  m_model->removeRelation(m_channel->filter(), m_segmentation, SeedGrowSegmentationFilter::INPUTLINK);
  m_model->removeRelation(m_sample, m_segmentation, Sample::WHERE);
  m_model->removeRelation(m_filter, m_segmentation, Filter::CREATELINK);
  m_model->removeSegmentation(m_segmentation);

  m_model->removeRelation(m_channel, m_filter, Channel::LINK);
  m_model->removeFilter(m_filter);
}
