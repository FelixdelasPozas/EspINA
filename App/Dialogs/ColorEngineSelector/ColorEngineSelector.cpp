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
#include <Dialogs/ColorEngineSelector/ColorEngineSelector.h>

// Qt
#include <QCheckBox>
#include <QVBoxLayout>

using namespace ESPINA;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::ColorEngines;

//--------------------------------------------------------------------
ColorEngineSelector::ColorEngineSelector(GUI::ColorEngines::MultiColorEngineSPtr engine, QWidget* parent, Qt::WindowFlags flags)
: QDialog      {parent, flags}
, m_colorEngine{engine}
{
  setupUi(this);

  connectSignals();

  m_useDefault->setChecked(true);
  m_useCustom->setChecked(false);

  auto layout = new QVBoxLayout(this);
  m_colorEnginesBox->setLayout(layout);

  for(auto registeredEngine: engine->availableEngines())
  {
    auto checkbox = new QCheckBox{registeredEngine->tooltip(), this};

    if(registeredEngine->isActive())
    {
      checkbox->setChecked(true);
    }

    layout->addWidget(checkbox);
  }

  m_colorEnginesBox->setEnabled(false);

  this->adjustSize();
}

//--------------------------------------------------------------------
const GUI::ColorEngines::ColorEngineSPtr ColorEngineSelector::colorEngine() const
{
  MultiColorEngineSPtr engine = std::make_shared<MultiColorEngine>();

  if(m_useCustom->isChecked())
  {
    auto layout  = m_colorEnginesBox->layout();
    auto engines = m_colorEngine->availableEngines();

    Q_ASSERT(layout->count() == m_colorEngine->availableEngines().size());

    for(int i = 0; i < layout->count(); ++i)
    {
      auto widget = qobject_cast<QCheckBox *>(layout->itemAt(i)->widget());
      if(widget && widget->isChecked())
      {
        auto selectedEngine = engines.at(i)->clone();
        engine->add(selectedEngine);
        selectedEngine->setActive(true);
      }
    }
  }

  if(engine->availableEngines().isEmpty())
  {
    engine = nullptr;
  }

  return engine;
}

//--------------------------------------------------------------------
void ColorEngineSelector::onRadioChangedState()
{
  m_colorEnginesBox->setEnabled(m_useCustom->isChecked());
}

//--------------------------------------------------------------------
void ColorEngineSelector::accept()
{
  unsigned int count = 0;
  bool isSameAsDefault = true;

  auto layout  = m_colorEnginesBox->layout();

  for(int i = 0; i < layout->count(); ++i)
  {
    auto widget = qobject_cast<QCheckBox *>(layout->itemAt(i)->widget());
    if(widget)
    {
      auto checked = widget->isChecked();
      if(checked) ++count;
      if(m_colorEngine->availableEngines().at(i)->isActive() != checked) isSameAsDefault = false;
    }
  }

  if(count == 0)
  {
    auto message = tr("Your didn't select any coloring engine. The selected segmentations will be colored by "
                      "the color engine determined by the visualization configuration.");
    DefaultDialogs::InformationMessage(message);
  }
  else
  {
    if(m_useCustom->isChecked() && isSameAsDefault)
    {
      auto message = tr("The selected color engines are the same as the current ones determined by the visualization configuration. "
                        "The segmentations will be colored by the default color engine.");
      DefaultDialogs::InformationMessage(message);
    }
  }

  QDialog::accept();
}

//--------------------------------------------------------------------
void ColorEngineSelector::connectSignals()
{
  connect(m_useDefault, SIGNAL(clicked(bool)),
          this,         SLOT(onRadioChangedState()));

  connect(m_useCustom, SIGNAL(clicked(bool)),
          this,        SLOT(onRadioChangedState()));
}
