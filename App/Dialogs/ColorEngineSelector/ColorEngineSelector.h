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

#ifndef APP_DIALOGS_COLORENGINESELECTOR_COLORENGINESELECTOR_H_
#define APP_DIALOGS_COLORENGINESELECTOR_COLORENGINESELECTOR_H_

// ESPINA
#include "ui_ColorEngineSelector.h"
#include <GUI/ColorEngines/MultiColorEngine.h>
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QDialog>

namespace ESPINA
{
  /** \class ColorEngineSelector
   * \brief Presents a dialog with the current registered color engines and lets the user select a combination.
   *
   */
  class ColorEngineSelector
  : public QDialog
  , private Ui::ColorEngineSelector
  {
      Q_OBJECT
    public:
      /** \brief ColorEngineSelector class constructor.
       * \param[in] engine Application multi-color engine containing all the registered engines.
       * \param[in] parent Raw pointer of the widget parent of this one.
       * \param[in] flags Window flags.
       *
       */
      ColorEngineSelector(GUI::ColorEngines::MultiColorEngineSPtr engine, QWidget *parent = GUI::DefaultDialogs::defaultParentWidget(), Qt::WindowFlags flags = Qt::WindowFlags());

      /** \brief ColorEngineSelector class virtual destructor.
       *
       */
      virtual ~ColorEngineSelector()
      {};

      /** \brief Returns the multi-color engine selected by the user or nullptr if the user selected to use the application one.
       *
       */
      const GUI::ColorEngines::ColorEngineSPtr colorEngine() const;

    public slots:
      virtual void accept() override;

    private slots:
      /** brief Disables/Enables the color engines widgets based on the selected radio button.
       *
       */
      void onRadioChangedState();

    private:
      /** \brief Helper method to connect Widget signals to slots in this dialog.
       *
       */
      void connectSignals();

      GUI::ColorEngines::MultiColorEngineSPtr m_colorEngine; /** application multi-color engine. */
  };

} // namespace ESPINA

#endif // APP_DIALOGS_COLORENGINESELECTOR_COLORENGINESELECTOR_H_
