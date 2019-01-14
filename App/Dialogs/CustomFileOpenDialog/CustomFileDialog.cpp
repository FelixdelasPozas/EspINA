/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <Dialogs/CustomFileOpenDialog/CustomFileDialog.h>
#include <Dialogs/CustomFileOpenDialog/OptionsPanel.h>
#include <Core/Analysis/Filters/VolumetricStreamReader.h>

// Qt
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QtCore>
#include <QPushButton>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::IO;

//--------------------------------------------------------------------
CustomFileDialog::CustomFileDialog(QWidget* parent, Qt::WindowFlags flags)
: QFileDialog{parent, flags}
{
  modifyUI();
}

//--------------------------------------------------------------------
CustomFileDialog::CustomFileDialog(QWidget* parent, const QString& caption, const QString& directory, const QString& filter)
: QFileDialog{parent, caption, directory, filter}
{
  modifyUI();
}

//--------------------------------------------------------------------
void CustomFileDialog::onOptionsToggled()
{
  auto visible    = m_options->isVisible();
  auto widgetSize = m_options->sizeHint().width();
  auto spacing    = layout()->spacing();

  m_options->setVisible(!visible);

  if(!visible)
  {
    resize(m_size.width()+widgetSize+spacing, m_size.height());
  }
  else
  {
    resize(m_size);
  }

  update();
}

//--------------------------------------------------------------------
void CustomFileDialog::resizeEvent(QResizeEvent *event)
{
  QFileDialog::resizeEvent(event);

  m_size = size();
  auto widgetSize = m_options->sizeHint().width();
  auto spacing    = layout()->spacing();
  if(m_options->isVisible())
  {
    m_size = QSize(m_size.width() - widgetSize - spacing, m_size.height());
  }
}

//--------------------------------------------------------------------
void CustomFileDialog::showEvent(QShowEvent* event)
{
  QFileDialog::showEvent(event);

  m_size = size();
}

//--------------------------------------------------------------------
const IO::LoadOptions CustomFileDialog::options() const
{
  LoadOptions options;

  options.insert(VolumetricStreamReader::STREAMING_OPTION, QVariant::fromValue(m_options->streamingValue()));
  options.insert(tr("Load Tool Settings"), QVariant::fromValue(m_options->toolSettingsValue()));
  options.insert(tr("Check analysis"), QVariant::fromValue(m_options->checkAnalysisValue()));

  return options;
}

//--------------------------------------------------------------------
void CustomFileDialog::modifyUI()
{
  auto mainLayout = qobject_cast<QGridLayout *>(layout());

  auto button = new QPushButton(tr("Options >>"), this);
  button->setCheckable(true);
  button->setChecked(false);
  connect(button, SIGNAL(pressed()), this, SLOT(onOptionsToggled()));
  mainLayout->addWidget(button, mainLayout->rowCount(), mainLayout->columnCount()-1, 1, 1);

  m_options = new OptionsPanel(this);
  mainLayout->addWidget(m_options, 0, mainLayout->columnCount(), mainLayout->rowCount(), 1);
  m_options->setVisible(false);
}
