/*

 Copyright (C) 2015 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_WIDGETS_PIXELVALUESELECTOR_H_
#define GUI_WIDGETS_PIXELVALUESELECTOR_H_

// ESPINA
#include <GUI/EspinaGUI_Export.h>

// Qt
#include <QWidget>

class QMouseEvent;
class QPaintEvent;

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      /** \class PixelValueSelector
       * \brief Implements a QWidget to select a grayscale value.
       *
       */
      class EspinaGUI_EXPORT PixelValueSelector
      : public QWidget
      {
          Q_OBJECT
        public:
          /** \brief PixelValueSelector class constructor.
           * \param[in] parent raw pointer of the widget parent of this one.
           *
           */
          PixelValueSelector(QWidget* parent = nullptr);

          /** \brief PixelValueSelector class virtual destructor.
           *
           */
          virtual ~PixelValueSelector();

          /** \brief Returns the actual value.
           *
           */
          int value() const;

        public slots:
          /** \brief Sets the value to the given paramenter and updates the UI.
           * \param[in] value new value [0-255]
           *
           */
          void setValue(int value);

        signals:
          void newValue(int value);

        protected:
          void paintEvent(QPaintEvent *unused) override;

          void mouseMoveEvent(QMouseEvent *e) override;

          void mousePressEvent(QMouseEvent *e) override;
  
        private:
          int      m_value;
          QPixmap *m_pixmap;
      };
    } // namespace Widgets
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_WIDGETS_PIXELVALUESELECTOR_H_
