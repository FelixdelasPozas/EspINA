/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_METADATA_VIEWER_H
#define ESPINA_METADATA_VIEWER_H

#include "Support/EspinaSupport_Export.h"

#include <QWidget>
#include <QTimer>
#include <Support/ui_MetadataViewer.h>
#include <GUI/Model/ChannelAdapter.h>

namespace ESPINA
{
  class MetadataLoader;
  using MetadataLoaderSPtr = std::shared_ptr<MetadataLoader>;

  /** \class MetadataViewer
   * \brief Class to show a dialog with metadata.
   *
   */
  class EspinaSupport_EXPORT MetadataViewer
  : public QWidget
  , private Ui::MatadataViewer
  {
    Q_OBJECT
  public:
    /** \brief MetadataViewer class constructor.
     * \param[in] channel stack of the metadata.
     * \param[in] scheduler application task scheduler.
     * \param[in] parent pointer of the widget parent of this one.
     * \param[in] flags window flags
     */
    explicit MetadataViewer(const ChannelAdapterPtr channel,
                            SchedulerSPtr           scheduler,
                            QWidget*                parent = nullptr,
                            Qt::WindowFlags         flags = 0);

    virtual void showEvent(QShowEvent *event);

  private slots:
    /** \brief Animates the waiting message.
     *
     */
    void updateMessage();

    /** \brief Updates the dialog with the recovered metadata.
     *
     */
    void metadataReady();

  private:
    const ChannelAdapterPtr m_channel;        /** stack of the metadata.           */
    SchedulerSPtr           m_scheduler;      /** application task scheduler.      */
    MetadataLoaderSPtr      m_retrieverTask;  /** metadata retriever task.         */

    QTimer                  m_animationTimer; /** timer for the animation message. */
    int                     m_animationStep;  /** step of the animation.           */
  };

} // namespace ESPINA

#endif // ESPINA_METADATA_VIEWER_H
