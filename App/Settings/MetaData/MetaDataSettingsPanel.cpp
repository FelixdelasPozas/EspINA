/*

    Copyright (C) 2014 Félix de las Pozas Álvarez <fpozas@cesvima.upm.es>

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
#include "MetaDataSettingsPanel.h"

// Qt
#include <QHBoxLayout>
#include <QLabel>

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
    auto label = new QLabel();
    label->setText(tr("MetaData Storage hasn't been configured for this build of ESPINA."));
    if(layout() == nullptr)
      setLayout(new QVBoxLayout());

    layout()->setAlignment(Qt::AlignCenter);
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
  SettingsPanelPtr MetaDataSettingsPanel::clone()
  {
    return new MetaDataSettingsPanel();
  }

} // namespace ESPINA
