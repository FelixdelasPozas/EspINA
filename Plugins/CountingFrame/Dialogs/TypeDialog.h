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

#ifndef ESPINA_CF_TYPE_DIALOG_H
#define ESPINA_CF_TYPE_DIALOG_H

// EspINA
#include "Dialogs/TypeDialog.h"
#include "ui_TypeDialog.h"

#include <CountingFrames/CountingFrame.h>

// Qt
#include <QDialog>

namespace EspINA
{
  namespace CF
  {
    class TypeDialog
    : public QDialog
    , private Ui::TypeDialog
    {
      Q_OBJECT

    public:
      TypeDialog(QWidget *parent);
      virtual ~TypeDialog() {};

      void setType(CFType type);

      CFType type() const { return m_type; }

    public slots:
      void radioChanged(bool);

    private:
      CFType m_type;
    };

  }
} /* namespace EspINA */

#endif // ESPINA_CF_TYPE_DIALOG_H
