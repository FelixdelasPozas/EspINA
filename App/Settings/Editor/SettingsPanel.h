/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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
 */

#ifndef MORPHOLOGICALFILTERSPREFERENCES_H
#define MORPHOLOGICALFILTERSPREFERENCES_H

#include "Toolbars/Editor/EditorToolBar.h"
#include <GUI/ISettingsPanel.h>
#include "ui_EditorSettingsPanel.h"

namespace EspINA
{
  class EditorToolBarSettings;

  class EditorToolBar::SettingsPanel
  : public ISettingsPanel
  , public Ui::EditorSettingsPanel
  {
    Q_OBJECT
  public:
    SettingsPanel(EditorToolBarSettings *settings);
    virtual ~SettingsPanel(){}

    virtual const QString shortDescription()
    { return tr("Edition Tools"); }

    virtual const QString longDescription()
    { return tr("Edition Tools"); }

    virtual const QIcon icon()
    { return QIcon(":/espina/pencil.png"); }

    virtual void acceptChanges();
    virtual void rejectChanges();
    virtual bool modified() const;

    virtual ISettingsPanelPtr clone();

  public slots:

  private:
  EditorToolBarSettings *m_settings;
  };

} // namespace EspINA

#endif // SEEDGROWSEGMENTATIONPREFERENCES_H
