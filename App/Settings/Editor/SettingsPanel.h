/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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

#ifndef MORPHOLOGICALFILTERSPREFERENCES_H
#define MORPHOLOGICALFILTERSPREFERENCES_H

#include "Toolbars/Editor/EditorToolBar.h"
#include <GUI/ISettingsPanel.h>
#include "ui_EditorToolBarSettingsPanel.h"


class EditorToolBar::SettingsPanel
: public ISettingsPanel
, public Ui::EditorToolBarSettingsPanel
{
  Q_OBJECT
public:
  SettingsPanel(Settings *settings);
  virtual ~SettingsPanel(){}

  virtual const QString shortDescription()
  { return "Edition Tools"; }

  virtual const QString longDescription()
  { return "Edition Tools Settings"; }

  virtual const QIcon icon()
  { return QIcon(":/espina/pencil.png"); }

  virtual void acceptChanges();

  virtual bool modified() const;

  virtual ISettingsPanel *clone();

public slots:

private:
  Settings *m_settings;
};

#endif // SEEDGROWSEGMENTATIONPREFERENCES_H
