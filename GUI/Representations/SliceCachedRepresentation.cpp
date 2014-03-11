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
#include "SliceCachedRepresentation.h"
#include "SliceCachedRepresentationTask.h"
#include "RepresentationEmptySettings.h"
#include <Core/Analysis/Data/VolumetricDataUtils.h>
#include <GUI/ColorEngines/TransparencySelectionHighlighter.h>

// VTK
#include <vtkMath.h>
#include <vtkAssembly.h>
#include <vtkActor.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkPolyDataMapper.h>
#include <vtkTexture.h>
#include <vtkPlaneSource.h>
#include <vtkImageCanvasSource2D.h>

namespace EspINA
{
  const Representation::Type ChannelSliceCachedRepresentation::TYPE = "Slice (Cached)";

  //-----------------------------------------------------------------------------
  ChannelSliceCachedRepresentation::ChannelSliceCachedRepresentation(DefaultVolumetricDataSPtr data,
                                                                     View2D *view,
                                                                     SchedulerSPtr scheduler)
  : CachedRepresentation(view)
  , m_data{data}
  , m_scheduler{scheduler}
  {
  }
  
  //-----------------------------------------------------------------------------
  RepresentationSPtr ChannelSliceCachedRepresentation::cloneImplementation(View2D* view)
  {
    ChannelSliceCachedRepresentation *representation =  new ChannelSliceCachedRepresentation(m_data, view, m_scheduler);
    representation->setView(view);

    return RepresentationSPtr(representation);
  }

  //-----------------------------------------------------------------------------
  bool ChannelSliceCachedRepresentation::isInside(const NmVector3 &point) const
  {
    return contains(m_data->bounds(), point);
  }

