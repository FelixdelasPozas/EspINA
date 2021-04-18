/*
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef ESPINA_GUI_WIDGETS_TOOLBUTTON_H
#define ESPINA_GUI_WIDGETS_TOOLBUTTON_H

#include "GUI/EspinaGUI_Export.h"

// C++
#include <memory>

// Qt
#include <QPushButton>

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      class EspinaGUI_EXPORT ToolButton
      : public QPushButton
      {
          Q_OBJECT

        public:
          /** \brief ToolButton class constructor.
           * \param[in] parent Pointer of the QWidget parent of this one.
           *
           */
          explicit ToolButton(QWidget* parent = 0);

          /** \brief ToolButton class virtual destructor.
           *
           */
          virtual ~ToolButton()
          {};

        public slots:
          /** \brief Updates the icon of the button.
           * \param[in] icon new QIcon object to put as icon.
           *
           */
          void changeIcon(const QIcon &icon);

          /** \brief Updates the button tooltip.
           * \param[in] tooltip tooltip text.
           *
           */
          void changeTooltip(const QString &tooltip);
      };

      using ToolButtonPtr  = ToolButton *;
      using ToolButtonSPtr = std::shared_ptr<ToolButton>;
    } // namespace Widgets
  } // namespace GUI
} // namespace ESPINA

#endif // ESPINA_GUI_WIDGETS_TOOLBUTTON_H
