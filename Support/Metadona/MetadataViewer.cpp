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
#include "MetadataViewer.h"

#include <Utils.h>
#include "EntryWidget.h"
#include "StorageFactory.h"

#include <QVBoxLayout>
#include <QScrollArea>
#include <QApplication>

using namespace EspINA;

namespace EspINA
{
  class MetadataLoader
  : public Task
  {
  public:
    explicit MetadataLoader(ChannelAdapterPtr channel, SchedulerSPtr scheduler)
    : Task(scheduler)
    , m_channel(channel)
    {}

  private:
    virtual void run()
    {
      auto storage  = StorageFactory::newStorage();
      Metadata = Metadona::Utils::parse(m_channel->metadata().toStdString(), storage);
    }

  public:
    Metadona::Metadata Metadata;

  private:
    ChannelAdapterPtr m_channel;
  };
}


//------------------------------------------------------------------------
MetadataViewer::MetadataViewer(const ChannelAdapterPtr channel,
                               SchedulerSPtr           scheduler,
                               QWidget*                parent,
                               Qt::WindowFlags         f)
: QWidget(parent, f)
, m_channel(channel)
, m_scheduler(scheduler)
, m_animationStep(0)
{
  setupUi(this);

  m_scrollArea->setVisible(false);

  m_animationTimer.setInterval(500);
  connect(&m_animationTimer, SIGNAL(timeout()),
          this,              SLOT(upadteMessage()));
}

//------------------------------------------------------------------------
void MetadataViewer::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);

  if (!m_task)
  {
    m_task = MetadataLoaderSPtr{new MetadataLoader(m_channel, m_scheduler)};
    connect(m_task.get(), SIGNAL(finished()),
            this, SLOT(metadataReady()));

    Task::submit(m_task);

    m_animationTimer.start();
  }
}

//------------------------------------------------------------------------
void MetadataViewer::upadteMessage()
{
  QString message = tr("Retrieving Metadata from servers");

  for (int i = 0; i <= m_animationStep; ++i)
  {
    message += ".";
  }

  m_animationStep = (m_animationStep + 1) % 3;

  m_loadingMessage->setText(message);
}

//------------------------------------------------------------------------
void MetadataViewer::metadataReady()
{
  auto metadataLoader = dynamic_cast<MetadataLoader *>(sender());

  auto layout = scrollAreaWidgetContents->layout();

  for (auto& entry : metadataLoader->Metadata)
  {
    layout->addWidget(new EntryWidget(entry));
  }

  m_scrollArea->setVisible(true);
  m_loadingWidgets->setVisible(false);
}