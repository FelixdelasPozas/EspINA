/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef GUI_WIDGETS_LINEEDITWITHCLEAR_H_
#define GUI_WIDGETS_LINEEDITWITHCLEAR_H_

#include "GUI/EspinaGUI_Export.h"

// Qt
#include <QLineEdit>
#include <QToolButton>

namespace ESPINA
{
  namespace GUI
  {
    namespace Widgets
    {
      /** \class LineEditWithClear
       * \brief Imlements a QLineEdit with a clear input button that appears when the user enters text.
       *
       */
      class EspinaGUI_EXPORT LineEditWithClear
      : public QLineEdit
      {
          Q_OBJECT
        public:
          /** \brief LineEditWithClear class constructor.
           * \param[in] parent QWidget parent of this one.
           *
           */
          explicit LineEditWithClear(QWidget *parent = nullptr);

          /** LineEditWithClear class virtual destructor.
           *
           */
          virtual ~LineEditWithClear()
          {}

          virtual void resizeEvent(QResizeEvent *event);

        private slots:
          /** \brief Shows/hides the clear button depending on the input text.
           * \param[in] text text currently in the edit field.
           *
           */
          void updateClearButton(const QString& text);

        private:
          QToolButton *m_button; /** clear button. */
      };
    
    } // namespace Widgets
  } // namespace GUI
} // namespace ESPINA

#endif // GUI_WIDGETS_LINEEDITWITHCLEAR_H_
