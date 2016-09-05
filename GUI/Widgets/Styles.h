/*
 *    Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.
 *
 *    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_GUI_WIDGETS_STYLES_H
#define ESPINA_GUI_WIDGETS_STYLES_H

#include <GUI/EspinaGUI_Export.h>

class QAction;
class QIcon;
class QObject;
class QPushButton;
class QString;
class QWidget;

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      class ToolButton;

      namespace Styles
      {
        /** \class DefaultCursor
         * \brief Set the default cursor during the scope of its instances
         *
         */
        class EspinaGUI_EXPORT DefaultCursor
        {
          public:
            /** \brief DefaultCursor class constructor.
             *
             */
            DefaultCursor();

            /** \brief DefaultCursor class destructor.
             *
             */
            ~DefaultCursor();
        };

        /** \class WaitingCursor
         * \brief Set the waiting cursor during the scope of its instances
         *
         */
        class EspinaGUI_EXPORT WaitingCursor
        {
          public:
            /** \brief WaitingCursor class constructor.
             *
             */
            WaitingCursor();

            /** \brief WaitingCursor class destructor.
             *
             */
            ~WaitingCursor();
        };

        const int CONTEXTUAL_BAR_HEIGHT = 50;

        /** \brief Configures the given widget with the visual properties of a nested widget.
         * \param[in] widget QWidget object pointer.
         *
         */
        void EspinaGUI_EXPORT setNestedStyle(QWidget *widget);

        /** \brief Creates and returns a QAction object configured with the given parameters.
         * \param[in] icon action icon (needs to be present in application resources file).
         * \param[in] tooltip action tooltip.
         * \param[in] parent pointer of the widget parent of this one.
         *
         */
        EspinaGUI_EXPORT QAction* createToolAction(const QString &icon, const QString &tooltip, QObject *parent = nullptr);

        /** \brief Creates and returns a QAction object configured with the given parameters.
         * \param[in] icon action icon.
         * \param[in] tooltip action tooltip.
         * \param[in] parent pointer of the widget parent of this one.
         *
         */
        EspinaGUI_EXPORT QAction* createToolAction(const QIcon &icon, const QString &tooltip, QObject *parent = nullptr);

        /** \brief Creates and returns a ToolButton object configured with the given parameters.
         * \param[in] icon action icon (needs to be present in application resources file).
         * \param[in] tooltip action tooltip.
         * \param[in] parent pointer of the widget parent of this one.
         *
         */
        EspinaGUI_EXPORT ToolButton* createToolButton(const QString &icon, const QString &tooltip, QWidget *parent = nullptr);

        /** \brief Creates and returns a ToolButton object configured with the given parameters.
         * \param[in] icon action icon.
         * \param[in] tooltip action tooltip.
         * \param[in] parent pointer of the widget parent of this one.
         *
         */
        EspinaGUI_EXPORT ToolButton* createToolButton(const QIcon &icon, const QString &tooltip, QWidget *parent = nullptr);

        /** \brief Returns the application default button size.
         *
         */
        constexpr int EspinaGUI_EXPORT buttonSize()
        { return 36; }

        /** \brief Returns the application default icon size.
         *
         */
        constexpr int EspinaGUI_EXPORT iconSize()
        { return 0.74*buttonSize(); }

        /** \brief Returns the application default bar width.
         *
         */
        constexpr int EspinaGUI_EXPORT MediumBarWidth()
        { return 80; }

        /** \brief Configures the given bar with the default properties of an application bar.
         * \param[in] bar QWidget object pointer.
         *
         */
        void EspinaGUI_EXPORT setBarStyle(QWidget *bar);
      };
    }
  }
}

#endif // ESPINA_GUI_WIDGETS_STYLES_H
