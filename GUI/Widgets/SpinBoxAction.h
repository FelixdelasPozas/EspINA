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

#ifndef ESPINA_SPINBOX_ACTION_WIDGET_H_
#define ESPINA_SPINBOX_ACTION_WIDGET_H_

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QWidgetAction>
#include <QLabel>
#include <QSpinBox>

namespace ESPINA
{
  class EspinaGUI_EXPORT SpinBoxAction
  : public QWidgetAction
  {
    Q_OBJECT
    public:
      /** \brief SpinBoxAction class constructor.
       * \param[in] parent, raw pointer of the QObject parent of this one.
       *
       */
      explicit SpinBoxAction(QObject *parent = nullptr);

      /** \brief SpinBoxAction class virtual destructor.
       *
       */
      virtual ~SpinBoxAction();

      /** \brief Overrides QWidgetAction::createWidget().
       *
       */
      virtual QWidget* createWidget(QWidget* parent) override;

      /** \brief Returns widget's radius value.
       *
       */
      int value() const
      { return m_value; }

      /** \brief Set minimum value for widget's QSpinBox.
       * \param[in] value, new value.
       *
       */
      void setSpinBoxMinimum(int value);

      /** \brief Set maximum value for widget's QSpinBox.
       * \param[in] value, new value.
       *
       */
      void setSpinBoxMaximum(int value);

      /** \brief Set value for the widget's QLabel.
       * \param[in] label, new label.
       *
       */
      void setLabelText(const QString &label);

      /** \brief Set suffix of the spin box.
       * \param[in] suffix, new suffix.
       *
       */
      void setSuffix(const QString &suffix);

      /** brief Shadows QAction::setEnabled().
       *
       */
      void setEnabled(bool value)
      {
        m_enabled = value;

        if (nullptr != m_label)
        {
          m_label->setEnabled(value);
          m_spinBox->setEnabled(value);
        }
      }

      /** brief Shadows QAction::isEnabled().
       *
       */
      bool isEnabled()
      { return m_enabled; }

    public slots:
      /** \brief Sets widget's radius value.
       * \param[in] value, new value.
       *
       */
      void setValue(int value);

      /** \brief It's necessary to connect to the destroy signal of allocated
       *         widgets so we can nullify the pointers when they are destroyed.
       */
      void destroySignalEmmited()
      { m_label = nullptr; m_spinBox = nullptr; }

    signals:
      /** \brief Signal to propagate changes int the widget's values.
       *
       */
      void valueChanged(int);

    private:
      QLabel   *m_label;
      QSpinBox *m_spinBox;

      int       m_value;
      QString   m_text;
      QString   m_suffix;
      bool      m_enabled;
      int       m_maximumValue;
      int       m_minimumValue;
  };

} // namespace ESPINA

#endif // ESPINA_SPINBOX_ACTION_WIDGET_H_
