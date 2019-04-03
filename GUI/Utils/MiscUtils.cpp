/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

// ESPINA
#include <GUI/Utils/MiscUtils.h>

// QT
#include <QPixmap>
#include <QPainter>
#include <QString>

using namespace ESPINA;
using namespace ESPINA::GUI;

//------------------------------------------------------------------------
const QPixmap Utils::appendImage(const QPixmap& original, const QString& image, bool slim)
{
  QPixmap pixmap(image);

  pixmap = pixmap.scaled(slim?8:16, 16, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

  const unsigned char ICON_SPACING = 5;

  QPixmap tmpPixmap(original.width() + ICON_SPACING + pixmap.width(),16);
  tmpPixmap.fill(Qt::transparent);

  QPainter painter(&tmpPixmap);
  painter.drawPixmap(0,0, original);
  painter.drawPixmap(original.width() + ICON_SPACING,0, pixmap);

  return tmpPixmap;
}
