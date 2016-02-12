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

// ESPINA
#include "InformationColorEngineSwitch.h"
#include <GUI/ColorEngines/InformationColorEngine.h>
#include <GUI/Widgets/ColorBar.h>
#include <GUI/Widgets/InformationSelector.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Utils/Format.h>
#include <GUI/Utils/ColorRange.h>
#include <Core/Utils/ListUtils.hxx>
#include <Core/Utils/EspinaException.h>

// Qt
#include <QComboBox>
#include <QLabel>
#include <QLayout>

using namespace ESPINA;
using namespace ESPINA::Core::Analysis;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Utils;
using namespace ESPINA::GUI::Utils::Format;
using namespace ESPINA::Support;


const QString EXTENSION_KEY   = "Extension";
const QString INFORMATION_KEY = "Key";

//-----------------------------------------------------------------------------
UpdateColorEngineTask::UpdateColorEngineTask(const SegmentationExtension::InformationKey key,
                                             InformationColorEngine                     *colorEngine,
                                             SegmentationAdapterList                     segmentations,
                                             ModelFactorySPtr                            factory,
                                             SchedulerSPtr                               scheduler)
: Task           {scheduler}
, m_key          {key}
, m_colorEngine  {colorEngine}
, m_segmentations{segmentations}
, m_factory      {factory}
, m_error        {"No error"}
, m_failed       {false}
{
  setDescription(tr("Coloring by %1").arg(m_key.value()));
}

//-----------------------------------------------------------------------------
void UpdateColorEngineTask::run()
{
  double min = 0;
  double max = 0;

  double i     = 0;
  double total = m_segmentations.size();

  for(auto segmentation : m_segmentations)
  {
    if (!canExecute() || m_failed) return;

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

      reportProgress(i++/total*100);
    }
    catch(const EspinaException &e)
    {
      m_failed = true;
      m_error = e.details();

      reportProgress(100);
    }
  }

  if(!m_failed)
  {
    m_colorEngine->setInformation(m_key, min, max);
  }
}

//-----------------------------------------------------------------------------
InformationColorEngineSwitch::InformationColorEngineSwitch(Context& context)
: ColorEngineSwitch(std::make_shared<InformationColorEngine>(), ":espina/color_engine_switch_property.svg", context)
, m_key(informationColorEngine()->information())
, m_needUpdate(true)
{
  createPropertySelector();

  createColorRange();

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(onToolToggled(bool)));
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::restoreSettings(std::shared_ptr< QSettings > settings)
{
  ColorEngineSwitch::restoreSettings(settings);

  auto extension = settings->value(EXTENSION_KEY,   "MorphologicalInformation").toString();
  auto key       = settings->value(INFORMATION_KEY, "Size").toString();

  m_key = InformationKey(extension, key);
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::saveSettings(std::shared_ptr< QSettings > settings)
{
  ColorEngineSwitch::saveSettings(settings);

  settings->setValue(EXTENSION_KEY,   m_key.extension());
  settings->setValue(INFORMATION_KEY, m_key.value());
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
void InformationColorEngineSwitch::update()
{
  updateRange();

  updateLink();

  m_needUpdate = false;
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::updateLink()
{
  m_property->setText(createLink(m_key.value()));
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::changeProperty()
{
  auto segmentations = toRawList<SegmentationAdapter>(getModel()->segmentations());
  auto available     = availableInformation(segmentations, getFactory());

  auto selection = InformationSelector::GroupedInfo();

  selection[m_key.extension()] << m_key;

  InformationSelector propertySelector(available, selection, tr("Select property to color by"), true);

  if (propertySelector.exec() == QDialog::Accepted)
  {
    auto extension  = selection.keys().first();
    auto value      = selection[extension].first();

    m_key = InformationKey(extension, value);

    m_needUpdate = true;

    update();
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

  connect(m_task.get(), SIGNAL(finished()), this, SLOT(onTaskFinished()));

  Task::submit(m_task);
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onTaskFinished()
{
  auto task = qobject_cast<UpdateColorEngineTask *>(sender());

  if(task && (task == m_task.get()) && task->hasFailed() && !task->isAborted())
  {
    auto message = tr("Couldn't color segmentations by %1").arg(task->key());
    auto details = task->error();
    DefaultDialogs::ErrorMessage(message, tr("Coloring by %1").arg(task->key()), details);
  }

  if(m_task.get() == task)
  {
    m_task = nullptr;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onToolToggled(bool checked)
{
  if (checked)
  {
    if (m_needUpdate)
    {
      update();
    }

    connect(getModel().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
            this,             SLOT(updateRange()));

    connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
            this,             SLOT(updateRange()));
  }
  else
  {
    disconnect(getModel().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
               this,             SLOT(updateRange()));

    disconnect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
               this,             SLOT(updateRange()));
  }
}
