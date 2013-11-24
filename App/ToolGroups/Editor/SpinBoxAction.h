/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#ifndef ESPINA_EDITION_TOOLSR_ADIUS_H_
#define ESPINA_EDITION_TOOLSR_ADIUS_H_

#include <QWidgetAction>
#include <QLabel>
#include <QSpinBox>

#include <QDebug>

namespace EspINA
{
  class SpinBoxAction
  : public QWidgetAction
  {
    Q_OBJECT
    public:
      /** \brief Class constructor.
       *
       */
      explicit SpinBoxAction(QObject *parent = nullptr);

      /** \brief Class destructor.
       *
       */
      virtual ~SpinBoxAction();

      /** \brief Superclass method to create widget.
       *
       */
      virtual QWidget* createWidget(QWidget* parent);

      /** \brief Returns widget's radius value.
       *
       */
      int radius() const
      { return m_radius; }

      /** \brief Set minimum value for widget's QSpinBox.
       *
       */
      void setSpinBoxMinimum(int value);

      /** \brief Set maximum value for widget's QSpinBox.
       *
       */
      void setSpinBoxMaximum(int value);

      /** \brief Set value for the widget's QLabel.
       *
       */
      void setLabelText(const QString &label);

      /** \brief Set widget enabled/disabled.
       *
       */
      void setEnabled(bool value)
      {
        m_enabled = value;

        if (nullptr != m_radiusLabel)
        {
          m_radiusSpinBox->setEnabled(value);
          m_radiusLabel->setEnabled(value);
        }
      }

      /** \brief Returns the enabled/disabled state of the widgets.
       *
       */
      bool isEnabled()
      { return m_enabled; }

    public slots:
      /** \brief Sets widget's radius value.
       *
       */
      void setRadius(int value);

      /** \brief It's necessary to connect to the destroy signal of allocated
       *         widgets so we can nullify the pointers when they are destroyed.
       */
      void destroySignalEmmited()
      {
        m_radiusLabel = nullptr;
        m_radiusSpinBox = nullptr;
      }

    signals:
      /** \brief Signal to propagate changes int the widget's values.
       *
       */
      void radiusChanged(int);

    private:
      QLabel   *m_radiusLabel;
      QSpinBox *m_radiusSpinBox;

      int       m_radius;
      QString   m_label;
      bool      m_enabled;
      int       m_maximumValue;
      int       m_minimumValue;
  };

} // namespace EspINA

#endif // ESPINA_EDITION_TOOLSR_ADIUS_H_
