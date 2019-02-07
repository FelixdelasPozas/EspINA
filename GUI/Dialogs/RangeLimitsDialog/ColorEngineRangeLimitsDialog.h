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

#ifndef GUI_DIALOGS_COLORENGINERANGELIMITSDIALOG_H_
#define GUI_DIALOGS_COLORENGINERANGELIMITSDIALOG_H_

#include <GUI/EspinaGUI_Export.h>

// ESPINA
#include "ui_ColorEngineRangeLimitsDialog.h"
#include <GUI/Dialogs/DefaultDialogs.h>

// Qt
#include <QDialog>

namespace ESPINA
{
  namespace GUI
  {
    /** \class ColorEngineRangeLimitsDialog
     * \brief Dialog to specify the range limits.
     *
     */
    class EspinaGUI_EXPORT ColorEngineRangeLimitsDialog
    : public QDialog
    , private Ui::ColorEngineRangeLimitsDialog
    {
        Q_OBJECT
      public:
        /** \brief ColorEngineRangeMaximumDialog class constructor.
         * \param[in] min Initial minimum value.
         * \param[in] max Initial maximum value.
         * \param[in] property Text of the property.
         * \param[in] parent Raw pointer of the widget parent of this one.
         * \param[in] flags Window flags.
         *
         */
        explicit ColorEngineRangeLimitsDialog(const double min, const double max, const QString &property, QWidget *parent = DefaultDialogs::defaultParentWidget(), Qt::WindowFlags flags = Qt::WindowFlags());

        /** \brief ColorEngineRangeMaximumDialog class virtual destructor.
         *
         */
        virtual ~ColorEngineRangeLimitsDialog()
        {};

        /** \brief Returns the range minimum
         *
         */
        const double min() const
        { return m_min->value(); }

        /** \brief Returns the range maximum
         *
         */
        const double max() const
        { return m_max->value(); }

      private slots:
        /** \brief Adjusts value changed according to current maximum.
         * \param[in] value Minimum spinbox value.
         *
         */
        void onMinValueChanged(double value);

        /** \brief Adjusts value changed according to current minimum.
         * \param[in] value Maximum spinbox value.
         *
         */
        void onMaxValueChanged(double value);

      private:
        /** \brief Helper method to connect the widget's signals.
         *
         */
        void connectSignals();
    };
  
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_DIALOGS_COLORENGINERANGELIMITSDIALOG_H_