  //-----------------------------------------------------------------------------
  bool ChannelSliceCachedRepresentation::needUpdate(CacheNode *node)
  {
    QMutexLocker lock(&node->mutex);
    if (node->actor != nullptr)
      return node->creationTime != m_data->lastModified();
    else
      return false;
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::updateRepresentation()
  {
    setCrosshairPoint(m_view->crosshairPoint());

    Nm position = m_crosshair[m_planeIndex];

    // actual position range
    Nm min = (static_cast<Nm>(m_actualPos->position) - 0.5 )*m_planeSpacing;
    Nm max = min + m_planeSpacing;

    auto bounds = m_data->bounds();
    bool valid = (bounds[2*m_planeIndex] <= position) && (bounds[2*m_planeIndex+1] > position);

    if (!valid || !isVisible())
    {
      m_actualPos->mutex.lock();
      if (m_actualPos->actor != nullptr)
        m_actualPos->actor->SetVisibility(false);
      else
        if (m_symbolicActor != nullptr)
          m_symbolicActor->SetVisibility(false);
      m_actualPos->mutex.unlock();
      return;
    }

    // update slice if needed
    if (position < min || position > max)
    {
      // move symbolic actor into the correct depth position
      if (m_symbolicActor)
      {
        double pos[3];
        m_symbolicActor->GetPosition(pos);
        pos[m_planeIndex] = m_crosshair[m_planeIndex];
        m_symbolicActor->SetPosition(pos);
        m_symbolicActor->Modified();
      }

      setPosition(vtkMath::Floor((m_crosshair[m_planeIndex]/m_planeSpacing) + 0.5));
    }

    // update actors if needed
    if (needUpdate())
    {
      CacheNode *node = m_actualPos;
      for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
      {
        if (needUpdate(node))
        {
          node->mutex.lock();

          if (node == m_actualPos)
          {
            m_view->removeActor(node->actor);
            if (m_symbolicActor != nullptr)
            {
              m_symbolicActor->GetProperty()->SetOpacity(opacity());
              m_symbolicActor->SetVisibility(isVisible());
              m_view->addActor(m_symbolicActor);
            }
          }

          node->actor = nullptr;
          node->worker = createTask(node);
          if (node->worker != nullptr)
          {
            node->worker->setDescription(QString("Creating actor for slice %1 in plane %2").arg(node->position).arg(m_planeIndex));
            if (node == m_actualPos)
              connect(node->worker.get(), SIGNAL(render(CachedRepresentation::CacheNode *)), this, SLOT(renderFrame(CachedRepresentation::CacheNode *)), Qt::QueuedConnection);
            node->worker->submit(node->worker);
          }
          node->mutex.unlock();
        }
        node = node->next;
      }
    }
  }

  //-----------------------------------------------------------------------------
  CachedRepresentationTaskSPtr ChannelSliceCachedRepresentation::createTask(CacheNode *node, Priority priority)
  {
    Nm posNm = (static_cast<Nm>(node->position) - 0.5) * m_planeSpacing;
    auto bounds = m_data->bounds();
    if (!m_view || (bounds[2*m_planeIndex] > posNm) || (bounds[2*m_planeIndex+1] <= posNm))
      return nullptr;

//    qDebug() << "create channel task con pos" << node->position << "(" << posNm << "," << bounds << ")";

    ChannelSliceCachedRepresentationTask *task = new ChannelSliceCachedRepresentationTask(m_scheduler);
    task->setPriority(priority);
    task->setHidden(true);
    task->setDescription(QString("Creating actor for position %1 in plane %2").arg(node->position).arg(m_planeIndex));
    task->setInput(m_data, posNm, toPlane(m_planeIndex), brightness(), contrast(), color(), NmVector3(), node);

    return CachedRepresentationTaskSPtr{task};
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::setView(View2D *view)
  {
    auto spacing = m_data->spacing();

    m_view = view;
    m_planeIndex = normalCoordinateIndex(view->plane());
    m_planeSpacing = spacing[m_planeIndex];

    vtkSmartPointer<vtkImageCanvasSource2D> canvas = vtkSmartPointer<vtkImageCanvasSource2D>::New();
    canvas->DebugOn();
    canvas->SetScalarTypeToUnsignedChar();
    canvas->SetNumberOfScalarComponents(4);
    canvas->SetExtent(0,31,0,31,0,0);
    canvas->SetDefaultZ(0);
    canvas->SetDrawColor(0,0,0,0);
    canvas->FillBox(0,31,0,31);
    canvas->SetDrawColor(255,255,255,128);
    for (auto i = 0; i < 32; i += 2)
      for (auto j = 0; j < 32; j += 2)
      {
        canvas->DrawPoint(i,j);
        canvas->DrawPoint(i+1,j+1 % 32);
      }
    canvas->Update();

    vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
    texture->DebugOn();
    texture->SetInputData(canvas->GetOutput());
    texture->SetInterpolate(false);
    texture->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_NONE);
    texture->SetEdgeClamp(true);
    texture->SetRepeat(false);

    auto bounds = m_data->bounds();
    vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
    plane->SetOrigin(bounds[0], bounds[2], bounds[4]);

    switch(view->plane())
    {
      case Plane::XY:
        plane->SetPoint1(bounds[0], bounds[3], bounds[4]);
        plane->SetPoint2(bounds[1], bounds[2], bounds[4]);
        break;
      case Plane::XZ:
        plane->SetPoint1(bounds[0], bounds[2], bounds[5]);
        plane->SetPoint2(bounds[1], bounds[2], bounds[4]);
        break;
      case Plane::YZ:
        plane->SetPoint1(bounds[0], bounds[3], bounds[4]);
        plane->SetPoint2(bounds[0], bounds[2], bounds[5]);
        break;
      default:
        Q_ASSERT(false);
        break;
    }
    plane->Update();

    vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    Mapper->SetInputData(plane->GetOutput());
    Mapper->Update();

    m_symbolicActor = vtkSmartPointer<vtkActor>::New();
    m_symbolicActor->SetMapper(Mapper);
    m_symbolicActor->SetTexture(texture);
    m_symbolicActor->GetProperty()->SetColor(color().redF(),color().greenF(), color().blueF());
    m_symbolicActor->SetPickable(false);

    m_view->addActor(m_symbolicActor);
    m_view->updateView();

    setPosition(vtkMath::Floor((bounds[2*m_planeIndex]/spacing[m_planeIndex] + 0.5)));
  }

  //-----------------------------------------------------------------------------
  TransparencySelectionHighlighter *SegmentationSliceCachedRepresentation::s_highlighter = new TransparencySelectionHighlighter();
  const Representation::Type SegmentationSliceCachedRepresentation::TYPE = "Slice (Cached)";

  //-----------------------------------------------------------------------------
  SegmentationSliceCachedRepresentation::SegmentationSliceCachedRepresentation(DefaultVolumetricDataSPtr data, View2D *view, SchedulerSPtr scheduler)
  : CachedRepresentation(view)
  , m_data{data}
  , m_scheduler{scheduler}
  {
  }

  //-----------------------------------------------------------------------------
  QString SegmentationSliceCachedRepresentation::serializeSettings()
  {
    QStringList values;

    values << Representation::serializeSettings();
    values << QString("%1").arg(m_color.alphaF());

    return values.join(";");
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::restoreSettings(QString settings)
  {
    if (!settings.isEmpty())
    {
      QStringList values = settings.split(";");

      double alphaF = values[1].toDouble();

      QColor currentColor = color();
      currentColor.setAlphaF(alphaF);

      Representation::restoreSettings(values[0]);
    }
  }
  
  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::setColor(const QColor& color)
  {
    CachedRepresentation::setColor(color);

    for (auto clone: m_clones)
      clone->setColor(color);
  }
  
  //-----------------------------------------------------------------------------
  QColor SegmentationSliceCachedRepresentation::color() const
  {
    if (!m_clones.isEmpty())
      return m_clones.first()->color();
    else
      return Representation::color();
  }
  
  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::setHighlighted(bool highlighted)
  {
    Representation::setHighlighted(highlighted);

    int position = m_actualPos->position;
    clearCache();
    fillCache(position);

    if (m_symbolicActor != nullptr)
    {
      m_symbolicActor->GetProperty()->SetColor(color().redF(), color().greenF(), color().blueF());
      m_symbolicActor->Modified();
    }

    for (auto clone: m_clones)
      clone->setHighlighted(highlighted);
  }
  
  //-----------------------------------------------------------------------------
  bool SegmentationSliceCachedRepresentation::isInside(const NmVector3& point) const
  {
    return isSegmentationVoxel(m_data, point);
  }

  //-----------------------------------------------------------------------------
  RepresentationSPtr SegmentationSliceCachedRepresentation::cloneImplementation(View2D* view)
  {
    SegmentationSliceCachedRepresentation *representation =  new SegmentationSliceCachedRepresentation(m_data, view, m_scheduler);
    representation->setView(view);

    return RepresentationSPtr(representation);
  }
  
  //-----------------------------------------------------------------------------
  bool SegmentationSliceCachedRepresentation::needUpdate(CacheNode *node)
  {
    QMutexLocker lock(&node->mutex);
    if (node->actor != nullptr)
      return node->creationTime != m_data->lastModified();
    else
      return false;
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::updateRepresentation()
  {
    setCrosshairPoint(m_view->crosshairPoint());

    Nm position = m_crosshair[m_planeIndex];

    // actual position range
    Nm min = (static_cast<Nm>(m_actualPos->position) - 0.5 )*m_planeSpacing;
    Nm max = min + m_planeSpacing;

    auto bounds = m_data->bounds();
    bool valid = (bounds[2*m_planeIndex] <= position) && (bounds[2*m_planeIndex+1] > position);

    if (!valid || !isVisible())
    {
      m_actualPos->mutex.lock();
      if (m_actualPos->actor != nullptr)
        m_actualPos->actor->SetVisibility(false);
      else
        if (m_symbolicActor != nullptr)
          m_symbolicActor->SetVisibility(false);
      m_actualPos->mutex.unlock();
      return;
    }

    // update slice if needed
    if (position < min || position > max)
    {
      // move symbolic actor into the correct depth position
      if (m_symbolicActor)
      {
        double pos[3];
        m_symbolicActor->GetPosition(pos);
        pos[m_planeIndex] = m_crosshair[m_planeIndex] + m_depth[m_planeIndex];
        m_symbolicActor->SetPosition(pos);
        m_symbolicActor->Modified();
      }

      setPosition(vtkMath::Floor((m_crosshair[m_planeIndex]/m_planeSpacing) + 0.5));
    }

    // update actors if needed
    if (needUpdate())
    {
      CacheNode *node = m_actualPos;
      for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
      {
        if (needUpdate(node))
        {
          node->mutex.lock();

          if (node == m_actualPos)
          {
            m_view->removeActor(node->actor);
            if (m_symbolicActor != nullptr)
            {
              m_symbolicActor->GetProperty()->SetOpacity(opacity());
              m_symbolicActor->SetVisibility(isVisible());
              m_view->addActor(m_symbolicActor);
            }
          }

          node->actor = nullptr;
          node->worker = createTask(node);
          if (node->worker != nullptr)
          {
            node->worker->setDescription(QString("Creating actor for slice %1 in plane %2").arg(node->position).arg(m_planeIndex));
            if (node == m_actualPos)
              connect(node->worker.get(), SIGNAL(render(CachedRepresentation::CacheNode *)), this, SLOT(renderFrame(CachedRepresentation::CacheNode *)), Qt::QueuedConnection);
            node->worker->submit(node->worker);
          }
          node->mutex.unlock();
        }
        node = node->next;
      }
    }
  }
  
  //-----------------------------------------------------------------------------
  CachedRepresentationTaskSPtr SegmentationSliceCachedRepresentation::createTask(CacheNode *node, Priority priority)
  {
    Nm posNm = (static_cast<Nm>(node->position) - 0.5) * m_planeSpacing;
    auto bounds = m_data->bounds();

    if (!m_view || (bounds[2*m_planeIndex] > posNm) || (bounds[2*m_planeIndex+1] <= posNm))
      return nullptr;

//    qDebug() << "create seg task con pos" << node->position << "(" << posNm << "," << bounds << ")";

    SegmentationSliceCachedRepresentationTask *task = new SegmentationSliceCachedRepresentationTask(m_scheduler);
    task->setPriority(priority);
    task->setHidden(true);
    task->setDescription(QString("Creating actor for slice %1 in plane %2").arg(node->position).arg(m_planeIndex));
    task->setInput(m_data, posNm, toPlane(m_planeIndex), 0.0, 0.0, color(), m_depth, node);

    return CachedRepresentationTaskSPtr{task};
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::setView(View2D *view)
  {
    auto spacing = m_data->spacing();

    m_view = view;
    m_planeIndex = normalCoordinateIndex(view->plane());
    m_planeSpacing = spacing[m_planeIndex];
    NmVector3 m_depth;
    m_depth[m_planeIndex] = (view->plane() == Plane::XY) ? -view->segmentationDepth() : view->segmentationDepth();

    vtkSmartPointer<vtkImageCanvasSource2D> canvas = vtkSmartPointer<vtkImageCanvasSource2D>::New();
    canvas->DebugOn();
    canvas->SetScalarTypeToUnsignedChar();
    canvas->SetNumberOfScalarComponents(4);
    canvas->SetExtent(0,31,0,31,0,0);
    canvas->SetDefaultZ(0);
    canvas->SetDrawColor(0,0,0,0);
    canvas->FillBox(0,31,0,31);
    canvas->SetDrawColor(255,255,255,128);
    for (auto i = 0; i < 32; i += 2)
      for (auto j = 0; j < 32; j += 2)
      {
        canvas->DrawPoint(i,j);
        canvas->DrawPoint(i+1,j+1 % 32);
      }
    canvas->Update();

    vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
    texture->DebugOn();
    texture->SetInputData(canvas->GetOutput());
    texture->SetInterpolate(false);
    texture->SetBlendingMode(vtkTexture::VTK_TEXTURE_BLENDING_MODE_NONE);
    texture->SetEdgeClamp(true);
    texture->SetRepeat(false);

    auto bounds = m_data->bounds();
    vtkSmartPointer<vtkPlaneSource> plane = vtkSmartPointer<vtkPlaneSource>::New();
    plane->SetOrigin(bounds[0], bounds[2], bounds[4]);

    switch(view->plane())
    {
      case Plane::XY:
        plane->SetPoint1(bounds[0], bounds[3], bounds[4]);
        plane->SetPoint2(bounds[1], bounds[2], bounds[4]);
        break;
      case Plane::XZ:
        plane->SetPoint1(bounds[0], bounds[2], bounds[5]);
        plane->SetPoint2(bounds[1], bounds[2], bounds[4]);
        break;
      case Plane::YZ:
        plane->SetPoint1(bounds[0], bounds[3], bounds[4]);
        plane->SetPoint2(bounds[0], bounds[2], bounds[5]);
        break;
      default:
        Q_ASSERT(false);
        break;
    }
    plane->Update();

    vtkSmartPointer<vtkPolyDataMapper> Mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    Mapper->SetInputData(plane->GetOutput());
    Mapper->Update();

    m_symbolicActor = vtkSmartPointer<vtkActor>::New();
    m_symbolicActor->SetMapper(Mapper);
    m_symbolicActor->SetTexture(texture);
    m_symbolicActor->GetProperty()->SetColor(color().redF(),color().greenF(), color().blueF());
    m_symbolicActor->SetPickable(false);

    m_view->addActor(m_symbolicActor);
    m_view->updateView();

    setPosition(vtkMath::Floor((bounds[2*m_planeIndex]/spacing[m_planeIndex] + 0.5)));
  }

} // namespace EspINA
