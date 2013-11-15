/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "TaskProgress.h"

#include <QLayout>
#include <QProgressBar>

using namespace EspINA;

//------------------------------------------------------------------------
TaskProgress::TaskProgress(SchedulerSPtr   scheduler,
                           QWidget        *parent,
                           Qt::WindowFlags f)
: QWidget(parent, f)
, m_scheduler(scheduler)
{
  auto layout = new QHBoxLayout();

  auto progressBar = new QProgressBar(this);

  layout->addWidget(progressBar);

  setLayout(layout);
}
