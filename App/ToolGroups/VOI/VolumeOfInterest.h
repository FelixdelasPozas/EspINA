/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#ifndef VOLUMEOFINTEREST_H
#define VOLUMEOFINTEREST_H

#include <QToolBar>

#include <Core/Model/EspinaModel.h>
#include <Core/Interfaces/IToolBar.h>
#include <GUI/Pickers/PixelSelector.h>
#include <GUI/ViewManager.h>

#include <QMap>

class ActionSelector;
class QAction;

namespace EspINA
{
  class ViewManager;

  /// Volume Of Interest Toolbar
  class VolumeOfInterest
  : public IToolBar
  {
    Q_OBJECT
  public:
    explicit VolumeOfInterest(EspinaModel *model,
                              ViewManager *viewManager,
                              QWidget     *parent=NULL);
    virtual ~VolumeOfInterest();

    virtual void initToolBar(EspinaModel *model,
                             QUndoStack  *undoStack,
                             ViewManager *viewManager);
    virtual void resetToolbar();

    virtual void abortOperation() {};

  protected slots:
    void changeVOI(QAction *action);
    void cancelVOI();


  private:
    void buildVOIs();

  private:
    EspinaModel *m_model;
    ViewManager *m_viewManager;

    ActionSelector  *m_voiSelector;
    QMap<QAction *, IVOISPtr> m_vois;
  };

} // namespace EspINA

#endif// VOLUMEOFINTEREST_H
