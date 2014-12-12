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

#ifndef ESPINA_DOUBLE_SPINBOX_ACTION_H_
#define ESPINA_DOUBLE_SPINBOX_ACTION_H_

// Qt
#include <QWidgetAction>
#include <QLabel>
#include <QDoubleSpinBox>

namespace ESPINA
{
  
  class DoubleSpinBoxAction
  : public QWidgetAction
  {
    Q_OBJECT
    public:
      /** \brief DoubleSpinBoxAction class constructor.
       * \param[in] parent raw pointer of the QObject parent of this one.
       *
       */
      explicit DoubleSpinBoxAction(QObject *parent = nullptr);

      /** \brief DoubleSpinBoxAction class virtual destructor.
       *
       */
      virtual ~DoubleSpinBoxAction();

      virtual QWidget* createWidget(QWidget* parent) override;

      /** \brief Returns widget's radius value.
       *
       */
      double value() const
      { return m_value; }

      /** \brief Set minimum value for widget's QSpinBox.
       * \param[in] value new spinbox minimium value.
       *
       */
      void setSpinBoxMinimum(double value);

      /** \brief Returns the minimum value for the widget's QSpinBox.
       *
       */
      double getSpinBoxMinimumValue()
      { return m_minimumValue; }

      /** \brief Set maximum value for widget's QSpinBox.
       * \param[in] value new spinbox maximum value.
       *
       */
      void setSpinBoxMaximum(double value);

      /** \brief Returns the maximum value for the widget's QSpinBox.
       *
       */
      double getSpinBoxMaximumValue()
      { return m_maximumValue; }

      /** \brief Set value for the widget's QLabel.
       * \param[in] label new action label.
       *
       */
      void setLabelText(const QString &label);

      /** \brief Set suffix of the spin box.
       * \param[in] suffix new spinbox suffix.
       *
       */
      void setSuffix(const QString &suffix);

      /** \brief Sets the spinbox increment value.
       * \param[in] value new increment value.
       *
       */
      void setStepping(double value);

      /** \brief Returs the spinbox stepping value.
       *
       */
      double getSpinBoxStepping()
      { return m_step; }

      void setEnabled(bool value)
      {
        m_enabled = value;

        if (nullptr != m_label)
        {
          m_label->setEnabled(value);
          m_spinBox->setEnabled(value);
        }
      }

      bool isEnabled()
      { return m_enabled; }

    public slots:
      /** \brief Sets widget's radius value.
       * \param[in] value, new value.
       *
       */
      void setValue(double value);

      /** \brief It's necessary to connect to the destroy signal of allocated
       *         widgets so we can nullify the pointers when they are destroyed.
       */
      void destroySignalEmmited()
      { m_label = nullptr; m_spinBox = nullptr; }

    signals:
      /** \brief Signal to propagate changes into the widget's values.
       *
       */
      void valueChanged(double);

    private:
      QLabel         *m_label;
      QDoubleSpinBox *m_spinBox;

      double    m_value;
      QString   m_text;
      QString   m_suffix;
      bool      m_enabled;
      double    m_maximumValue;
      double    m_minimumValue;
      double    m_step;
  };

} // namespace ESPINA

#endif // ESPINA_DOUBLE_SPINBOX_ACTION_H_
