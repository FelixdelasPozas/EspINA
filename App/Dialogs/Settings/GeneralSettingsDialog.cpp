/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "GeneralSettingsDialog.h"
#include <Support/Settings/SettingsPanel.h>
#include <Core/Utils/EspinaException.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QDir>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QTime>

using namespace ESPINA;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::Support::Settings;

//------------------------------------------------------------------------
GeneralSettingsDialog::GeneralSettingsDialog(QWidget *parent, Qt::WindowFlags flags)
: QDialog      {parent, flags}
, m_activePanel{nullptr}
{
  setupUi(this);

  setWindowTitle(tr("ESPINA Settings"));

  connect(components,SIGNAL(currentRowChanged(int)),
          this, SLOT(changePreferencePanel(int)));
}

//------------------------------------------------------------------------
void GeneralSettingsDialog::accept()
{
  try
  {
    m_activePanel->acceptChanges();
  }
  catch(const EspinaException &e)
  {
    DefaultDialogs::InformationMessage(QString("Couldn't accept the changes in '%1' panel.\n\nError: %2").arg(m_activePanel->shortDescription()).arg(QString(e.what())));
    return;
  }

  QDialog::accept();
}

//------------------------------------------------------------------------
void GeneralSettingsDialog::reject()
{
  m_activePanel->rejectChanges();
  QDialog::reject();
}

//------------------------------------------------------------------------
void GeneralSettingsDialog::registerPanel(SettingsPanelSPtr panel)
{
  auto item = new QListWidgetItem();

  item->setData(Qt::DisplayRole,   panel->shortDescription());
  item->setData(Qt::DecorationRole,panel->icon());

  components->addItem(item);

  m_panels << panel;
}

//------------------------------------------------------------------------
SettingsPanelSPtr GeneralSettingsDialog::panel(const QString& shortDesc)
{
  for(auto panel : m_panels)
  {
    if (panel->shortDescription() == shortDesc)
    {
      return panel;
    }
  }

  Q_ASSERT(false);

  return SettingsPanelSPtr();
}


//------------------------------------------------------------------------
void GeneralSettingsDialog::changePreferencePanel(int panel)
{
  if (m_activePanel && m_activePanel->modified())
  {
    QMessageBox msg;
    msg.setWindowTitle(tr("ESPINA"));
    msg.setText(tr("Settings panel \"%1\" have been modified.\n"
                "Do you want to save the changes?")
                .arg(m_activePanel->shortDescription()));
    msg.setStandardButtons(QMessageBox::Yes|QMessageBox::No);
    if (msg.exec() == QMessageBox::Yes)
    {
      try
      {
        m_activePanel->acceptChanges();
      }
      catch(const EspinaException &e)
      {
        DefaultDialogs::InformationMessage(QString("Couldn't accept the changes in '%1' panel.\nError: %2").arg(m_activePanel->shortDescription()).arg(QString(e.what())));
        return;
      }
    }
    else
    {
      m_activePanel->rejectChanges();
    }
  }

  m_activePanel = m_panels[panel]->clone();
  longDescription->setText(m_activePanel->longDescription());
  icon->setPixmap(m_activePanel->icon().pixmap(icon->size()));
  scrollArea->setWidget(m_activePanel); // takes ownership of the widget and destroys it when another widget is set or the scroll is destroyed
}
