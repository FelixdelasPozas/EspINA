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


#ifndef TAXONOMYEXPLORER_H
#define TAXONOMYEXPLORER_H

//----------------------------------------------------------------------------
// File:    TaxonomyExplorer.h
// Purpose: Dock widget to manage taxonomies in the model
//----------------------------------------------------------------------------
#include <QDockWidget>

#include <Core/ColorEngines/TaxonomyColorEngine.h>
#include <Core/Model/EspinaModel.h>
#include <QSortFilterProxyModel>

namespace EspINA
{
  class ViewManager;

  class TaxonomyExplorer
  : public QDockWidget
  {
    Q_OBJECT

    class GUI;
  public:
    explicit TaxonomyExplorer(EspinaModel *model,
                              ViewManager *vm,
                              TaxonomyColorEnginePtr engine,
                              QWidget               *parent = 0);
    virtual ~TaxonomyExplorer();

  protected slots:
    // Create a new taxonomy at the same level of the selected index
    void addSameLevelTaxonomy();
    // Create a new taxonomy as a child of the selected index
    void addSubTaxonomy();
    // Change selected taxonomy's color
    void changeColor();
    // Remove taxonomy associated with selected index
    void removeSelectedTaxonomy();

  protected:
    EspinaModel *m_baseModel;
    ViewManager *m_viewManager;
    TaxonomyColorEnginePtr m_engine;

    GUI *m_gui;

    QSharedPointer<QSortFilterProxyModel> m_sort;
  };

} // namespace EspINA

#endif // TAXONOMYEXPLORER_H
