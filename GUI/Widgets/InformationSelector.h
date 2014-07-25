/*
    
    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef ESPINA_INFORMATION_SELECTOR_H
#define ESPINA_INFORMATION_SELECTOR_H

#include "GUI/EspinaGUI_Export.h"

#include <Core/Analysis/Segmentation.h>
#include <GUI/ModelFactory.h>

#include <QDialog>

#include <QStringListModel>

class QTreeWidgetItem;
namespace ESPINA
{

  class EspinaFactory;

  class EspinaGUI_EXPORT InformationSelector
  : public QDialog
  {
    Q_OBJECT

    class GUI;

  public:
    using GroupedInfo = QMap<QString, QStringList>;

  public:
    explicit InformationSelector(const GroupedInfo &availableGroups,
                                 GroupedInfo       &selection,
                                 QWidget         *parent = 0,
                                 Qt::WindowFlags  flags = 0);
    virtual ~InformationSelector();

  protected slots:
    void accept();

    void updateCheckState(QTreeWidgetItem *item, int column, bool updateParent = true);

  private:
    GUI *m_gui;

    ModelFactorySPtr m_factory;

    GroupedInfo &m_selection;
  };
}

#endif // ESPINA_INFORMATION_SELECTOR_H
