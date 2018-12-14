/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <App/ToolGroups/Visualize/ColorEngines/ConnectionsColorEngineSwitch.h>
#include <App/Dialogs/ConnectionCount/ConnectionCriteriaDialog.h>
#include <Core/Utils/ListUtils.hxx>
#include <GUI/View/ViewState.h>
#include <GUI/ColorEngines/ConnectionsColorEngine.h>
#include <GUI/Widgets/ColorBar.h>
#include <GUI/Widgets/ToolButton.h>
#include <GUI/Widgets/Styles.h>

// Qt
#include <QIcon>
#include <QLabel>
#include <QThread>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::ColorEngines;

const QString CRITERIA_SETTINGS_KEY          = QObject::tr("Connection criteria");
const QString VALID_COLOR_SETTINGS_KEY       = QObject::tr("Valid color hue");
const QString INVALID_COLOR_SETTINGS_KEY     = QObject::tr("Invalid color hue");
const QString INCOMPLETE_COLOR_SETTINGS_KEY  = QObject::tr("Incomplete color hue");
const QString UNCONNECTED_COLOR_SETTINGS_KEY = QObject::tr("Unconnected color hue");


//--------------------------------------------------------------------
ConnectionsColorEngineSwitch::ConnectionsColorEngineSwitch(Support::Context& context)
: ColorEngineSwitch{std::make_shared<ConnectionsColorEngine>(), QIcon{":/espina/connectionGradient.svg"}, context}
, m_validHue       {QColor{Qt::green}.hue()}
, m_invalidHue     {QColor{Qt::red}.hue()}
, m_incompleteHue  {QColor{Qt::blue}.hue()}
, m_unconnectedHue {QColor{Qt::yellow}.hue()}
{
  createWidgets();

}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);

  m_criteria       = settings->value(CRITERIA_SETTINGS_KEY,          QStringList()).toStringList();
  m_validHue       = settings->value(VALID_COLOR_SETTINGS_KEY,       QColor{Qt::green}.hue()).toInt();
  m_invalidHue     = settings->value(INVALID_COLOR_SETTINGS_KEY,     QColor{Qt::red}.hue()).toInt();
  m_incompleteHue  = settings->value(INCOMPLETE_COLOR_SETTINGS_KEY,  QColor{Qt::blue}.hue()).toInt();
  m_unconnectedHue = settings->value(UNCONNECTED_COLOR_SETTINGS_KEY, QColor{Qt::yellow}.hue()).toInt();

  m_warning->setVisible(m_criteria.isEmpty());
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);

  settings->setValue(CRITERIA_SETTINGS_KEY,          m_criteria);
  settings->setValue(VALID_COLOR_SETTINGS_KEY,       m_validHue);
  settings->setValue(INVALID_COLOR_SETTINGS_KEY,     m_invalidHue);
  settings->setValue(INCOMPLETE_COLOR_SETTINGS_KEY,  m_incompleteHue);
  settings->setValue(UNCONNECTED_COLOR_SETTINGS_KEY, m_unconnectedHue);
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::createWidgets()
{
  m_criteriaButton = Styles::createToolButton(QIcon(":/espina/weight-balance.svg"), tr("Connection criteria..."));
  connect(m_criteriaButton, SIGNAL(clicked(bool)), this, SLOT(onCriteriaButtonPressed(bool)));
  m_criteriaButton->setCheckable(false);

  addSettingsWidget(m_criteriaButton);

  m_warning = new QLabel{tr("<font color=\"Red\"><b>No criteria</b></font>")};

  addSettingsWidget(m_warning);
}

//--------------------------------------------------------------------
void ConnectionsColorEngineSwitch::onCriteriaButtonPressed(bool value)
{
  auto engine = std::dynamic_pointer_cast<ConnectionsColorEngine>(colorEngine());

  ConnectionCriteriaDialog dialog{getModel(), m_criteria};
  dialog.setValidColor(m_validHue);
  dialog.setInvalidColor(m_invalidHue);
  dialog.setIncompleteColor(m_incompleteHue);
  dialog.setUnconnectedColor(m_unconnectedHue);

  if (dialog.exec() == QDialog::Accepted)
  {
    m_criteria = dialog.criteria();
    m_validHue = dialog.validColor();
    m_invalidHue = dialog.invalidColor();
    m_incompleteHue = dialog.incompleteColor();
    m_unconnectedHue = dialog.unconnectedColor();

    engine->setCriteriaInformation(m_criteria, m_validHue, m_invalidHue, m_incompleteHue, m_unconnectedHue);

    m_warning->setVisible(m_criteria.isEmpty());
  }
}
