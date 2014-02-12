/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2014 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// EspINA
#include "CachedRepresentation.h"
#include "CachedRepresentationTask.h"
#include <GUI/View/RenderView.h>

// VTK
#include <vtkAssembly.h>
#include <vtkMapper.h>
#include <vtkDataSet.h>
#include <vtkImageMapper3D.h>
#include <vtkProperty.h>
#include <vtkImageProperty.h>

namespace EspINA
{
  int CachedRepresentation::WINDOW_INCREMENT = 5;

  //-----------------------------------------------------------------------------
  CachedRepresentation::CachedRepresentation(RenderView *view)
  : Representation(view)
  , m_windowWidth{5}
  , m_maxWindowWidth{15}
  , m_actualPos{new CacheNode()}
  , m_edgePos{m_actualPos}
  , m_actor{nullptr}
  , m_symbolicActor{nullptr}
  {
    m_actualPos->next = m_actualPos->previous = m_actualPos;

    CacheNode * node = m_edgePos = m_actualPos;

    for (unsigned int i = 0; i < m_windowWidth; ++i)
    {
      m_edgePos->next = new CacheNode();
      m_edgePos->next->previous = m_edgePos;
      m_edgePos = m_edgePos->next;

      node->previous = new CacheNode();
      node->previous->next = node;
      node = node->previous;
    }

    node->previous = m_edgePos;
    m_edgePos->next = node;
  }
  
