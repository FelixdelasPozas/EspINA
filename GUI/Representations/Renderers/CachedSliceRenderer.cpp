/*

 Copyright (C) 2014 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "CachedSliceRenderer.h"
#include "CachedSliceRendererTask.h"
#include <GUI/View/RenderView.h>
#include <GUI/Representations/SliceCachedRepresentation.h>
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>

// VTK
#include <vtkPropPicker.h>
#include <vtkPropCollection.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageMapToColors.h>
#include <vtkImageShiftScale.h>

// Qt
#include <QThread>

QString planeName(int plane)
{
  switch (plane)
  {
    case 0: return QString("Sagittal");
    case 1: return QString("Coronal");
    case 2: return QString("Axial");
    default: Q_ASSERT(false);
      break;
  }
  return QString();
}

namespace ESPINA
{
  const int CachedSliceRenderer::WINDOW_INCREMENT = 5;

  //-----------------------------------------------------------------------------
  CachedSliceRenderer::CachedSliceRenderer(SchedulerSPtr scheduler, QObject *parent)
  : RepresentationRenderer{parent}
  , m_windowWidth         {10}
  , m_maxWindowWidth      {25}
  , m_actualPos           {nullptr}
  , m_edgePos             {nullptr}
  , m_windowSpacing       {-1}
  , m_picker              {nullptr}
  , m_scheduler           {scheduler}
  , m_planeIndex          {-1}
  , m_needCameraReset     {true}
  {
  }

  //-----------------------------------------------------------------------------
  CachedSliceRenderer::~CachedSliceRenderer()
  {
    if (m_view == nullptr)
      return;

    // NOTE: do not remove actors because the vtkRenderer does not exist at this point.
    for (auto rep : m_representationsActors.keys())
      m_representationsActors[rep] = nullptr;

    auto node = m_actualPos;
    for (unsigned int i = 0; i < 2 * m_windowWidth + 1; ++i)
    {
      node->mutex.lockForWrite();
      if (node->worker != nullptr)
      {
        disconnect(node->worker.get(), SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));
        node->worker->abort();
        node->mutex.unlock();
        if (!node->worker->thread()->wait(500))
          node->worker->thread()->terminate();
      }
      else
        node->mutex.unlock();

      // NOTE: thread finished, no need for mutexes beyond this point.
      node->worker = nullptr;

      for (auto rep : node->representations.keys())
        node->representations[rep] = nullptr;

      auto deleteNode = node;
      node = node->next;
      delete deleteNode;
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::setEnable(bool value)
  {
    if (m_enable == value)
      return;

    Renderer::setEnable(value);

    if (m_view != nullptr)
    {
      auto view = dynamic_cast<View2D *>(m_view);
      Q_ASSERT(view);

      if (value)
      {
        connect(view, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(setPosition(Plane, Nm)), Qt::DirectConnection);
        connect(view, SIGNAL(sceneResolutionChanged()), this, SLOT(resolutionChanged()), Qt::DirectConnection);
        setPosition(view->plane(), view->crosshairPoint()[normalCoordinateIndex(view->plane())]);
      }
      else
      {
        disconnect(view, SIGNAL(sliceChanged(Plane, Nm)), this, SLOT(setPosition(Plane, Nm)));
        disconnect(view, SIGNAL(sceneResolutionChanged()), this, SLOT(resolutionChanged()));
      }
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

    initCache();

    m_picker = vtkSmartPointer<vtkPropPicker>::New();
    m_picker->InitializePickList();
    m_picker->PickFromListOn();
  }

  //-----------------------------------------------------------------------------
  CachedRepresentationSList CachedSliceRenderer::validRepresentationsForPosition(const Nm pos) const
  {
    CachedRepresentationSList validRepresentations;

    for (auto rep : m_representationsActors.keys())
      if (rep->existsIn(pos))
        validRepresentations << rep;

    return validRepresentations;
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
      m_representations[item] = list;
    }

    auto cachedRep = std::dynamic_pointer_cast<CachedRepresentation>(rep);
    m_representationsActors.insert(cachedRep, nullptr);
    connect(rep.get(), SIGNAL(update()), this, SLOT(updateRepresentation()), Qt::DirectConnection);
    connect(rep.get(), SIGNAL(changeVisibility()), this, SLOT(updateRepresentationVisibility()), Qt::DirectConnection);
    connect(rep.get(), SIGNAL(changeColor()), this, SLOT(updateRepresentationColor()), Qt::DirectConnection);

    if (rep->type() == ChannelSliceCachedRepresentation::TYPE)
    {
      connect(rep.get(), SIGNAL(changeOpacity()), this, SLOT(changeOpacity()), Qt::DirectConnection);
      connect(rep.get(), SIGNAL(changeContrastAndBrightness()), this, SLOT(changeContrastAndBrightness()), Qt::DirectConnection);
    }

    if (!m_enable)
      return;

    auto node = m_actualPos;
    CachedRepresentationSList repList;
    repList << cachedRep;

    for (unsigned int i = 0; i < 2 * m_windowWidth + 1; ++i, node = node->next)
    {
      if (!cachedRep->existsIn(node->position))
        continue;

      if (node->worker == nullptr)
      {
        node->worker = createTask((node == m_actualPos) ? Priority::VERY_HIGH : Priority::LOW);
        node->worker->setDescription(QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(node->position));
        node->worker->setInput(node, repList);
        node->worker->submit(node->worker);
      }
      else
      {
        node->mutex.lockForWrite();
        node->repsToAdd << cachedRep;
        if (node->repsToDelete.contains(cachedRep))
        {
          node->representations.remove(cachedRep);
          node->repsToDelete.removeOne(cachedRep);
        }
        node->mutex.unlock();
      }
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::removeRepresentation(RepresentationSPtr rep)
  {
    if (!hasRepresentation(rep))
      return;

    for (auto item : m_representations.keys())
      if (m_representations[item].contains(rep))
      {
        m_representations[item].removeOne(rep);
        if (m_representations[item].empty())
          m_representations.remove(item);
      }

    auto cachedRep = std::dynamic_pointer_cast<CachedRepresentation>(rep);
    if (m_representationsActors[cachedRep] != nullptr && m_enable)
      m_view->removeActor(m_representationsActors[cachedRep]);

    m_representationsActors[cachedRep] = nullptr;
    m_representationsActors.remove(cachedRep);
    disconnect(rep.get(), SIGNAL(update()), this, SLOT(updateRepresentation()));
    disconnect(rep.get(), SIGNAL(changeVisibility()), this, SLOT(updateRepresentationVisibility()));

    if (rep->type() == SegmentationSliceCachedRepresentation::TYPE)
      disconnect(rep.get(), SIGNAL(changeColor()), this, SLOT(updateRepresentationColor()));

    auto node = m_actualPos;
    for (unsigned int i = 0; i < 2 * m_windowWidth + 1; ++i, node = node->next)
    {
      node->mutex.lockForWrite();
      node->representations[cachedRep] = nullptr;
      node->representations.remove(cachedRep);

      if (node->worker != nullptr)
      {
        node->repsToDelete << cachedRep;
        if (node->repsToAdd.contains(cachedRep))
          node->repsToAdd.removeOne(cachedRep);
      }
      node->mutex.unlock();
    }

    if (m_representationsActors.keys().size() == 0)
      m_needCameraReset = true;
  }

  //-----------------------------------------------------------------------------
  bool CachedSliceRenderer::hasRepresentation(RepresentationSPtr rep) const
  {
    if (managesRepresentation(rep->type()))
    {
      auto cachedRep = std::dynamic_pointer_cast<CachedRepresentation>(rep);
      return m_representationsActors.keys().contains(cachedRep);
    }

    return false;
  }

  //-----------------------------------------------------------------------------
  bool CachedSliceRenderer::managesRepresentation(const QString &repType) const
  {
    return ((repType == ChannelSliceCachedRepresentation::TYPE) || (repType == SegmentationSliceCachedRepresentation::TYPE));
  }

  //-----------------------------------------------------------------------------
  unsigned int CachedSliceRenderer::numberOfvtkActors() const
  {
    int numActors = 0;

    if (m_enable)
      for (auto rep : m_representationsActors.keys())
        if (m_representationsActors[rep] != nullptr)
          ++numActors;

    return numActors;
  }

  //-----------------------------------------------------------------------------
  ViewItemAdapterList CachedSliceRenderer::pick(int x, int y, Nm z, vtkSmartPointer<vtkRenderer> renderer, RenderableItems itemType, bool repeat)
  {
    ViewItemAdapterList selection;

    if (m_representationsActors.keys().size() == 0 || !m_enable)
      return selection;

    if (!renderer || !renderer.GetPointer() || (!itemType.testFlag(RenderableType::CHANNEL) && !itemType.testFlag(RenderableType::SEGMENTATION)))
      return selection;

    auto realZ = m_view->crosshairPoint()[m_planeIndex];
    auto repList = validRepresentationsForPosition(realZ);

    for (auto rep : repList)
      if (m_representationsActors[rep] != nullptr)
        m_picker->AddPickList(m_representationsActors[rep]);

    while (m_picker->Pick(x, y, 0, renderer))
    {
      double point[3];
      m_picker->GetPickPosition(point);
      m_lastValidPickPosition = NmVector3 { point[0], point[1], point[2] };
      point[m_planeIndex] = realZ;

      vtkProp *pickedProp = m_picker->GetViewProp();
      Q_ASSERT(pickedProp);

      m_picker->DeletePickList(pickedProp);

      NmVector3 vecPoint { point[0], point[1], point[2] };
      RepresentationSPtr pickedRepresentation;

      for (auto rep : repList)
        if (pickedProp == dynamic_cast<vtkProp *>(m_representationsActors[rep].GetPointer()))
          pickedRepresentation = rep;

      for (auto item : m_representations.keys())
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
              m_picker->GetPickList()->RemoveAllItems();

              return selection;
            }

            break;
          }
        }
      }
    }

    m_picker->GetPickList()->RemoveAllItems();
    return selection;
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::resolutionChanged()
  {
    auto spacing = m_view->sceneResolution()[m_planeIndex];

    if (m_windowSpacing != spacing)
    {
      m_windowSpacing = spacing;

      // adjust to actual position;
      fillCache(m_view->crosshairPoint()[m_planeIndex]);
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::hide()
  {
    for (auto rep : m_representationsActors.keys())
      if (m_representationsActors[rep] != nullptr)
        m_view->removeActor(m_representationsActors[rep]);

    emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::show()
  {
    for (auto rep : m_representationsActors.keys())
      if (m_representationsActors[rep] != nullptr)
        m_view->addActor(m_representationsActors[rep]);

    emit renderRequested();
  }

  //-----------------------------------------------------------------------------
  CachedSliceRendererTaskSPtr CachedSliceRenderer::createTask(Priority priority)
  {
    auto task = new CachedSliceRendererTask(m_scheduler);
    task->setPriority(priority);
    connect(task, SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)), Qt::BlockingQueuedConnection);

    return CachedSliceRendererTaskSPtr { task };
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

    CachedRepresentationSList reps = m_representationsActors.keys();
    auto node = m_edgePos->next;

    for (int i = 0; i < diff; ++i)
    {
      if (smaller)
      {
        m_edgePos->mutex.lockForWrite();
        if (m_edgePos->worker != nullptr)
        {
          disconnect(m_edgePos->worker.get(), SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));
          m_edgePos->worker->abort();
          m_edgePos->mutex.unlock();
          if (!m_edgePos->worker->thread()->wait(500))
            m_edgePos->worker->thread()->terminate();
        }
        else
          m_edgePos->mutex.unlock();

        m_edgePos->worker = nullptr;

        for (auto rep : m_edgePos->representations.keys())
          m_edgePos->representations[rep] = nullptr;

        node->mutex.lockForWrite();
        if (node->worker != nullptr)
        {
          disconnect(node->worker.get(), SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));
          node->worker->abort();
          node->mutex.unlock();
          if (!node->worker->thread()->wait(500))
            node->worker->thread()->terminate();
        }
        else
          node->mutex.unlock();

        node->worker = nullptr;

        for (auto rep : node->representations.keys())
          node->representations[rep] = nullptr;

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
        m_edgePos->worker->setInput(m_edgePos, reps);
        m_edgePos->worker->setDescription(QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(m_edgePos->position));
        m_edgePos->worker->submit(m_edgePos->worker);

        node->previous = new CacheNode();
        node->previous->next = node;
        node = node->previous;
        node->position = node->next->position - m_windowSpacing;

        node->worker = createTask();
        node->worker->setInput(node, reps);
        node->worker->setDescription(QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(node->position));
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

    node->mutex.lockForRead();
    for (auto rep : node->representations.keys())
      if (node->representations[rep] != nullptr)
        size += node->representations[rep]->GetMapper()->GetInput()->GetActualMemorySize();
    node->mutex.unlock();

    return size;
  }

  //-----------------------------------------------------------------------------
  unsigned long long CachedSliceRenderer::getEstimatedMemoryUsed()
  {
    unsigned long long size = 0;

    auto node = m_actualPos;
    for (unsigned int i = 0; i < ((2 * m_windowWidth) + 1); ++i)
      size += getNodeExtimatedMemoryUsed(node);

    return size;
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::updateRepresentation()
  {
    auto channelRep = qobject_cast<ChannelSliceCachedRepresentationPtr>(sender());
    auto segRep = qobject_cast<SegmentationSliceCachedRepresentationPtr>(sender());

    if ((channelRep == nullptr && segRep == nullptr) || !m_enable)
      return;

    CachedRepresentationSPtr rep = nullptr;
    auto repList = m_representationsActors.keys();

    for (auto representation : repList)
      if (representation.get() == channelRep || representation.get() == segRep)
        rep = representation;

    if (rep == nullptr)
      return;

    CachedRepresentationSList segList;
    segList << rep;

    auto node = m_actualPos;
    for (unsigned int i = 0; i < (2 * m_windowWidth) + 1; ++i, node = node->next)
    {
      node->mutex.lockForWrite();
      node->representations[rep] = nullptr;

      if (rep->existsIn(node->position))
      {
        if (node->worker != nullptr)
        {
          node->repsToAdd << rep;
          node->repsToDelete << rep;
        }
        else
        {
          node->worker = createTask(node == m_actualPos ? Priority::VERY_HIGH : Priority::LOW);
          node->worker->setInput(node, segList);
          node->worker->setDescription(QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(node->position));
          node->worker->submit(node->worker);
        }
      }
      node->mutex.unlock();
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::updateRepresentationVisibility()
  {
    auto channelRep = qobject_cast<ChannelSliceCachedRepresentationPtr>(sender());
    auto segRep = qobject_cast<SegmentationSliceCachedRepresentationPtr>(sender());

    if ((channelRep == nullptr && segRep == nullptr) || !m_enable)
      return;

    CachedRepresentationSPtr rep = nullptr;
    auto repList = m_representationsActors.keys();

    for (auto representation : repList)
      if (representation.get() == channelRep || representation.get() == segRep)
        rep = representation;

    if (rep == nullptr)
      return;

    if (m_representationsActors[rep] != nullptr)
    {
      m_representationsActors[rep]->SetVisibility(rep->isVisible());
      m_representationsActors[rep]->Update();
    }

    auto node = m_actualPos;
    for (unsigned int i = 0; i < (2 * m_windowWidth) + 1; ++i, node = node->next)
    {
      node->mutex.lockForWrite();
      if (node->representations[rep] != nullptr)
      {
        node->representations[rep]->SetVisibility(rep->isVisible());
        node->representations[rep]->Update();
      }
      node->mutex.unlock();
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::updateRepresentationColor()
  {
    auto segRep = qobject_cast<SegmentationSliceCachedRepresentation *>(sender());
    auto channelRep = qobject_cast<ChannelSliceCachedRepresentationPtr>(sender());

    if ((channelRep == nullptr && segRep == nullptr) || !m_enable)
      return;

    CachedRepresentationSPtr rep = nullptr;
    auto repList = m_representationsActors.keys();
    for (auto representation : repList)
      if ((representation.get() == segRep) || (representation.get() == channelRep))
        rep = representation;

    if (rep == nullptr)
      return;

    QColor color;
    vtkSmartPointer<vtkLookupTable> lut = nullptr;

    if(rep->type() == SegmentationSliceCachedRepresentation::TYPE)
    {
      color = segRep->color();
      lut = SegmentationSliceCachedRepresentation::s_highlighter->lut(color, segRep->isHighlighted());
    }
    else
    {
      color = channelRep->color();
      lut = vtkSmartPointer<vtkLookupTable>::New();
      lut->Allocate();
      lut->SetTableRange(0,255);
      lut->SetHueRange(color.hueF(), color.hueF());
      lut->SetSaturationRange(0.0, color.saturationF());
      lut->SetValueRange(0.0, 1.0);
      lut->SetAlphaRange(1.0,1.0);
      lut->SetNumberOfColors(256);
      lut->SetRampToLinear();
      lut->Build();
    }

      if (m_representationsActors[rep] != nullptr)
    {
      auto imageMapToColors = vtkImageMapToColors::SafeDownCast(m_representationsActors[rep]->GetMapper()->GetInputAlgorithm(0, 0));
      imageMapToColors->SetLookupTable(lut);
      imageMapToColors->Update();
      m_representationsActors[rep]->GetMapper()->Update();
      m_representationsActors[rep]->Update();
    }

    auto node = m_actualPos;
    for (unsigned int i = 0; i < (2 * m_windowWidth) + 1; ++i, node = node->next)
    {
      if (rep->existsIn(node->position))
      {
        node->mutex.lockForWrite();
        if (node->representations[rep] != nullptr)
        {
          auto imageMapToColors = vtkImageMapToColors::SafeDownCast(node->representations[rep]->GetMapper()->GetInputAlgorithm(0, 0));
          imageMapToColors->SetLookupTable(lut);
          imageMapToColors->Update();
          node->representations[rep]->GetMapper()->Update();
          node->representations[rep]->Update();
        }
        node->mutex.unlock();
      }
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::changeContrastAndBrightness()
  {
    auto channelRep = qobject_cast<ChannelSliceCachedRepresentation *>(sender());

    CachedRepresentationSPtr rep = nullptr;
    auto repList = m_representationsActors.keys();
    for (auto representation : repList)
      if (representation.get() == channelRep)
        rep = representation;

    if (rep == nullptr)
      return;

    if (m_representationsActors[rep] != nullptr)
    {
      auto imageMapToColors = vtkImageMapToColors::SafeDownCast(m_representationsActors[rep]->GetMapper()->GetInputAlgorithm(0, 0));
      auto shiftScaleFilter = vtkImageShiftScale::SafeDownCast(imageMapToColors->GetInputAlgorithm());
      shiftScaleFilter->SetShift(static_cast<int>(channelRep->brightness()*255));
      shiftScaleFilter->SetScale(channelRep->contrast());
      shiftScaleFilter->Update();

      imageMapToColors->Update();
      m_representationsActors[rep]->GetMapper()->Update();
      m_representationsActors[rep]->Update();
    }

    auto node = m_actualPos;
    for (unsigned int i = 0; i < (2 * m_windowWidth) + 1; ++i, node = node->next)
    {
      if (rep->existsIn(node->position))
      {
        node->mutex.lockForWrite();
        if (node->representations[rep] != nullptr)
        {
          auto imageMapToColors = vtkImageMapToColors::SafeDownCast(node->representations[rep]->GetMapper()->GetInputAlgorithm(0, 0));
          auto shiftScaleFilter = vtkImageShiftScale::SafeDownCast(imageMapToColors->GetInputAlgorithm());
          shiftScaleFilter->SetShift(static_cast<int>(channelRep->brightness()*255));
          shiftScaleFilter->SetScale(channelRep->contrast());
          shiftScaleFilter->Update();

          imageMapToColors->Update();
          node->representations[rep]->GetMapper()->Update();
          node->representations[rep]->Update();
        }
        node->mutex.unlock();
      }
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::changeOpacity()
  {
    auto channelRep = qobject_cast<ChannelSliceCachedRepresentation *>(sender());

    CachedRepresentationSPtr rep = nullptr;
    auto repList = m_representationsActors.keys();
    for (auto representation : repList)
      if (representation.get() == channelRep)
        rep = representation;

    if (rep == nullptr)
      return;

    if (m_representationsActors[rep] != nullptr)
    {
      m_representationsActors[rep]->SetOpacity(channelRep->opacity());
      m_representationsActors[rep]->Update();
    }

    auto node = m_actualPos;
    for (unsigned int i = 0; i < (2 * m_windowWidth) + 1; ++i, node = node->next)
    {
      if (rep->existsIn(node->position))
      {
        node->mutex.lockForWrite();
        if (node->representations[rep] != nullptr)
        {
          node->representations[rep]->SetOpacity(channelRep->opacity());
          node->representations[rep]->Update();
        }
        node->mutex.unlock();
      }
    }
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::printBufferInfo()
  {
    auto node = m_edgePos->next;
    QString info = "| ";
    unsigned long long memUsed = 0;

    for (unsigned int i = 0; i < (2 * m_windowWidth) + 1; ++i)
    {
      auto memory = getNodeExtimatedMemoryUsed(node);

      if (memory == 0)
        info += QString("X");
      else
        info += QString::number(node->position); // + QString("(%1)").arg(memory);

      info += QString(" | ");

      node = node->next;
      memUsed += memory;
    }

    qDebug() << info << "memory used:" << memUsed << "bytes (" << memUsed / 1024 << "MB - " << memUsed / 1024 / 1024 << "GB)";
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::initCache()
  {
    auto view = dynamic_cast<View2D *>(m_view);
    auto point = view->crosshairPoint();
    auto position = point[m_planeIndex];

    m_actualPos = new CacheNode();
    m_actualPos->next = m_actualPos->previous = m_actualPos;
    m_actualPos->position = position;

    auto node = m_edgePos = m_actualPos;
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
    auto repList = m_representationsActors.keys();

    for (auto rep : repList)
    {
      if (m_representationsActors[rep] != nullptr)
        m_view->removeActor(m_representationsActors[rep]);

      m_representationsActors[rep] = nullptr;
    }

    m_actualPos->mutex.lockForWrite();
    m_actualPos->position = position;
    m_actualPos->repsToAdd.clear();
    m_actualPos->repsToDelete.clear();

    for (auto rep : m_actualPos->representations.keys())
      m_actualPos->representations[rep] = nullptr;

    Priority priority = Priority::VERY_HIGH;
    auto reps = m_representationsActors.keys();

    if (m_actualPos->worker != nullptr)
    {
      m_actualPos->repsToAdd = reps;
      m_actualPos->restart = true;
      m_actualPos->worker->setPriority(priority);
    }
    else
    {
      m_actualPos->worker = createTask(priority);
      m_actualPos->worker->setDescription(QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(m_actualPos->position));
      m_actualPos->worker->setInput(m_actualPos, reps);
      m_actualPos->worker->submit(m_actualPos->worker);
    }

    m_actualPos->mutex.unlock();

    auto node = m_edgePos = m_actualPos;
    for (unsigned int i = 0; i < m_windowWidth; ++i)
    {
      m_edgePos = m_edgePos->next;
      m_edgePos->mutex.lockForWrite();
      m_edgePos->position = m_edgePos->previous->position + m_windowSpacing;
      m_edgePos->repsToAdd.clear();
      m_edgePos->repsToDelete.clear();

      for (auto rep : m_edgePos->representations.keys())
        m_edgePos->representations[rep] = nullptr;

      priority = ((m_edgePos->position - position) < (5 * m_windowSpacing)) ? Priority::HIGH : Priority::LOW;
      if (m_edgePos->worker != nullptr)
      {
        m_edgePos->repsToAdd = reps;
        m_edgePos->restart = true;
        m_edgePos->worker->setPriority(priority);
      }
      else
      {
        m_edgePos->worker = createTask(priority);
        m_edgePos->worker->setDescription(
            QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(m_edgePos->position));
        m_edgePos->worker->setInput(m_edgePos, reps);
        m_edgePos->worker->submit(m_edgePos->worker);
      }

      m_edgePos->mutex.unlock();

      node = node->previous;
      node->mutex.lockForWrite();
      node->position = node->next->position - m_windowSpacing;
      node->repsToAdd.clear();
      node->repsToDelete.clear();

      for (auto rep : node->representations.keys())
        node->representations[rep] = nullptr;

      priority = ((node->position - position) < (5 * m_windowSpacing)) ? Priority::HIGH : Priority::LOW;
      if (node->worker != nullptr)
      {
        node->repsToAdd = reps;
        node->restart = true;
        node->worker->setPriority(priority);
      }
      else
      {
        node->worker = createTask(priority);
        node->worker->setDescription(QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(node->position));
        node->worker->setInput(node, reps);
        node->worker->submit(node->worker);
      }

      node->mutex.unlock();
    }

    Q_ASSERT(node->previous = m_edgePos);
    Q_ASSERT(m_edgePos->next = node);
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::renderFrame(CachedSliceRenderer::CacheNode *node)
  {
    auto task = qobject_cast<CachedSliceRendererTask *>(sender());

    if (!task)
      return;
    else
      disconnect(task, SIGNAL(ready(CachedSliceRenderer::CacheNode *)), this, SLOT(renderFrame(CachedSliceRenderer::CacheNode *)));

    node->mutex.lockForWrite();
    node->worker = nullptr;
    if (!node->repsToDelete.empty())
    {
      for (auto rep : node->repsToDelete)
      {
        node->representations[rep] = nullptr;
        node->representations.remove(rep);
      }

      node->repsToDelete.clear();
    }

    if (node->restart)
    {
      node->restart = false;
      auto repList = validRepresentationsForPosition(node->position);
      if (!repList.empty())
      {
        node->worker = createTask((node == m_actualPos) ? Priority::VERY_HIGH : Priority::LOW);
        node->worker->setInput(node, repList);
        node->worker->setDescription(QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(node->position));
        node->worker->submit(node->worker);
      }
    }
    else
    {
      if (node == m_actualPos)
      {
        bool update = false;
        bool resetCamera = false;
        for (auto rep : node->representations.keys())
        {
          if (node->representations[rep] != m_representationsActors[rep])
          {
            update = true;
            if (rep->type() == ChannelSliceCachedRepresentation::TYPE && m_needCameraReset)
              resetCamera = true;

            if (m_representationsActors[rep] != nullptr)
            {
              if (m_enable)
                m_view->removeActor(m_representationsActors[rep]);

              m_representationsActors[rep] = nullptr;
            }
            m_representationsActors[rep] = node->representations[rep];

            if (m_enable && node->representations[rep] != nullptr)
            {
              node->representations[rep]->SetVisibility(rep->isVisible());
              m_view->addActor(node->representations[rep]);
            }
          }
        }

        if ((resetCamera || update) && m_enable)
        {
          if (resetCamera)
          {
            m_needCameraReset = false;
            m_view->resetCamera();
          }

          m_view->updateView();
        }
      }

      if (!node->repsToAdd.empty())
      {
        node->worker = createTask((node == m_actualPos) ? Priority::VERY_HIGH : Priority::LOW);
        node->worker->setInput(node, node->repsToAdd);
        node->worker->setDescription(QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(node->position));
        node->worker->submit(node->worker);
      }
    }
    node->repsToAdd.clear();
    node->mutex.unlock();
  }

  //-----------------------------------------------------------------------------
  void CachedSliceRenderer::setPosition(Plane plane, Nm position)
  {
    if (m_actualPos->position == position || (m_representationsActors.keys().size() == 0))
      return;

    auto jump = std::fabs(m_actualPos->position - position);
    // this is necessary as setPosition() is called during file loading with incorrect
    // values and that makes the cache positions invalid, if an invalid position is detected
    // the cache forces a refill.
    auto needReadjustment = (jump < static_cast<double>(m_windowSpacing * 0.999)) && (jump > static_cast<double>(m_windowSpacing * 0.001));
    auto needRefill = jump > static_cast<double>(m_windowWidth * m_windowSpacing);

    // check if is a complete reposition of the cache
    if (needRefill || needReadjustment)
    {
      fillCache(position);
      return;
    }

    CachedRepresentationSList reps = m_representationsActors.keys();

    for (auto rep : reps)
    {
      if (m_representationsActors[rep] != nullptr)
        m_view->removeActor(m_representationsActors[rep]);

      m_representationsActors[rep] = nullptr;
    }

    if (m_actualPos->worker != nullptr)
      m_actualPos->worker->setPriority(Priority::LOW);

    if (position < m_actualPos->position)
    {
      // left shift
      while (position < m_actualPos->position)
      {
        m_actualPos = m_actualPos->previous;

        m_edgePos->position = m_edgePos->next->position - m_windowSpacing;
        m_edgePos->mutex.lockForWrite();
        for (auto rep : m_edgePos->representations.keys())
          m_edgePos->representations[rep] = nullptr;

        m_edgePos->repsToAdd.clear();
        m_edgePos->repsToDelete.clear();

        if (m_edgePos->worker != nullptr)
        {
          m_edgePos->repsToAdd = reps;
          m_edgePos->restart = true;
        }
        else
        {
          m_edgePos->worker = createTask();
          m_edgePos->worker->setInput(m_edgePos, reps);
          m_edgePos->worker->setDescription(QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(m_edgePos->position));
          m_edgePos->worker->submit(m_edgePos->worker);
        }

        m_edgePos->mutex.unlock();

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
        m_edgePos->mutex.lockForWrite();
        for (auto rep : m_edgePos->representations.keys())
          m_edgePos->representations[rep] = nullptr;

        m_edgePos->repsToAdd.clear();
        m_edgePos->repsToDelete.clear();

        if (m_edgePos->worker != nullptr)
        {
          m_edgePos->repsToAdd = reps;
          m_edgePos->restart = true;
        }
        else
        {
          m_edgePos->worker = createTask();
          m_edgePos->worker->setInput(m_edgePos, reps);
          m_edgePos->worker->setDescription(
              QString("Cache %1 Pos %2").arg(planeName(m_planeIndex)).arg(m_edgePos->position));
          m_edgePos->worker->submit(m_edgePos->worker);
        }

        m_edgePos->mutex.unlock();
      }
    }

    // add actors if possible
    m_actualPos->mutex.lockForRead();
    int numActors = 0;
    for (auto rep : m_actualPos->representations.keys())
    {
      if (m_actualPos->representations[rep] != nullptr)
      {
        m_actualPos->representations[rep]->SetVisibility(rep->isVisible());
        //m_actualPos->representations[rep]->Update();
        m_view->addActor(m_actualPos->representations[rep]);
        ++numActors;
      }

      m_representationsActors[rep] = m_actualPos->representations[rep];
    }
    m_actualPos->mutex.unlock();

    // avoid unnecessary renders if the actors haven't been computed yet.
    if (numActors != 0)
    {
      if (m_needCameraReset)
      {
        m_needCameraReset = false;
        m_view->resetCamera();
      }
    }

    if (m_actualPos->worker != nullptr)
    {
      m_actualPos->worker->setPriority(Priority::VERY_HIGH);

      // TODO: evitar el fallo de cache inicial
      setWindowWidth(m_windowWidth + WINDOW_INCREMENT);
    }

//    printBufferInfo();
  }

} // namespace ESPINA

