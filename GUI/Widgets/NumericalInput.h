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

        void setLabelVisibility(bool value);

        void setSliderVisibility(bool value);

        void setSpinBoxVisibility(bool value);

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