  //-----------------------------------------------------------------------------
  CachedRepresentation::~CachedRepresentation()
  {
    // delete actors in view
    if (m_actualPos->actor != nullptr)
    {
      m_actor->RemovePart(m_actualPos->actor);
      if (m_symbolicActor != nullptr)
      {
        m_symbolicActor->GetProperty()->SetOpacity(opacity());
        m_actor->AddPart(m_symbolicActor);
      }
      m_actor->Modified();
    }

    if (m_view != nullptr)
      m_view->removeActor(m_actor);

    m_actor = nullptr;

    // delete all nodes
    CacheNode *node;
    for (unsigned int i = 0; i < 2*m_windowWidth + 1; ++i)
    {
      node = m_actualPos;
      m_actualPos = m_actualPos->next;

      deleteTask(node);
      node->actor = nullptr;
      delete node;
    }
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::deleteTask(CacheNode *node)
  {
    if (node->worker == nullptr)
      return;

    node->worker->disconnect(node->worker.get(), SIGNAL(finished()), this, SLOT(addActor()));
    node->worker->abort();
    node->worker.reset();
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::clearCache()
  {
    if (m_actualPos->actor != nullptr)
    {
      m_actor->RemovePart(m_actualPos->actor);
      if (m_symbolicActor != nullptr)
      {
        m_symbolicActor->GetProperty()->SetOpacity(opacity());
        m_actor->AddPart(m_symbolicActor);
      }
      m_actor->Modified();
    }

    for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
    {
      deleteTask(m_actualPos);
      m_actualPos->position = 0;
      m_actualPos->actor = nullptr;
      m_actualPos->creationTime = 0;
      m_actualPos = m_actualPos->next;
    }
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::fillCache(int position)
  {
    m_actualPos->position = position;
    m_actualPos->worker = createTask(m_actualPos->position, EspINA::Priority::VERY_HIGHT);
    if (m_actualPos->worker != nullptr)
    {
      connect(m_actualPos->worker.get(), SIGNAL(finished()), this, SLOT(addActor()), Qt::QueuedConnection);
      m_actualPos->worker->submit(m_actualPos->worker);
    }

    CacheNode *node = m_edgePos = m_actualPos;

    for(unsigned int i = 0; i < m_windowWidth; ++i)
    {
      m_edgePos = m_edgePos->next;
      m_edgePos->position = m_edgePos->previous->position + 1;
      m_edgePos->worker = createTask(m_edgePos->position);
      if (m_edgePos->worker != nullptr)
      {
        connect(m_edgePos->worker.get(), SIGNAL(finished()), this, SLOT(addActor()), Qt::QueuedConnection);
        m_edgePos->worker->submit(m_edgePos->worker);
      }

      node = node->previous;
      node->position = node->next->position - 1;
      node->worker = createTask(node->position);
      if (node->worker != nullptr)
      {
        connect(node->worker.get(), SIGNAL(finished()), this, SLOT(addActor()), Qt::QueuedConnection);
        node->worker->submit(node->worker);
      }
    }

    Q_ASSERT(node->previous = m_edgePos);
    Q_ASSERT(m_edgePos->next = node);
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::setWindowWidth(unsigned int proposedWidth)
  {
    if (proposedWidth > m_maxWindowWidth)
      proposedWidth = m_maxWindowWidth;

    int diff = abs(proposedWidth - m_windowWidth);
    bool smaller = (proposedWidth - m_windowWidth) < 0;

    CacheNode *node = m_edgePos->next;

    for(int i = 0; i < diff; ++i)
    {
      if (smaller)
      {
        deleteTask(m_edgePos);
        m_edgePos->actor = nullptr;
        deleteTask(node);
        node->actor = nullptr;

        m_edgePos = m_edgePos->previous;
        node = node->next;

        delete m_edgePos->next;
        delete node->previous;
      }
      else
      {
        m_edgePos->next = new CacheNode();
        m_edgePos->next->previous = m_edgePos;
        m_edgePos = m_edgePos->next;
        m_edgePos->position = m_edgePos->previous->position + 1;
        m_edgePos->worker = createTask(m_edgePos->position);
        if (m_edgePos->worker != nullptr)
        {
          connect(m_edgePos->worker.get(), SIGNAL(finished()), this, SLOT(addActor()), Qt::QueuedConnection);
          m_edgePos->worker->submit(m_edgePos->worker);
        }

        node->previous = new CacheNode();
        node->previous->next = node;
        node = node->previous;
        node->position = node->next->position - 1;
        node->worker = createTask(node->position);
        if (node->worker != nullptr)
        {
          connect(node->worker.get(), SIGNAL(finished()), this, SLOT(addActor()), Qt::QueuedConnection);
          node->worker->submit(node->worker);
        }
      }
    }

    m_edgePos->next = node;
    node->previous = m_edgePos;

    m_windowWidth = proposedWidth;
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::setWindowMaximumWidth(unsigned int width)
  {
    m_maxWindowWidth = width;

    if (m_maxWindowWidth < m_windowWidth)
    {
      setWindowWidth(m_maxWindowWidth);
    }
  }

  //-----------------------------------------------------------------------------
  unsigned long long CachedRepresentation::getEstimatedMemoryUsed()
  {
    unsigned long long size = 0;
    CacheNode *node = m_actualPos;

    for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
    {
      if (node->actor != nullptr)
        size += node->actor->GetMapper()->GetInput()->GetActualMemorySize();

      node = node->next;
    }

    return size;
  }

  //-----------------------------------------------------------------------------
  double CachedRepresentation::getAverageTaskTime()
  {
    if(m_time.taskNum == 0)
      return 0;

    return m_time.usec / m_time.taskNum;
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::addActor()
  {
    auto task = static_cast<CachedRepresentationTask *>(sender());
    if (task == nullptr)
      return;

    // need to find the node of this task
    CacheNode *node = m_actualPos;
    for (unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
    {
      if (node->worker.get() == task)
        break;

      node = node->next;
    }

    if (node->worker.get() != task)
    {
      task->disconnect(task, SIGNAL(finished()), this, SLOT(addActor()));
      delete task;

      return;
    }

    node->actor = task->getActor();
    if (node->actor == nullptr)
      return;

    node->creationTime = task->getCreationTime();
    node->worker->disconnect(node->worker.get(), SIGNAL(finished()), this, SLOT(addActor()));
    node->worker = nullptr;

    m_time.usec += task->getExecutionTime();
    m_time.taskNum++;

    if (m_actualPos->position == node->position)
    {
      if (m_symbolicActor != nullptr)
        m_actor->RemovePart(m_symbolicActor);

      node->actor->SetOpacity(opacity());
      m_actor->AddPart(node->actor);
      m_actor->Modified();
      m_view->updateView();
    }
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::printBufferInfo()
  {
    CacheNode *node = m_edgePos->next;
    QString info = "| ";
    for(unsigned int i = 0; i < (2*m_windowWidth)+1; ++i)
    {
      if ((node->worker == nullptr) && (node->actor == nullptr))
        info += QString("X");
      else
        info += QString::number(node->position);

      info += QString(" | ");

      node = node->next;
    }

    auto memory = getEstimatedMemoryUsed();
    qDebug() << info << "average access:" << getAverageTaskTime() << "memory used:" << memory << "KB (" << memory/1024 << "MB)";
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::setPosition(int position)
  {
    if (m_actualPos->position == position)
      return;

    // check if is a complete reposition of the cache, this doesn't count as
    // a cache miss
    if (abs(m_actualPos->position - position) > static_cast<int>(m_windowWidth))
    {
      clearCache();
      fillCache(position);
      return;
    }

    // remove position actor
    if (m_actualPos->actor)
      m_actor->RemovePart(m_actualPos->actor);

    if (position < m_actualPos->position)
    {
      // left shift
      while(m_actualPos->position != position)
      {
        m_actualPos = m_actualPos->previous;

        deleteTask(m_edgePos);
        m_edgePos->position = m_edgePos->next->position - 1;
        m_edgePos->actor = nullptr;
        m_edgePos->creationTime = 0;
        m_edgePos->worker = createTask(m_edgePos->position);
        if (m_edgePos->worker)
        {
          connect(m_edgePos->worker.get(), SIGNAL(finished()), this, SLOT(addActor()), Qt::QueuedConnection);
          m_edgePos->worker->submit(m_edgePos->worker);
        }
        m_edgePos = m_edgePos->previous;
      }
    }
    else
    {
      // right shift
      while (m_actualPos->position != position)
      {
        m_actualPos = m_actualPos->next;

        m_edgePos = m_edgePos->next;
        deleteTask(m_edgePos);
        m_edgePos->position = m_edgePos->previous->position + 1;
        m_edgePos->actor = nullptr;
        m_edgePos->creationTime = 0;
        m_edgePos->worker = createTask(m_edgePos->position);
        if (m_edgePos->worker)
        {
          connect(m_edgePos->worker.get(), SIGNAL(finished()), this, SLOT(addActor()), Qt::QueuedConnection);
          m_edgePos->worker->submit(m_edgePos->worker);
        }
      }
    }

    // add actor if possible
    if (m_actualPos->actor != nullptr)
    {
      m_actualPos->actor->SetOpacity(opacity());
      m_actor->AddPart(m_actualPos->actor);
    }
    else
    {
      // cache miss, make the window bigger. the actor will be added when the task finishes so for
      // the moment the symbolic actor will be shown (if any).
      if (m_symbolicActor != nullptr)
      {
        m_symbolicActor->GetProperty()->SetOpacity(opacity());
        m_actor->AddPart(m_symbolicActor);
      }

      setWindowWidth(m_windowWidth + WINDOW_INCREMENT);
    }

    m_actor->Modified();
//    printBufferInfo();
  }

} /* namespace EspINA */
