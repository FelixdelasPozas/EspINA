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

#ifndef ESPINA_STACK_EXPLORER_H
#define ESPINA_STACK_EXPLORER_H

// ESPINA
#include <Core/Types.h>
#include <Support/Widgets/Panel.h>
#include <Support/Context.h>
#include <GUI/Model/Proxies/ChannelProxy.h>

//Qt
#include <QSortFilterProxyModel>

class QObject;

namespace ESPINA
{
  class ChannelInspector;
  class ViewManager;

  /** \clas StackExplorer
   * \brief Stack exploration panel.
   *
   */
  class StackExplorer
  : public Panel
  {
    Q_OBJECT

    class CentralWidget;

  public:
    /** \brief StackExplorer class constructor.
     * \param[in] context application context.
     * \param[in] parent QWidget parent of this one.
     *
     */
    explicit StackExplorer(Support::Context &context, QWidget *parent = nullptr);

    /** \brief StackExplorer class virtual destructor.
     *
     */
    virtual ~StackExplorer();

  public slots:
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
    void updateTooltips(int index);
    /******************************************************/

    /** \brief Sets the currently selected channel as the active one.
     *
     */
    void activateChannel();

    /** \brief Removes the channel from the analysis.
     *
     */
    void unloadChannel();

    /** \brief Updates the channel representation.
     * \param[in] index item index in the tree.
     *
     */
    void updateChannelRepresentations(QModelIndex index);

    /** \brief Opens a channel inspector dialog.
     *
     */
    void showInformation();

    /** \brief Changes the sample association of a channel that has been dragged onto a sample.
     *
     */
    void channelsDragged(ChannelAdapterList channel, SampleAdapterPtr sample);

    /** \brief Updates the active channel on the channel proxy.
     *
     */
    void onActiveChannelChanged(ChannelAdapterPtr channel);

    /** \brief Updates the unload button state.
     *
     */
    void onChannelsModified();

    virtual void contextMenuEvent(QContextMenuEvent *);

  private:
    std::shared_ptr<ChannelProxy>          m_channelProxy;
    std::shared_ptr<QSortFilterProxyModel> m_sort;

    CentralWidget *m_gui;
  };

} // namespace ESPINA

#endif // ESPINA_STACK_EXPLORER_H
