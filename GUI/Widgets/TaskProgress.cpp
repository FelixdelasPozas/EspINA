/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

// ESPINA
#include "TaskProgress.h"
#include <Core/MultiTasking/Task.h>

using namespace ESPINA;

TaskProgress::TaskProgress(TaskSPtr task)
: QWidget()
, m_task(task)
{
  setupUi(this);

  m_progressBar->setTextVisible(true);

  connect(m_cancelButton, SIGNAL(clicked(bool)),
          this, SLOT(onCancel()));
  connect(m_task.get(), SIGNAL(progress(int)),
          this, SLOT(updateProgress(int)));

  updateProgress(0);
}

//------------------------------------------------------------------------
TaskProgress::~TaskProgress()
{
}

//------------------------------------------------------------------------
void TaskProgress::showEvent(QShowEvent *event)
{
  QWidget::showEvent(event);
  updateProgress(m_progressBar->value());
}


//------------------------------------------------------------------------
void TaskProgress::updateProgress(int value)
{
  if(m_task->isAborted())
  {
    emit aborted();
    return;
  }

  QString text = m_task->description();

  int valueWidth = m_progressBar->fontMetrics().width(": 100%");
  int charWidht  = m_progressBar->fontMetrics().width("A");
  int maxLength  = (m_progressBar->width() - valueWidth) / charWidht;

  if (text.length() > maxLength)
  {
    text = text.left(maxLength - 4) + "... ";
  }

  m_progressBar->setFormat(text + QString(": %1%").arg(value));
  m_progressBar->setValue(value);

  if(value >= 100 || value <= 0 || m_task->hasFinished())
  {
    if(isVisible()) hide();
  }
  else
  {
    if(!isVisible()) show();
  }
}

//------------------------------------------------------------------------
void TaskProgress::onCancel()
{
  m_task->abort();

  emit aborted();
}
