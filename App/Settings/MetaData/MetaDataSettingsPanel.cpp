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

// ESPINA
#include <Support/Metadona/StorageFactory.h>
#include "MetaDataSettingsPanel.h"

// Qt
#include <QHBoxLayout>
#include <QLabel>
#include <QProcess>
#include <QFont>

#if USE_METADONA
  #include <Producer.h>
  #include <IRODS_Storage.h>
  #include <Utils.h>
  #include <Support/Metadona/Coordinator.h>
  #include <Support/Metadona/MetadataViewer.h>
  #include <Support/Metadona/StorageFactory.h>
#endif

namespace ESPINA
{
  //------------------------------------------------------------------------
  MetaDataSettingsPanel::MetaDataSettingsPanel()
  {
    QString text;

    auto supportedStorages = StorageFactory::supportedStorages();

    if(supportedStorages.isEmpty())
    {
      text = tr("No metadata storage provider has been detected.");
    }
    else
    {
      if(supportedStorages.contains(StorageFactory::Type::IRODS))
      {
        text = tr("<u><b>IRODS Enviroment information</b></u><br>");

        QProcess process{this};
        process.start("ienv");

        if(!process.waitForFinished(1500))
        {
          // Enough time, assume failure
          process.kill();
          text = tr("IRODS metadata storage hasn't been correctly configured.");
        }
        else
        {
          text += process.readAll();
        }
      }
    }

    auto label = new QLabel(this);
    label->setText(text);
    label->setTextFormat(Qt::RichText);

    if(layout() == nullptr)
    {
      setLayout(new QVBoxLayout(this));
    }

    layout()->setAlignment(Qt::AlignTop|Qt::AlignLeft);
    layout()->addWidget(label);
  }

  //------------------------------------------------------------------------
  void MetaDataSettingsPanel::acceptChanges()
  {
  }

  //------------------------------------------------------------------------
  void MetaDataSettingsPanel::rejectChanges()
  {
  }

  //------------------------------------------------------------------------
  bool MetaDataSettingsPanel::modified() const
  {
    return false;
  }

  //------------------------------------------------------------------------
  ESPINA::Support::Settings::SettingsPanelPtr MetaDataSettingsPanel::clone()
  {
    return new MetaDataSettingsPanel();
  }

} // namespace ESPINA
