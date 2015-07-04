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

#ifndef ESPINA_GUI_WIDGETS_COLOR_BAR_H
#define ESPINA_GUI_WIDGETS_COLOR_BAR_H

#include "GUI/EspinaGUI_Export.h"

#include <GUI/Types.h>

// Qt includes
#include <QWidget>

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      class EspinaGUI_EXPORT ColorBar
      : public QWidget
      {
        Q_OBJECT
      public:
        /** \brief ColorBar class constructor.
         * \param[in] parent widget
         *
         */
        ColorBar(Utils::ColorRange *range, QWidget* parent = nullptr);

        /** \brief ColorBar class destructor.
         *
         */
        ~ColorBar();

      protected:
        void paintEvent(QPaintEvent *event) override;

      private slots:
        void updateTooltip();

      private:
        Utils::ColorRange *m_colorRange;
        QPixmap            m_colorMap;
      };
    }

  }
} // namespace ESPINA

#endif // ESPINA_GUI_WIDGETS_COLOR_BAR_H
