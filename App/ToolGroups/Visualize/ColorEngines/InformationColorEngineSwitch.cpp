/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "InformationColorEngineSwitch.h"

#include <GUI/ColorEngines/InformationColorEngine.h>
#include <GUI/Widgets/ColorBar.h>
#include <GUI/Widgets/InformationSelector.h>
#include <GUI/Utils/Format.h>
#include <GUI/Utils/ColorRange.h>
#include <Core/Utils/ListUtils.hxx>
#include <QComboBox>
#include <QLabel>
#include <QLayout>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Utils;
using namespace ESPINA::GUI::Utils::Format;
using namespace ESPINA::Support;

class UpdateColorEngineTask
: public Task
{
public:
    explicit UpdateColorEngineTask(const QString    &type,
                             const QString          &info,
                             InformationColorEngine *colorEngine,
                             SegmentationAdapterList segmentations,
                             ModelFactorySPtr        factory,
                             SchedulerSPtr           scheduler)
    : Task(scheduler)
    , m_type(type)
    , m_info(info)
    , m_colorEngine(colorEngine)
    , m_segmentations(segmentations)
    , m_factory(factory)
    {
    }

private:
  virtual void run()
  {
    double min = 0;
    double max = 0;

    double i     = 0;
    double total = m_segmentations.size();
    for(auto segmentation : m_segmentations)
    {
      auto extension = retrieveOrCreateExtension(segmentation, m_type, m_factory);

      if (extension)
      {
        auto info = extension->information(m_info);

        if (info.isValid() && info.canConvert<double>())
        {
          auto value = info.toDouble();
          if (i > 0)
          {
            min = qMin(min, value);
            max = qMax(max, value);
          }
          else
          {
            min = max = value;
          }
        }
      }

      reportProgress(i++/total*100);
    }

    m_colorEngine->setInformation(m_info, min, max);
  }

private:
  const QString &m_type;
  const QString &m_info;
  InformationColorEngine *m_colorEngine;
  SegmentationAdapterList m_segmentations;
  ModelFactorySPtr m_factory;
};

//-----------------------------------------------------------------------------
InformationColorEngineSwitch::InformationColorEngineSwitch(Context& context)
: ColorEngineSwitch(std::make_shared<InformationColorEngine>(context), ":espina/color_engine_switch_property.svg", context)
, m_informationTag(informationColorEngine()->information())
{
  createPropertySelector();

  createColorRange();

  updateSettings();
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::createPropertySelector()
{
  auto label = new QLabel(tr("Color by:"));
  m_property = new QLabel();

  m_property->setOpenExternalLinks(false);

  connect(m_property, SIGNAL(linkActivated(QString)),
          this,       SLOT(changeProperty()));


  addSettingsWidget(label);
  addSettingsWidget(m_property);
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::createColorRange()
{
  auto colorBar = new ColorBar(informationColorEngine()->colorRange());

  colorBar->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
  colorBar->setFixedHeight(20);
  colorBar->setMinimumWidth(80);
  colorBar->setMaximumWidth(80);

  addSettingsWidget(colorBar);
}

//-----------------------------------------------------------------------------
InformationColorEngine* InformationColorEngineSwitch::informationColorEngine() const
{
  return dynamic_cast<InformationColorEngine *>(colorEngine().get());
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::updateSettings()
{
  m_property->setText(createLink(m_informationTag));
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::changeProperty()
{
  auto available = availableInformation(getFactory());

  auto selection = InformationSelector::GroupedInfo();

  InformationSelector propertySelector(available, selection, tr("Select property to color by"));

  if (propertySelector.exec() == QDialog::Accepted)
  {
    m_extensionType  = selection.keys().first();
    m_informationTag = selection[m_extensionType].first();

    auto segmentations = toRawList<SegmentationAdapter>(getModel()->segmentations());
    auto task = std::make_shared<UpdateColorEngineTask>(m_extensionType,
                                                        m_informationTag,
                                                        informationColorEngine(),
                                                        segmentations,
                                                        getFactory(),
                                                        getScheduler());
    showTaskProgress(task);

    Task::submit(task);

    updateSettings();
  }
}
