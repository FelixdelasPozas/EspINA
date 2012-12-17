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


#ifndef CHANNELEXPLORER_H
#define CHANNELEXPLORER_H

//----------------------------------------------------------------------------
// File:    ChannelExplorer.h
// Purpose: Dock widget to manage channels in the model
//----------------------------------------------------------------------------
#include <QDockWidget>

// EspINA
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Proxies/ChannelProxy.h>

//Qt
#include <QSortFilterProxyModel>

#ifdef TEST_ESPINA_MODELS
class ModelTest;
#endif

namespace EspINA
{
  class ViewManager;

  class ChannelExplorer
  : public QDockWidget
  {
    Q_OBJECT
    class CentralWidget;
  public:
    explicit ChannelExplorer(EspinaModelPtr model,
                             ViewManager   *vm,
                             QWidget       *parent = 0);
    virtual ~ChannelExplorer();

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
    void changeChannelColor();
    void updateChannelPosition();
    void updateTooltips(int index);
    void focusOnChannel();

  private:
    EspinaModelPtr m_model;
    ViewManager   *m_viewManager;

    QSharedPointer<ChannelProxy> m_channelProxy;
    QSharedPointer<QSortFilterProxyModel> m_sort;

    CentralWidget *m_gui;

    #ifdef TEST_ESPINA_MODELS
    QSharedPointer<ModelTest>   m_modelTester;
    #endif
  };

} // namespace EspINA

#endif // CHANNELEXPLORER_H
