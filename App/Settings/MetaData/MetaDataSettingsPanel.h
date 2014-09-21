/*

    Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
      /** \brief MetaDataSettingsPanel class constructor.
       *
       */
      explicit MetaDataSettingsPanel();

      /** \brief MetaDataSettingsPanel class virtual destructor.
       *
       */
      virtual ~MetaDataSettingsPanel()
      {};

      /** \brief Overrides SettingsPanel::shortDescription().
       *
       */
      virtual const QString shortDescription() override
      { return tr("Metadata Storage"); }

      /** \brief Overrides SettingsPanel::longDescription().
       *
       */
      virtual const QString longDescription() override
      { return tr("Metadata Storage"); }

      /** \brief Overrides SettingsPanel::icon():
       *
       */
      virtual const QIcon icon() override
      { return QIcon(":/espina/database.png"); }

      /** \brief Overrides SettingsPanel::addPanel(SettingsPanel *).
       *
       */
      virtual void addPanel(SettingsPanel *panel) override
      {}

      /** \brief Overrides SettingsPanel::acceptChanges().
       *
       */
      virtual void acceptChanges() override;

      /** \brief Overrides SettingsPanel::rejectChanges().
       *
       */
      virtual void rejectChanges() override;

      /** \brief Overrides SettingsPanel::modified().
       *
       */
      virtual bool modified() const override;

      /** \brief Overrides SettingsPanel::clone().
       *
       */
      virtual SettingsPanelPtr clone() override;
  };

} // namespace ESPINA

#endif // METADATASETTINGSPANEL_H_
