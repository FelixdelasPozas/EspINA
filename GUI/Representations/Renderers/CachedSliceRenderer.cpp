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
#include "CachedSliceRenderer.h"
#include "CachedSliceRendererTask.h"
#include <GUI/View/RenderView.h>
#include <GUI/Representations/SliceCachedRepresentation.h>

// VTK
#include <vtkPropPicker.h>
#include <vtkPropCollection.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>

// Qt
#include <QThread>

namespace EspINA
{
  const int CachedSliceRenderer::WINDOW_INCREMENT = 5;

  //-----------------------------------------------------------------------------
  CachedSliceRenderer::CachedSliceRenderer(SchedulerSPtr scheduler, QObject *parent)
  : RepresentationRenderer(parent)
  , m_windowWidth{15}
  , m_maxWindowWidth{25}
  , m_actualPos{nullptr}
  , m_edgePos{nullptr}
  , m_picker(vtkSmartPointer<vtkPropPicker>::New())
  , m_scheduler{scheduler}
  , m_planeIndex{-1}
  {
    m_picker->PickFromListOn();
  }

  //-----------------------------------------------------------------------------
  CachedSliceRenderer::~CachedSliceRenderer()
  {
    if (m_view == nullptr)
      return;

    auto view = dynamic_cast<View2D *>(m_view);
    disconnect(view, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(changePosition(Plane, Nm)));

    CacheNode *node = m_actualPos;
    for (unsigned int i = 0; i < 2*m_windowWidth + 1; ++i)
    {
      node->mutex.lock();
      if (node->worker != nullptr)
      {
        disconnect(node->worker.get(), SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));
        node->worker->abort();
      }
      node->worker = nullptr;

      for(auto rep: node->representations.keys())
      {
        if ((node == m_actualPos) && node->representations[rep].actor != nullptr)
          m_view->removeActor(node->representations[rep].actor);

        node->representations[rep].actor = nullptr;
      }

      node->mutex.unlock();
      CacheNode *deleteNode = node;
      node = node->next;
      delete deleteNode;
    }
  }
  
  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::setView(RenderView* rView)
  {
    m_view = rView;
    auto sceneResolution = m_view->sceneResolution();

    auto view = dynamic_cast<View2D *>(m_view);
    Q_ASSERT(view);
    m_planeIndex = normalCoordinateIndex(view->plane());
    m_windowSpacing = sceneResolution[m_planeIndex];
    connect(view, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(changePosition(Plane, Nm)), Qt::QueuedConnection);
    connect(view, SIGNAL(sceneResolutionChanged()), this, SLOT(resolutionChanged()), Qt::QueuedConnection);

    initCache();
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::addRepresentation(ViewItemAdapterPtr item, RepresentationSPtr rep)
  {
    if (!managesRepresentation(rep->type()) || hasRepresentation(rep) || (m_view == nullptr))
      return;

    if (m_representations.contains(item))
      m_representations[item] << rep;
    else
    {
      RepresentationSList list;
      list << rep;
      m_representations.insert(item, list);
    }

    m_representationList << rep;
    connect(rep.get(), SIGNAL(update()), this, SLOT(updateRepresentation()), Qt::QueuedConnection);
    connect(rep.get(), SIGNAL(changeVisibility(bool)), this, SLOT(updateRepresentationVisibility(bool)), Qt::QueuedConnection);

    CacheNode *node = m_actualPos;
    for(unsigned int i = 0; i < 2*m_windowWidth+1; ++i)
    {
      struct ActorData dummy;

      node->mutex.lock();
      node->representations.insert(rep, dummy);

      if(node->worker == nullptr)
      {
        node->worker = createTask((node == m_actualPos) ? Priority::VERY_HIGHT : Priority::NORMAL);
        RepresentationSList repList;
        repList << rep;
        node->worker->setDescription(QString("ADD %1 - plane %2").arg(node->position).arg(m_planeIndex));
        node->worker->setInput(node, repList);
        node->worker->submit(node->worker);
      }
      else
      {
        node->repsToAdd << rep;
        if (node->repsToDelete.contains(rep))
          node->repsToDelete.removeOne(rep);
      }
      node->mutex.unlock();

      node = node->next;
    }
  }
  
  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::removeRepresentation(RepresentationSPtr rep)
  {
    if (!hasRepresentation(rep))
      return;

    for(auto item: m_representations.keys())
      if (m_representations[item].contains(rep))
      {
        m_representations[item].removeOne(rep);
        if (m_representations[item].empty())
          m_representations.remove(item);
      }

    m_representationList.removeOne(rep);
    disconnect(rep.get(), SIGNAL(update()), this, SLOT(updateRepresentation()));
    disconnect(rep.get(), SIGNAL(changeVisibility(bool)), this, SLOT(updateRepresentationVisibility(bool)));

    CacheNode *node = m_actualPos;
    for(unsigned int i = 0; i < 2*m_windowWidth+1; ++i)
    {
      node->mutex.lock();
      if (node->worker == nullptr)
      {
        if (m_actualPos == node && node->representations[rep].actor != nullptr)
          m_view->removeActor(node->representations[rep].actor);

        node->representations[rep].actor = nullptr;
        node->representations.remove(rep);
      }
      else
      {
        node->repsToDelete << rep;
        if (node->repsToAdd.contains(rep))
          node->repsToAdd.removeOne(rep);
      }
      node->mutex.unlock();

      node = node->next;
    }
  }
  
  //-----------------------------------------------------------------------------
  bool CachedSliceRenderer::hasRepresentation(RepresentationSPtr rep) const
  {
    if (managesRepresentation(rep->type()))
      return m_representationList.contains(rep);

    return false;
  }
  
  //-----------------------------------------------------------------------------
  bool CachedSliceRenderer::managesRepresentation(const QString& repName) const
  {
    return ((repName == ChannelSliceCachedRepresentation::TYPE) || (repName == SegmentationSliceCachedRepresentation::TYPE));
  }
  
  //-----------------------------------------------------------------------------
  unsigned int CachedSliceRenderer::numberOfvtkActors() const
  {
    m_actualPos->mutex.lock();
    int numActors = 0;
    for(auto rep: m_actualPos->representations.keys())
      if (m_actualPos->representations[rep].actor != nullptr)
        ++numActors;
    m_actualPos->mutex.unlock();

    return numActors;
  }
  
  //-----------------------------------------------------------------------------
  ViewItemAdapterList CachedSliceRenderer::pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType, bool repeat)
  {
    ViewItemAdapterList selection;
    View2D *view = reinterpret_cast<View2D *>(m_view);

    if (!renderer || !renderer.GetPointer() || (!itemType.testFlag(RenderableType::CHANNEL) && !itemType.testFlag(RenderableType::SEGMENTATION)))
      return selection;

    Nm pickPoint[3] = { static_cast<Nm>(x), static_cast<Nm>(y), ((view->plane() == Plane::XY) ? -View2D::SEGMENTATION_SHIFT : View2D::SEGMENTATION_SHIFT) };

    m_actualPos->mutex.lockInline();
    for(auto rep: m_actualPos->representations.keys())
      if (m_actualPos->representations[rep].actor != nullptr)
        m_picker->AddPickList(m_actualPos->representations[rep].actor);
    m_actualPos->mutex.unlockInline();

    while (m_picker->Pick(pickPoint, renderer))
    {
      vtkProp *pickedProp = m_picker->GetViewProp();
      Q_ASSERT(pickedProp);

      Nm point[3];
      m_picker->GetPickPosition(point);
      point[m_planeIndex] = z;

      m_picker->DeletePickList(pickedProp);

      NmVector3 vecPoint{ point[0], point[1], point[2] };
      RepresentationSPtr pickedRepresentation;

      m_actualPos->mutex.lockInline();
      for(auto rep: m_actualPos->representations.keys())
        if(pickedProp == dynamic_cast<vtkProp *>(m_actualPos->representations[rep].actor.GetPointer()))
          pickedRepresentation = rep;
      m_actualPos->mutex.unlockInline();

      for (auto item: m_representations.keys())
      {
        if (!((item->type() == ViewItemAdapter::Type::CHANNEL && itemType.testFlag(RenderableType::CHANNEL)) ||
              (item->type() == ViewItemAdapter::Type::SEGMENTATION && itemType.testFlag(RenderableType::SEGMENTATION))))
          continue;

        if (m_representations[item].contains(pickedRepresentation))
        {
          if (pickedRepresentation->isVisible() && pickedRepresentation->isInside(vecPoint) && !selection.contains(item))
          {
            selection << item;

            if (!repeat)
            {
              auto collection = m_picker->GetPickList();
              collection->InitTraversal();
              auto prop = collection->GetNextProp();
              while(prop != nullptr)
              {
                m_picker->DeletePickList(prop);
                prop = collection->GetNextProp();
              }

              return selection;
            }

            break;
          }
        }
      }
    }

    auto collection = m_picker->GetPickList();
    collection->InitTraversal();
    auto prop = collection->GetNextProp();
    while(prop != nullptr)
    {
      m_picker->DeletePickList(prop);
      prop = collection->GetNextProp();
    }

    return selection;
  }
  
  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::resolutionChanged()
  {
    auto sceneResolution = m_view->sceneResolution();
    auto spacing = sceneResolution[m_planeIndex];

    if (m_windowSpacing != spacing)
    {
      m_windowSpacing = spacing;
      int iPos = m_actualPos->position / spacing; // try to adjust actual position;

      clearCache();
      fillCache(iPos * m_windowSpacing);
    }
  }

  //-----------------------------------------------------------------------------
  NmVector3 CachedSliceRenderer::pickCoordinates() const
  {
    double point[3];
    m_picker->GetPickPosition(point);

    return NmVector3{point[0], point[1], point[2]};
  }
  
  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::hide()
  {
    m_actualPos->mutex.lockInline();
    for(auto rep: m_actualPos->representations.keys())
      if(m_actualPos->representations[rep].actor != nullptr)
        m_view->removeActor(m_actualPos->representations[rep].actor);
    m_actualPos->mutex.unlockInline();
  }
  
  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::show()
  {
    m_actualPos->mutex.lockInline();
    for(auto rep: m_actualPos->representations.keys())
      if(m_actualPos->representations[rep].actor != nullptr)
        m_view->addActor(m_actualPos->representations[rep].actor);
    m_actualPos->mutex.unlockInline();
  }

  //-----------------------------------------------------------------------------
  CachedSliceRendererTaskSPtr CachedSliceRenderer::createTask(Priority priority)
  {
    CachedSliceRendererTask *task = new CachedSliceRendererTask(m_scheduler);
    task->setPriority(priority);
    connect(task, SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)), Qt::BlockingQueuedConnection);

    return CachedSliceRendererTaskSPtr{task};
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::setWindowWidth(unsigned int proposedWidth)
  {
    if (proposedWidth > m_maxWindowWidth)
      proposedWidth = m_maxWindowWidth;

    if (proposedWidth == m_windowWidth)
      return;

    int diff = abs(proposedWidth - m_windowWidth);
    bool smaller = (proposedWidth - m_windowWidth) < 0;

    CacheNode *node = m_edgePos->next;

    for(int i = 0; i < diff; ++i)
    {
      if (smaller)
      {
        m_edgePos->mutex.lockInline();
        if (m_edgePos->worker != nullptr)
        {
          disconnect(m_edgePos->worker.get(), SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));
          m_edgePos->worker->abort();
        }
        m_edgePos->worker = nullptr;
        m_edgePos->mutex.unlockInline();

        for (auto rep: m_edgePos->representations.keys())
          m_edgePos->representations[rep].actor = nullptr;

        node->mutex.lockInline();
        if (node->worker != nullptr)
        {
          disconnect(node->worker.get(), SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));
          node->worker->abort();
        }
        node->worker = nullptr;
        node->mutex.unlockInline();

        for (auto rep: node->representations.keys())
          node->representations[rep].actor = nullptr;

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
        m_edgePos->position = m_edgePos->previous->position + m_windowSpacing;
        m_edgePos->worker = createTask();
        m_edgePos->worker->setInput(m_edgePos, m_representationList);
        m_edgePos->worker->setDescription(QString("ALL %1 - plane %2").arg(m_edgePos->position).arg(m_planeIndex));
        m_edgePos->worker->submit(m_edgePos->worker);

        node->previous = new CacheNode();
        node->previous->next = node;
        node = node->previous;
        node->position = node->next->position - m_windowSpacing;
        node->worker = createTask();
        node->worker->setInput(node, m_representationList);
        node->worker->setDescription(QString("ALL %1 - plane %2").arg(node->position).arg(m_planeIndex));
        node->worker->submit(node->worker);
      }
    }

    m_edgePos->next = node;
    node->previous = m_edgePos;

    m_windowWidth = proposedWidth;
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::setWindowMaximumWidth(unsigned int width)
  {
    m_maxWindowWidth = width;

    if (m_maxWindowWidth < m_windowWidth)
      setWindowWidth(m_maxWindowWidth);
  }

  //-----------------------------------------------------------------------------
  unsigned long long CachedSliceRenderer::getNodeExtimatedMemoryUsed(CacheNode *node)
  {
    unsigned long long size = 0;

    node->mutex.lockInline();
    for(auto rep: node->representations.keys())
      if (node->representations[rep].actor != nullptr)
        size += node->representations[rep].actor->GetMapper()->GetInput()->GetActualMemorySize();
    node->mutex.unlockInline();

    return size;
  }

  //-----------------------------------------------------------------------------
  unsigned long long CachedSliceRenderer::getEstimatedMemoryUsed()
  {
    unsigned long long size = 0;

    CacheNode *node = m_actualPos;
    for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
      size += getNodeExtimatedMemoryUsed(node);

    return size;
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::updateRepresentation()
  {
    ChannelSliceCachedRepresentation *channelRep = qobject_cast<ChannelSliceCachedRepresentation *>(sender());
    SegmentationSliceCachedRepresentation *segRep = qobject_cast<SegmentationSliceCachedRepresentation *>(sender());

    RepresentationSPtr rep = nullptr;
    if (segRep != nullptr)
    {
      for(auto representation: m_representationList)
        if (representation.get() == segRep)
          rep = representation;
    }
    else
    {
      for(auto representation: m_representationList)
        if (representation.get() == channelRep)
          rep = representation;
    }

    if (rep == nullptr)
      return;

    RepresentationSList repList;
    repList << rep;

    CacheNode *node = m_actualPos;

    for(unsigned int i = 0; i < (2*m_windowWidth)+1; ++i)
    {
      node->mutex.lockInline();
      if(node == m_actualPos && node->representations[rep].actor != nullptr)
        m_view->removeActor(node->representations[rep].actor);
      node->representations[rep].actor = nullptr;

      if(node->worker != nullptr)
      {
        node->repsToAdd << rep;
        node->repsToDelete << rep;
      }
      else
      {
        node->worker = createTask(node == m_actualPos ? Priority::VERY_HIGHT : Priority::NORMAL);
        node->worker->setInput(node, repList);
        node->worker->setDescription(QString("UPDATE %1 - plane %2").arg(node->position).arg(m_planeIndex));
        node->worker->submit(node->worker);
      }
      node->mutex.unlockInline();
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::updateRepresentationVisibility(bool value)
  {
    ChannelSliceCachedRepresentation *channelRep = qobject_cast<ChannelSliceCachedRepresentation *>(sender());
    SegmentationSliceCachedRepresentation *segRep = qobject_cast<SegmentationSliceCachedRepresentation *>(sender());

    RepresentationSPtr rep = nullptr;
    if (segRep != nullptr)
    {
      for(auto representation: m_representationList)
        if (representation.get() == segRep)
          rep = representation;
    }
    else
    {
      for(auto representation: m_representationList)
        if (representation.get() == channelRep)
          rep = representation;
    }

    if (rep == nullptr)
      return;

    CacheNode *node = m_actualPos;
    for(unsigned int i = 0; i < (2*m_windowWidth)+1; ++i)
    {
      node->mutex.lockInline();
      if(node->representations[rep].actor != nullptr)
      {
        node->representations[rep].actor->SetVisibility(value);
        node->representations[rep].actor->Modified();
      }
      node->mutex.unlockInline();
      node = node->next;
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::changePosition(Plane plane, Nm pos)
  {
    setPosition(pos);
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::printBufferInfo()
  {
    CacheNode *node = m_edgePos->next;
    QString info = "| ";
    unsigned long long memUsed = 0;

    for(unsigned int i = 0; i < (2*m_windowWidth)+1; ++i)
    {
      auto memory = getNodeExtimatedMemoryUsed(node);
      node->mutex.lockInline();
      if (memory == 0)
        info += QString("X");
      else
        info += QString::number(node->position) + QString("(%1)").arg(memory);
      node->mutex.unlockInline();
      info += QString(" | ");

      node = node->next;
      memUsed += memory;
    }

    qDebug() << info << "memory used:" << memUsed << "KB (" << memUsed/1024 << "MB)";
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::clearCache()
  {
    CacheNode *node = m_actualPos;
    for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
    {
      node->mutex.lockInline();
      if (node->worker != nullptr)
        node->worker->abort();
      node->mutex.unlockInline();

      if (node == m_actualPos)
      {
        for(auto rep: node->representations.keys())
          if(node->representations[rep].actor != nullptr)
            m_view->removeActor(node->representations[rep].actor);
      }

      for(auto rep: node->representations.keys())
        node->representations[rep].actor = nullptr;

      node->repsToAdd.clear();
      node->repsToDelete.clear();

      node = m_actualPos->next;
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::initCache()
  {
    auto view = dynamic_cast<View2D *>(m_view);
    auto point = view->crosshairPoint();
    Nm position = point[m_planeIndex];

    m_actualPos = new CacheNode();
    m_actualPos->next = m_actualPos->previous = m_actualPos;
    m_actualPos->position = position;

    CacheNode * node = m_edgePos = m_actualPos;
    for (unsigned int i = 1; i <= m_windowWidth; ++i)
    {
      m_edgePos->next = new CacheNode();
      m_edgePos->next->previous = m_edgePos;
      m_edgePos = m_edgePos->next;
      m_edgePos->position = position + (i * m_windowSpacing);

      node->previous = new CacheNode();
      node->previous->next = node;
      node = node->previous;
      node->position = position - (i * m_windowSpacing);
    }

    node->previous = m_edgePos;
    m_edgePos->next = node;
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::fillCache(Nm position)
  {
    m_actualPos->position = position;

    CacheNode *node = m_edgePos = m_actualPos;
    m_actualPos->worker = createTask(Priority::VERY_HIGHT);
    m_actualPos->worker->setDescription(QString("ALL %1 - plane %2").arg(m_actualPos->position).arg(m_planeIndex));
    m_actualPos->worker->setInput(m_actualPos, m_representationList);
    m_actualPos->worker->submit(m_actualPos->worker);

    for(unsigned int i = 0; i < m_windowWidth; ++i)
    {
      m_edgePos = m_edgePos->next;
      m_edgePos->position = m_edgePos->previous->position + m_windowSpacing;
      m_edgePos->worker = createTask();
      m_edgePos->worker->setDescription(QString("ALL %1 - plane %2").arg(m_edgePos->position).arg(m_planeIndex));
      m_edgePos->worker->setInput(m_edgePos, m_representationList);
      m_edgePos->worker->submit(m_edgePos->worker);

      node = node->previous;
      node->position = node->next->position - m_windowSpacing;
      node->worker = createTask();
      node->worker->setDescription(QString("ALL %1 - plane %2").arg(node->position).arg(m_planeIndex));
      node->worker->setInput(node, m_representationList);
      node->worker->submit(node->worker);
    }

    Q_ASSERT(node->previous = m_edgePos);
    Q_ASSERT(m_edgePos->next = node);
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::renderFrame(CachedSliceRenderer::CacheNode *node)
  {
    auto task = qobject_cast<CachedSliceRendererTask *>(sender());

    // this happens
    if(!task || node->worker == nullptr || node->worker.get() != task)
      return;

    disconnect(task, SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));
    node->worker = nullptr;

    if (!node->repsToDelete.empty())
    {
      for(auto rep: node->repsToDelete)
      {
        node->representations[rep].actor = nullptr;
        node->representations.remove(rep);
      }

      node->repsToDelete.clear();
    }

    if (node == m_actualPos)
    {
      for(auto rep: node->representations.keys())
        if (node->representations[rep].actor != nullptr)
        {
          node->representations[rep].actor->SetVisibility(rep->isVisible());
          node->representations[rep].actor->SetOpacity(rep->opacity());
          node->representations[rep].actor->Modified();
          m_view->addActor(node->representations[rep].actor);
        }

      m_view->updateView();
    }

    if (!node->repsToAdd.empty())
    {
      RepresentationSList repList;
      repList << node->repsToAdd;
      node->worker = createTask();
      node->worker->setInput(node, repList);
      node->worker->setDescription(QString("ADDITIONAL %1 - plane %2").arg(node->position).arg(m_planeIndex));
      node->worker->submit(node->worker);
      node->repsToAdd.clear();
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::setPosition(Nm position)
  {
    if (m_actualPos->position == position)
      return;

    // check if is a complete reposition of the cache, this doesn't count as
    // a cache miss
    if (abs(m_actualPos->position - position) > static_cast<int>(m_windowWidth*m_windowSpacing))
    {
      clearCache();
      fillCache(position);
      return;
    }

    // remove position actors if any
    m_actualPos->mutex.lockInline();
    for(auto rep: m_actualPos->representations.keys())
      if (m_actualPos->representations[rep].actor != nullptr)
        m_view->removeActor(m_actualPos->representations[rep].actor);
    m_actualPos->mutex.unlockInline();

    if (position < m_actualPos->position)
    {
      // left shift
      while (position < m_actualPos->position)
      {
        m_actualPos = m_actualPos->previous;

        m_edgePos->position = m_edgePos->next->position - m_windowSpacing;

        m_edgePos->mutex.lockInline();
        if (m_edgePos->worker != nullptr)
        {
          disconnect(m_edgePos->worker.get(), SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));
          m_edgePos->worker->abort();
        }
        m_edgePos->mutex.unlockInline();
        for(auto rep: m_edgePos->representations.keys())
        {
          m_edgePos->representations[rep].actor = nullptr;
          m_edgePos->representations[rep].time = 0;
        }

        m_edgePos->repsToAdd.clear();
        m_edgePos->repsToDelete.clear();
        m_edgePos->worker = createTask();
        m_edgePos->worker->setInput(m_edgePos, m_representationList);
        m_edgePos->worker->setDescription(QString("ALL %1 - plane %2").arg(m_edgePos->position).arg(m_planeIndex));
        m_edgePos->worker->submit(m_edgePos->worker);

        m_edgePos = m_edgePos->previous;
      }
    }
    else
    {
      // right shift
      while (position > m_actualPos->position)
      {
        m_actualPos = m_actualPos->next;

        m_edgePos = m_edgePos->next;

        m_edgePos->position = m_edgePos->previous->position + m_windowSpacing;

        m_edgePos->mutex.lockInline();
        if (m_edgePos->worker != nullptr)
        {
          disconnect(m_edgePos->worker.get(), SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));
          m_edgePos->worker->abort();
        }
        m_edgePos->mutex.unlockInline();
        for(auto rep: m_edgePos->representations.keys())
        {
          m_edgePos->representations[rep].actor = nullptr;
          m_edgePos->representations[rep].time = 0;
        }

        m_edgePos->repsToAdd.clear();
        m_edgePos->repsToDelete.clear();
        m_edgePos->worker = createTask();
        m_edgePos->worker->setInput(m_edgePos, m_representationList);
        m_edgePos->worker->setDescription(QString("ALL %1 - plane %2").arg(m_edgePos->position).arg(m_planeIndex));
        m_edgePos->worker->submit(m_edgePos->worker);
      }
    }

    // add actors if possible
    m_actualPos->mutex.lockInline();
    for(auto rep: m_actualPos->representations.keys())
      if (m_actualPos->representations[rep].actor != nullptr)
        m_view->addActor(m_actualPos->representations[rep].actor);
    m_actualPos->mutex.unlockInline();
    m_view->updateView();

    m_actualPos->mutex.lockInline();
    if(m_actualPos->worker != nullptr)
    {
      m_actualPos->worker->setPriority(Priority::VERY_HIGHT);
      m_actualPos->mutex.unlockInline();
      setWindowWidth(m_windowWidth + WINDOW_INCREMENT);

      View2D *view = dynamic_cast<View2D *>(m_view);
      Q_ASSERT(view);
      qDebug() << "cache fail in plane" << normalCoordinateIndex(view->plane()) << "window width" << m_windowWidth + WINDOW_INCREMENT;
    }
    else
      m_actualPos->mutex.unlockInline();

    //printBufferInfo();
  }

} // namespace EspINA
