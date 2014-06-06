/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#include <QWidget>
#include <QTimer>
#include <Support/ui_MetadataViewer.h>
#include <GUI/Model/ChannelAdapter.h>

namespace EspINA {

  class MetadataLoader;
  using MetadataLoaderSPtr = std::shared_ptr<MetadataLoader>;

  class MetadataViewer
  : public QWidget
  , private Ui::MatadataViewer
  {
    Q_OBJECT
  public:
    explicit MetadataViewer(const ChannelAdapterPtr channel,
                            SchedulerSPtr           scheduler,
                            QWidget*                parent = 0,
                            Qt::WindowFlags         f = 0);

    virtual void showEvent(QShowEvent *event);

  private slots:
    void upadteMessage();

    void metadataReady();

  private:
    const ChannelAdapterPtr m_channel;
    SchedulerSPtr           m_scheduler;
    MetadataLoaderSPtr      m_task;

    QTimer                  m_animationTimer;
    int                     m_animationStep;
  };

} // namespace EspINA

#endif // ESPINA_METADATA_VIEWER_H