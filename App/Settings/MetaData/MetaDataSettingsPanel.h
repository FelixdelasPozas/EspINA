/*

    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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

#ifndef METADATA_SETTINGS_PANEL_H_
#define METADATA_SETTINGS_PANEL_H_

// ESPINA
#include <Support/Settings/SettingsPanel.h>

namespace ESPINA
{
  
  class MetaDataSettingsPanel
  : public SettingsPanel
  {
    public:
      /* \brief MetaDataSettingsPanel class constructor.
       *
       */
      explicit MetaDataSettingsPanel();

      /* \brief MetaDataSettingsPanel class virtual destructor.
       *
       */
      virtual ~MetaDataSettingsPanel()
      {};

      /* \brief Implements SettingsPanel::shortDescription().
       *
       */
      virtual const QString shortDescription()
      { return tr("Metadata Storage"); }

      /* \brief Implements SettingsPanel::longDescription().
       *
       */
      virtual const QString longDescription()
      { return tr("Metadata Storage"); }

      /* \brief Implements SettingsPanel::icon():
       *
       */
      virtual const QIcon icon()
      { return QIcon(":/espina/database.png"); }

      /* \brief Implements SettingsPanel::addPanel(SettingsPanel *).
       *
       */
      virtual void addPanel(SettingsPanel *panel)
      {}

      /* \brief Implements SettingsPanel::acceptChanges().
       *
       */
      virtual void acceptChanges();

      /* \brief Implements SettingsPanel::rejectChanges().
       *
       */
      virtual void rejectChanges();

      /* \brief Implements SettingsPanel::modified().
       *
       */
      virtual bool modified() const;

      /* \brief Implements SettingsPanel::clone().
       *
       */
      virtual SettingsPanelPtr clone();
  };

} // namespace ESPINA

#endif // METADATASETTINGSPANEL_H_
