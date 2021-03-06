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

#ifndef ESPINA_TASK_PROGRESS_H
#define ESPINA_TASK_PROGRESS_H

#include "GUI/EspinaGUI_Export.h"

// ESPINA
#include <Core/MultiTasking/Task.h>

// Qt
#include <ui_TaskProgress.h>
#include <QWidget>

// C++
#include <memory>

namespace ESPINA
{
  class Task;

  /** \class TaskProgress
   * \brief Implements a progress widget for a task.
   *
   */
  class EspinaGUI_EXPORT TaskProgress
  : public QWidget
  , public Ui::TaskProgress
  {
      Q_OBJECT
    public:
      /** \brief TaskProgress class constructor.
       *
       */
      explicit TaskProgress(TaskSPtr task);

      /** \brief TaskProgress class virtual destructor.
       *
       */
      virtual ~TaskProgress();

      /** \brief Returns the process of the task.
       *
       */
      int progress()
      { return m_progressBar->value(); }

      /** \brief Returns the task smart pointer.
       *
       */
      TaskSPtr task() const
      { return m_task; }

    signals:
      void aborted();

    public slots:
      /** \brief Aborts the task.
       *
       */
      void onCancel();

    protected:
      /** \brief Overrides QWidget::showEvent().
       *
       */
      virtual void showEvent(QShowEvent *event) override;

    private slots:
      /** \brief Updates the progress bar value and text
       * \param[in] value progress value.
       *
       */
      void updateProgress(int value);

    private:
      TaskSPtr m_task;
  };

  using TaskProgressSPtr = std::shared_ptr<TaskProgress>;
}

#endif // ESPINA_TASK_PROGRESS_H
