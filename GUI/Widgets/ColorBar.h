/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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
        ~ColorBar()
        {};

        /** \brief Generates and returns a QImage of the given range with the given width and height.
         * \param[in] range ColorRange object.
         * \param[in] width Resulting pixmap width.
         * \param[in] height Resulting pixmap height.
         *
         */
        static QImage rangeImage(const Utils::ColorRange *, int width, int height);

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
