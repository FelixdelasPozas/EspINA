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

// Qt
#include <QDebug>
#include <QEvent>
#include <QMouseEvent>
#include <QLabel>
#include <QDesktopServices>

using namespace ESPINA;
using namespace ESPINA::GUI;

//-----------------------------------------------------------------------------
AboutDialog::AboutDialog()
: QDialog(DefaultDialogs::defaultParentWidget())
{
  setupUi(this);

  setWindowTitle(tr("About ESPINA"));
  version->setText(QString("Version: %1").arg(ESPINA_VERSION));

  // Adjust label pixmaps. If the ui file is modified those values need to be modified to be in sync. Don't like it but it does the job scaling
  // the bitmap and SVG graphics without pixelating them.
  constexpr auto fixedWidth = 158;
  constexpr auto fixedHeight = 104;

  logoUPM->setPixmap(QIcon(":/espina/upm.gif").pixmap(fixedHeight).scaled(fixedWidth, fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  logoUPM->setToolTip(trUtf8("Universidad Politécnica de Madrid"));
  logoUPM->setAlignment(Qt::AlignCenter);
  logoUPM->installEventFilter(this);

  logoCeSViMa->setPixmap(QIcon(":/espina/cesvima.svg").pixmap(fixedWidth).scaled(fixedWidth, fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  logoCeSViMa->setToolTip(trUtf8("Centro de Supercomputación y Visualización de Madrid"));
  logoCeSViMa->setAlignment(Qt::AlignCenter);
  logoCeSViMa->installEventFilter(this);

  logoCBBP->setPixmap(QIcon(":/espina/cajalbbp.svg").pixmap(fixedWidth).scaled(fixedWidth,fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  logoCBBP->setToolTip(trUtf8("Cajal Blue Brain Project"));
  logoCBBP->setAlignment(Qt::AlignCenter);
  logoCBBP->installEventFilter(this);

  logoURJC->setPixmap(QIcon(":/espina/Logo_URJC.svg").pixmap(fixedWidth).scaled(fixedWidth,fixedHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation));
  logoURJC->setToolTip(trUtf8("Universidad Rey Juan Carlos"));
  logoURJC->setAlignment(Qt::AlignCenter);
  logoURJC->installEventFilter(this);

  espinaLogo->installEventFilter(this);
  espinaText->installEventFilter(this);
  version->installEventFilter(this);
}

//-----------------------------------------------------------------------------
bool AboutDialog::eventFilter(QObject* object, QEvent* event)
{
  if(event->type() == QEvent::MouseButtonPress)
  {
    auto label = qobject_cast<QLabel *>(object);
    if(label)
    {
      auto me = static_cast<const QMouseEvent*>(event);

      if(me && me->button() == Qt::LeftButton)
      {
        if(label == logoUPM)
        {
          QDesktopServices::openUrl(QUrl("http://www.upm.es/"));
          return true;
        }

        if(label == logoCeSViMa)
        {
          QDesktopServices::openUrl(QUrl("http://www.cesvima.upm.es/"));
          return true;
        }

        if(label == logoCBBP)
        {
          QDesktopServices::openUrl(QUrl("http://cajalbbp.cesvima.upm.es/"));
          return true;
        }

        if(label == logoURJC)
        {
          QDesktopServices::openUrl(QUrl("https://www.urjc.es/"));
          return true;
        }

        if(label == version || label == espinaLogo || label == espinaText)
        {
          QDesktopServices::openUrl(QUrl("http://cajalbbp.cesvima.upm.es/espina/"));
          return true;
        }
      }
    }
  }

  return false;
}
