/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "DefaultIcons.h"

// Qt
#include <QStyle>
#include <QApplication>

using namespace ESPINA;
using namespace ESPINA::GUI;

//------------------------------------------------------------------------
QIcon DefaultIcons::Save()
{
  return qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
}

//------------------------------------------------------------------------
QIcon DefaultIcons::Load()
{

 return qApp->style()->standardIcon(QStyle::SP_DialogOpenButton);
}

//------------------------------------------------------------------------
QIcon DefaultIcons::File()
{
  return qApp->style()->standardIcon(QStyle::SP_FileIcon);
}

//------------------------------------------------------------------------
QIcon DefaultIcons::Settings()
{
  return QIcon(":/espina/settings.png");
}
