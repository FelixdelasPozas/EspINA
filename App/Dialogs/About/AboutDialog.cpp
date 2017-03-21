/*
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

        ESPINA is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ESPINA is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// ESPINA
#include "AboutDialog.h"
#include "EspinaConfig.h"
#include <GUI/Dialogs/DefaultDialogs.h>
#include <QDebug>

using namespace ESPINA;
using namespace ESPINA::GUI;

//-----------------------------------------------------------------------------
AboutDialog::AboutDialog()
: QDialog(DefaultDialogs::defaultParentWidget())
{
  setupUi(this);

  setWindowTitle(tr("About ESPINA"));
  version->setText(QString("Version: %1").arg(ESPINA_VERSION));

  // Adjust label pixmaps. If the ui file is modified those values need to be modified to be in sync. Don'tlike it but it does the job scaling
  // the bitmap and SVG graphics without pixelating.
  constexpr auto fixedWidth = 158;
  constexpr auto fixedHeight = 104;

  logoUPM->setPixmap(QIcon(":/espina/upm.gif").pixmap(fixedHeight).scaled(fixedWidth, fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  logoCeSViMa->setPixmap(QIcon(":/espina/cesvima.svg").pixmap(fixedWidth).scaled(fixedWidth, fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  logoCBBP->setPixmap(QIcon(":/espina/cajalbbp.svg").pixmap(fixedWidth).scaled(fixedWidth,fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
}


