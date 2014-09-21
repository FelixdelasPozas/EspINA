/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include <Core/EspinaTypes.h>
#include <Support/Widgets/DockWidget.h>
#include <GUI/Model/Proxies/ChannelProxy.h>

//Qt
#include <QSortFilterProxyModel>

class QObject;

namespace ESPINA
{
  class ChannelInspector;
  class ViewManager;

  class ChannelExplorer
  : public DockWidget
  {
    Q_OBJECT
    class CentralWidget;
  public:
    /** brief ChannelExplorer class constructor.
     * \param[in] model, model adapter smart pointer.
     * \param[in] viewManager, view manager smart pointer.
     * \param[in] scheduler, scheduler smart pointer.
     * \param[in] undoStack, QUndoStack raw pointer.
     * \param[in] parent, parent widget raw pointer.
     *
     */
    explicit ChannelExplorer(ModelAdapterSPtr model,
                             ViewManagerSPtr  viewManager,
                             SchedulerSPtr    scheduler,
                             QUndoStack      *undoStack,
                             QWidget         *parent = nullptr);

    /** brief ChannelExplorer class virtual destructor.
     *
     */
    virtual ~ChannelExplorer();

  public slots:
    /** brief Implements DockWidget::reset().
     *
     */
    virtual void reset();

  protected slots:

  	/******************************************************/
  	/* TODO: TILING/UNLOAD CHANNEL PENDING IMPLEMENTATION */
    void channelSelected();
    void alignLeft();
    void alignCenter();
    void alignRight();
    void moveRight();
    void moveLelft();
    void updateChannelPosition();
    void unloadChannel();
    void focusOnChannel();
    void updateTooltips(int index);
  	/******************************************************/

    /** brief Opens a channel inspector dialog.
     *
     */
    void showInformation();

    /** brief Sets the selected channel as the active one.
     *
     */
    void activateChannel();

    /** brief Deletes the closed dialog from the opened dialog lists and updates representations.
     * \param[in] object, raw pointer of the closed dialog.
     *
     */
    void dialogClosed(QObject *);

    /** brief Resets the views.
     *
     */
    void inspectorChangedSpacing();

    /** brief Changes the sample association of a channel that has been dragged onto a sample.
     *
     */
    void channelsDragged(ChannelAdapterList channel, SampleAdapterPtr sample);

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

} // namespace ESPINA

#endif // ESPINA_CHANNEL_EXPLORER_H
