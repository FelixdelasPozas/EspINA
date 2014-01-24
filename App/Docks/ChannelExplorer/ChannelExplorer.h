/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef ESPINA_CHANNEL_EXPLORER_H
#define ESPINA_CHANNEL_EXPLORER_H

//----------------------------------------------------------------------------
// File:    ChannelExplorer.h
// Purpose: Dock widget to manage channels in the model
//----------------------------------------------------------------------------
#include <Core/EspinaTypes.h>
#include <Support/DockWidget.h>

// EspINA
#include <GUI/Model/Proxies/ChannelProxy.h>

//Qt
#include <QSortFilterProxyModel>

class QObject;

namespace EspINA
{
  class ChannelInspector;
  class ViewManager;

  class ChannelExplorer
  : public DockWidget
  {
    Q_OBJECT
    class CentralWidget;
  public:
    explicit ChannelExplorer(ModelAdapterSPtr model,
                             ViewManagerSPtr  viewManager,
                             SchedulerSPtr    scheduler,
                             QUndoStack      *undoStack,
                             QWidget         *parent = 0);
    virtual ~ChannelExplorer();

    virtual void initDockWidget(ModelAdapterSPtr model,
                                QUndoStack      *undoStack,
                                ViewManagerSPtr  viewManager);

    virtual void reset(); // slot

  protected slots:
    void channelSelected();
    void showInformation();
    void activateChannel();
    void unloadChannel();
    void alignLeft();
    void alignCenter();
    void alignRight();
    void moveRight();
    void moveLelft();
    void updateChannelPosition();
    void updateTooltips(int index);
    void focusOnChannel();
    void dialogClosed(QObject *);
    void inspectorChangedSpacing();

  private:
    ModelAdapterSPtr m_model;
    ViewManagerSPtr  m_viewManager;
    SchedulerSPtr    m_scheduler;
    QUndoStack      *m_undoStack;

    std::shared_ptr<ChannelProxy>          m_channelProxy;
    std::shared_ptr<QSortFilterProxyModel> m_sort;

    CentralWidget *m_gui;
    QMap<ChannelAdapterPtr, ChannelInspector *> m_informationDialogs;
  };

} // namespace EspINA

#endif // ESPINA_CHANNEL_EXPLORER_H
