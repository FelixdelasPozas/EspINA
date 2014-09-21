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


#ifndef ESPINA_GENERAL_SETTINGS_DIALOG_H
#define ESPINA_GENERAL_SETTINGS_DIALOG_H

// Qt
#include <QDialog>
#include "ui_GeneralSettingsDialog.h"

// ESPINA
#include <Core/EspinaTypes.h>
#include <Support/Settings/SettingsPanel.h>

namespace ESPINA
{
  class GeneralSettingsDialog
  : public QDialog
  , Ui::GeneralSettingsDialog
  {
    Q_OBJECT
  public:
    /** brief GeneralSettingsDialog class constructor.
     * \param[in] parent, parent widget raw pointer.
     * \param[in] flags, dialog flags.
     *
     */
    explicit GeneralSettingsDialog(QWidget *parent = nullptr,
                                   Qt::WindowFlags flags  = 0);

    /** brief Overrides QDialog::accept()
     *
     */
    virtual void accept() override;

    /** brief Overrides QDialog::reject()
     *
     */
    virtual void reject() override;

    /** brief Adds a panel to the dialog.
     * \param[in] panel, settings panel smart pointer.
     *
     */
    void registerPanel(SettingsPanelSPtr panel);

  private:
    /** brief Helper method to return a smart pointer of the
     *        panel with the passed short description.
     * \param[in] shortDesc, short description of the panel.
     *
     */
    SettingsPanelSPtr panel(const QString &shortDesc);

  public slots:
		/** brief Changes the panel in view.
		 * \param[in] panel, panel index in the m_panels list.
		 */
    void changePreferencePanel(int panel);

  private:
    SettingsPanelPtr   m_activePanel;
    SettingsPanelSList m_panels;
  };

} // namespace ESPINA

#endif // ESPINA_GENERAL_SETTINGS_DIALOG_H
