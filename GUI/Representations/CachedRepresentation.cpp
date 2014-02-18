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
#include "RepresentationSettings.h"
#include "RepresentationEmptySettings.h"
#include <GUI/View/View2D.h>

// VTK
#include <vtkAssembly.h>
#include <vtkMapper.h>
#include <vtkDataSet.h>
#include <vtkImageMapper3D.h>
#include <vtkProperty.h>
#include <vtkImageProperty.h>
#include <vtkPropCollection.h>

// Qt
#include <QThread>

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
    if (m_symbolicActor)
      m_view->removeActor(m_symbolicActor);

    // Delete all nodes, wait for threads to finish as they have a link to the
    // node and the program will crash if we delete the node earlier.
    CacheNode *node;
    for (unsigned int i = 0; i < 2*m_windowWidth + 1; ++i)
    {
      node = m_actualPos;
      node->mutex.lock();
      if (node->worker != nullptr)
      {
        node->worker->abort();
        node->mutex.unlock();
        node->worker->thread()->wait();
        node->worker = nullptr;
      }
      else
        node->mutex.unlock();

      if (node->actor != nullptr)
        m_view->removeActor(node->actor);
      node->actor = nullptr;

      m_actualPos = m_actualPos->next;
      delete node;
    }
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::clearCache()
  {
    for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
    {
      m_actualPos->mutex.lock();
      if (m_actualPos->worker != nullptr)
      {
        m_actualPos->worker->abort();
        m_actualPos->worker = nullptr;
      }
      m_actualPos->mutex.unlock();

      if (m_actualPos->actor != nullptr)
      {
        m_view->removeActor(m_actualPos->actor);
        m_actualPos->actor = nullptr;
      }
      m_actualPos->creationTime = 0;
      m_actualPos->position = 0;
      m_actualPos = m_actualPos->next;
    }

    if (m_symbolicActor != nullptr)
    {
      m_symbolicActor->GetProperty()->SetOpacity(opacity());
      m_view->addActor(m_symbolicActor);
    }
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::fillCache(int position)
  {
    // this method assumes all tasks have ended or are aborted (and on its way to
    // task heaven), that is, a previous call to clearCache();
    m_actualPos->position = position;
    m_actualPos->worker = createTask(m_actualPos, EspINA::Priority::VERY_HIGHT);
    if (m_actualPos->worker != nullptr)
    {
      connect(m_actualPos->worker.get(), SIGNAL(render(CachedRepresentation::CacheNode *)), this, SLOT(renderFrame(CachedRepresentation::CacheNode *)), Qt::QueuedConnection);
      m_actualPos->worker->submit(m_actualPos->worker);
    }

    CacheNode *node = m_edgePos = m_actualPos;

    for(unsigned int i = 0; i < m_windowWidth; ++i)
    {
      m_edgePos = m_edgePos->next;
      m_edgePos->position = m_edgePos->previous->position + 1;
      m_edgePos->worker = createTask(m_edgePos);
      if (m_edgePos->worker != nullptr)
        m_edgePos->worker->submit(m_edgePos->worker);

      node = node->previous;
      node->position = node->next->position - 1;
      node->worker = createTask(node);
      if (node->worker != nullptr)
        node->worker->submit(node->worker);
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
        m_edgePos->mutex.lock();
        if (m_edgePos->worker != nullptr)
        {
          m_edgePos->worker->abort();
          m_edgePos->worker = nullptr;
        }
        m_edgePos->mutex.unlock();
        m_edgePos->actor = nullptr;

        node->mutex.lock();
        if (node->worker != nullptr)
        {
          node->worker->abort();
          node->worker = nullptr;
        }
        node->mutex.unlock();
        node->actor = nullptr;

        m_edgePos = m_edgePos->previous;
        node = node->next;

        delete m_edgePos->next;
        delete node->previous;
      }
      else
      {
        // There are not tasks running that can overwrite this positions, so
        // no mutexes are needed.
        m_edgePos->next = new CacheNode();
        m_edgePos->next->previous = m_edgePos;
        m_edgePos = m_edgePos->next;
        m_edgePos->position = m_edgePos->previous->position + 1;
        m_edgePos->worker = createTask(m_edgePos);
        if (m_edgePos->worker != nullptr)
          m_edgePos->worker->submit(m_edgePos->worker);

        node->previous = new CacheNode();
        node->previous->next = node;
        node = node->previous;
        node->position = node->next->position - 1;
        node->worker = createTask(node);
        if (node->worker != nullptr)
          node->worker->submit(node->worker);
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

    // NOTE: no mutexes are needed because the ordering of the write accesses to
    // the actor position.
    for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
    {
      if (node->actor != nullptr)
        size += node->actor->GetMapper()->GetInput()->GetActualMemorySize();

      node = node->next;
    }

    return size;
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::printBufferInfo()
  {
    CacheNode *node = m_edgePos->next;
    QString info = "| ";
    for(unsigned int i = 0; i < (2*m_windowWidth)+1; ++i)
    {
      node->mutex.lock();
      if ((node->worker == nullptr) && (node->actor == nullptr))
        info += QString("X");
      else
        info += QString::number(node->position);
      node->mutex.unlock();
      info += QString(" | ");

      node = node->next;
    }

    auto memory = getEstimatedMemoryUsed();
    qDebug() << info << "memory used:" << memory << "KB (" << memory/1024 << "MB)";
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

    // remove position actor if any
    m_actualPos->mutex.lock();
    if (m_actualPos->worker != nullptr)
      disconnect(m_actualPos->worker.get(), SIGNAL(render(CachedRepresentation::CacheNode *)), this, SLOT(renderFrame(CachedRepresentation::CacheNode *)));
    else
      if (m_actualPos->actor)
        m_view->removeActor(m_actualPos->actor);
    m_actualPos->mutex.unlock();

    if (position < m_actualPos->position)
    {
      // left shift
      while(m_actualPos->position != position)
      {
        m_actualPos = m_actualPos->previous;

        m_edgePos->position = m_edgePos->next->position - 1;
        auto task = createTask(m_edgePos);

        m_edgePos->mutex.lock();
        if (m_edgePos->worker != nullptr)
          m_edgePos->worker->abort();
        m_edgePos->worker = task;
        m_edgePos->mutex.unlock();

        m_edgePos->actor = nullptr;
        m_edgePos->creationTime = 0;
        if (task != nullptr)
          task->submit(task);

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

        m_edgePos->position = m_edgePos->previous->position + 1;
        auto task = createTask(m_edgePos);

        m_edgePos->mutex.lock();
        if (m_edgePos->worker != nullptr)
          m_edgePos->worker->abort();
        m_edgePos->worker = task;
        m_edgePos->mutex.unlock();

        m_edgePos->actor = nullptr;
        m_edgePos->creationTime = 0;
        if (task != nullptr)
          task->submit(task);
      }
    }

    // add actor if possible
    m_actualPos->mutex.lock();
    if (m_actualPos->actor != nullptr)
    {
      m_actualPos->mutex.unlock();
      if (m_symbolicActor)
        m_view->removeActor(m_symbolicActor);

      m_actualPos->actor->SetOpacity(opacity());
      m_actualPos->actor->SetVisibility(isVisible());
      m_view->addActor(m_actualPos->actor);
    }
    else
    {
      // connect to this task render signal
      if (m_actualPos->worker != nullptr)
        connect(m_actualPos->worker.get(), SIGNAL(render(CachedRepresentation::CacheNode *)), this, SLOT(renderFrame(CachedRepresentation::CacheNode *)), Qt::QueuedConnection);
      m_actualPos->mutex.unlock();

      // cache miss, make the window bigger. the actor will be added when the task finishes so for
      // the moment the symbolic actor will be shown (if any).
      if (m_symbolicActor != nullptr)
      {
        m_symbolicActor->GetProperty()->SetOpacity(opacity());
        m_symbolicActor->SetVisibility(isVisible());
        m_view->addActor(m_symbolicActor);
      }

      setWindowWidth(m_windowWidth + WINDOW_INCREMENT);
    }

//    printBufferInfo();
  }

  //-----------------------------------------------------------------------------
  bool CachedRepresentation::hasActor(vtkProp *actor) const
  {
    if (m_view == nullptr)
      return false;

    QMutexLocker lock(&m_actualPos->mutex);

    if (m_actualPos->actor != nullptr)
      return (m_actualPos->actor.Get() == actor);
    else
      return (m_symbolicActor.Get() == actor);
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::setColor(const QColor &color)
  {
    Representation::setColor(color);

    int position = m_actualPos->position;
    clearCache();
    fillCache(position);

    if (m_symbolicActor != nullptr)
    {
      m_symbolicActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
      m_symbolicActor->Modified();
    }
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::setBrightness(double brightness)
  {
    Representation::setBrightness(brightness);

    int position = m_actualPos->position;
    clearCache();
    fillCache(position);
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::setContrast(double contrast)
  {
    Representation::setContrast(contrast);

    int position = m_actualPos->position;
    clearCache();
    fillCache(position);
  }

  //-----------------------------------------------------------------------------
  QList<vtkProp*> CachedRepresentation::getActors()
  {
    QList<vtkProp *> list;
    QMutexLocker lock(&m_actualPos->mutex);
    if (m_actualPos->actor != nullptr)
      list << m_actualPos->actor;
    else
      list << m_symbolicActor;

    return list;
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::updateVisibility(bool visible)
  {
    QMutexLocker lock(&m_actualPos->mutex);

    if (m_actualPos->actor != nullptr)
    {
      m_actualPos->actor->SetVisibility(visible);
      m_actualPos->actor->Modified();
    }
    else
      if (m_symbolicActor != nullptr)
      {
        m_symbolicActor->SetVisibility(visible);
        m_symbolicActor->Modified();
      }
  }

  //-----------------------------------------------------------------------------
  RepresentationSettings *CachedRepresentation::settingsWidget()
  {
    return new RepresentationEmptySettings();
  }

  //-----------------------------------------------------------------------------
  void CachedRepresentation::renderFrame(CacheNode *node)
  {
    if (node != m_actualPos)
      return;

    if (m_actualPos->actor != nullptr)
    {
      if (m_symbolicActor != nullptr)
        m_view->removeActor(m_symbolicActor);
      m_actualPos->actor->SetOpacity(opacity());
      m_actualPos->actor->SetVisibility(isVisible());
      m_view->addActor(m_actualPos->actor);
      m_view->updateView();
    }
  }

  //-----------------------------------------------------------------------------
  bool CachedRepresentation::needUpdate()
  {
    return needUpdate(m_actualPos);
  }

} /* namespace EspINA */
