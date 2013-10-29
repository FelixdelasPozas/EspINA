/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#ifndef INFORMATION_SELECTOR_H
#define INFORMATION_SELECTOR_H

#include "EspinaGUI_Export.h"

#include <QDialog>

#include <QStringListModel>
#include <Core/Model/Segmentation.h>

class QTreeWidgetItem;
namespace EspINA
{

  class EspinaFactory;

  class EspinaGUI_EXPORT InformationSelector
  : public QDialog
  {
    Q_OBJECT

    class GUI;
  public:
    typedef QMap<QString, Segmentation::InfoTagList> TaxonomyInformation;

  public:
    explicit InformationSelector(TaxonomyInformation &tags,
                                 EspinaFactory   *factory,
                                 QWidget         *parent = 0,
                                 Qt::WindowFlags f = 0);
    virtual ~InformationSelector();

  protected slots:
    void accept();
    void updateCheckState(QTreeWidgetItem *item, int column, bool updateParent = true);

  private:
    GUI *m_gui;

    EspinaFactory *m_factory;

    TaxonomyInformation &m_tags;
  };
}

#endif // INFORMATION_SELECTOR_H