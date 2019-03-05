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

#ifndef ESPINA_SLIDER_NUMERICAL_INPUT_H_
#define ESPINA_SLIDER_NUMERICAL_INPUT_H_

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QWidget>

class QSpinBox;
class QSlider;
class QLabel;
namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      class EspinaGUI_EXPORT NumericalInput
      : public QWidget
      {
        Q_OBJECT
      public:
        /** \brief SliderAction class constructor.
         *
         */
        explicit NumericalInput(QWidget *parent = nullptr);

        /** \brief SliderAction class virtual destructor.
         *
         */
        virtual ~NumericalInput();

        /** \brief Returns widget's radius value.
         *
         */
        int value() const;

        /** \brief Set minimum value for widget's QSpinBox.
         * \param[in] value new value.
         *
         */
        void setMinimum(int value);

        /** \brief Set maximum value for widget's QSpinBox.
         * \param[in] value new value.
         *
         */
        void setMaximum(int value);

        /** \brief Sets the label of the slider
         * \param[in] label new label.
         *
         */
        void setLabelText(const QString &label);

        /** \brief Modifies the label visibility.
         * \param[in] value true to set visible and false otherwise.
         *
         */
        void setLabelVisibility(bool value);

        /** \brief Modifies the slider visibility.
         * \param[in] value true to set visible and false otherwise.
         *
         */
        void setSliderVisibility(bool value);

        /** \brief Modifies the spinbox visibility.
         * \param[in] value true to set visible and false otherwise.
         *
         */
        void setSpinBoxVisibility(bool value);

        /** \brief Enables/disables slider tracking (emit valueChanged(int) when the slider changes
         *  position or only when slider is released).
         *  \param[in] enabled true to enable and false otherwise.
         *
         */
        void setSliderTracking(bool enabled);

        /** \brief Sets the tooltip of the widgets.
         * \param[in] tooltip tooltip text.
         */
        void setWidgetsToolTip(const QString &tooltip);

        /** \brief Sets the spin box suffix.
         * \param[in] suffix Suffix text.
         *
         */
        void setSpinBoxSuffix(const QString &suffix);

      public slots:
        /** \brief change slider value
         * \param[in] value to be set
         *
         */
        void setValue(int value);

      signals:
        /** \brief Signal to propagate changes int the widget's values.
         *
         */
        void valueChanged(int);

      private:
        QLabel   *m_label;
        QSlider  *m_slider;
        QSpinBox *m_spinBox;
      };

    }
  }
} // namespace ESPINA

#endif // ESPINA_SLIDER_NUMERICAL_INPUT_H_
