/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_GUI_WIDGETS_STYLES_H
#define ESPINA_GUI_WIDGETS_STYLES_H

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
      namespace Styles
      {
         void setNestedStyle(QWidget *widget);

         QAction *createToolAction( const QString &icon, const QString &tooltip, QObject *parent );

         QAction *createToolAction(const QIcon &icon, const QString &tooltip, QObject *parent);

         QPushButton *createToolButton(const QString &icon, const QString &tooltip, QWidget *parent = 0);

         QPushButton *createToolButton(const QIcon &icon, const QString &tooltip, QWidget *parent = 0);

         constexpr int buttonSize()
         { return 30; }

         constexpr int iconSize()
         { return 0.74*buttonSize(); }

         constexpr int MediumBarWidth()
         { return 80; }

         void setBarStyle(QWidget *bar);
      };
    }
  }
}

#endif // ESPINA_GUI_WIDGETS_STYLES_H
