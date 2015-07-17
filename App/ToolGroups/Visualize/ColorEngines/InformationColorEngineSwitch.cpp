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
#include <GUI/Widgets/Styles.h>
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
    explicit UpdateColorEngineTask(const SegmentationExtension::InformationKey &key,
                                   InformationColorEngine *colorEngine,
                                   SegmentationAdapterList segmentations,
                                   ModelFactorySPtr        factory,
                                   SchedulerSPtr           scheduler)
    : Task(scheduler)
    , m_key(key)
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
      if (!canExecute()) return;

      try
      {
        auto extension = retrieveOrCreateExtension(segmentation, m_key.extension(), m_factory);

        auto info = extension->information(m_key);

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
      catch(SegmentationExtension::Extension_Not_Found &e)
      {
        qWarning() << "Information not available";
      }

      reportProgress(i++/total*100);
    }

    m_colorEngine->setInformation(m_key, min, max);
  }

private:
  const SegmentationExtension::InformationKey &m_key;
  InformationColorEngine *m_colorEngine;
  SegmentationAdapterList m_segmentations;
  ModelFactorySPtr m_factory;
};

//-----------------------------------------------------------------------------
InformationColorEngineSwitch::InformationColorEngineSwitch(Context& context)
: ColorEngineSwitch(std::make_shared<InformationColorEngine>(), ":espina/color_engine_switch_property.svg", context)
, m_key(informationColorEngine()->information())
{
  createPropertySelector();

  createColorRange();

  updateSettings();

  connect(getModel().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
          this,             SLOT(updateRange()));

  connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
          this,             SLOT(updateRange()));
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

  Styles::setBarStyle(colorBar);

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
  m_property->setText(createLink(m_key.value()));
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::changeProperty()
{
  auto available = availableInformation(getFactory());

  auto selection = InformationSelector::GroupedInfo();

  InformationSelector propertySelector(available, selection, tr("Select property to color by"));

  if (propertySelector.exec() == QDialog::Accepted)
  {
    auto extension  = selection.keys().first();
    auto value      = selection[extension].first();

    m_key = InformationKey(extension, value);

    updateRange();

    updateSettings();
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::updateRange()
{
  if (m_task && m_task->isRunning())
  {
    m_task->abort();
  }

  auto segmentations = toRawList<SegmentationAdapter>(getModel()->segmentations());

  m_task = std::make_shared<UpdateColorEngineTask>(m_key,
                                                   informationColorEngine(),
                                                   segmentations,
                                                   getFactory(),
                                                   getScheduler());
  showTaskProgress(m_task);

  Task::submit(m_task);
}
