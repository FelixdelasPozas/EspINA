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

#ifndef APP_DIALOGS_CUSTOMFILEOPENDIALOG_OPTIONSPANEL_H_
#define APP_DIALOGS_CUSTOMFILEOPENDIALOG_OPTIONSPANEL_H_

// ESPINA
#include <ui_OptionsPanel.h>

// Qt
#include <QWidget>

namespace ESPINA
{
  /** \class OptionsPanel
   * \brief Options panel widget for custom file open dialog.
   *
   */
  class OptionsPanel
  : public QWidget
  , private Ui_OptionsPanel
  {
      Q_OBJECT
    public:
      /** \brief OptionsPanel class constructor.
       * \param[in] parent Raw pointer of the QWidget parent of this one.
       * \param[in] flags Widget flags.
       */
      explicit OptionsPanel(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

      /** \brief OptionsPanel class virtual destructor.
       *
       */
      virtual ~OptionsPanel()
      {};

      /** \brief Returns true if streaming is enabled and false otherwise.
       *
       */
      bool streamingValue() const;

      /** \brief Returns true if the 'load tool settings' checkbox is checked and false otherwise.
       *
       */
      bool toolSettingsValue() const;

      /** \brief Returns true if the 'check analysis' checkbox is checked and false otherwise.
       *
       */
      bool checkAnalysisValue() const;

    private slots:
      /** \brief Updates the UI when the state of the 'use default streaming settings' checkbox changes state.
       *
       */
      void onSettingsUseStateChanged(int state);
  };

} // namespace ESPINA

#endif // APP_DIALOGS_CUSTOMFILEOPENDIALOG_OPTIONSPANEL_H_
