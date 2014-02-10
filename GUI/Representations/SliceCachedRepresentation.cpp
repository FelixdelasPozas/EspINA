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
  bool ChannelSliceCachedRepresentation::hasActor(vtkProp *actor) const
  {
    if (m_actor == nullptr || m_view == nullptr)
      return false;

    vtkPropCollection *props = nullptr;
    m_actor->GetActors(props);
    if (props != nullptr)
      for (int i = 0; i < props->GetNumberOfItems(); ++i)
          if (actor == props->GetNextProp())
            return true;

    return false;
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
  void ChannelSliceCachedRepresentation::setColor(const QColor &color)
  {
    Representation::setColor(color);

    int position = m_actualPos->position;
    clearCache();
    fillCache(position);
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::setBrightness(double brightness)
  {
    Representation::setBrightness(brightness);

    int position = m_actualPos->position;
    clearCache();
    fillCache(position);
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::setContrast(double contrast)
  {
    Representation::setContrast(contrast);

    int position = m_actualPos->position;
    clearCache();
    fillCache(position);
  }
  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::setOpacity(double opacity)
  {
    Representation::setOpacity(opacity);

    // Opacity of actors is handled in CachedRepresentation class, no need to
    // recalculate the actors.
  }

  bool ChannelSliceCachedRepresentation::needUpdate()
  {
    return (m_actualPos->creationTime != m_data->lastModified());
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::updateRepresentation()
  {
    setCrosshairPoint(m_view->crosshairPoint());

    Nm position = m_crosshair[m_planeIndex];

    // actual position range
    Nm min = (static_cast<Nm>(m_actualPos->position) - 0.5 )*m_planeSpacing;
    Nm max = min + m_planeSpacing;

    bool valid = (m_min <= position) && (m_max > position);

    // update slice if needed
    if (position < min || position > max)
    {
      if (!valid)
      {
        m_actor->SetVisibility(false);
        return;
      }

      // move symbolic actor into the correct depth position
      double pos[3];
      m_symbolicActor->GetPosition(pos);
      pos[m_planeIndex] = m_crosshair[m_planeIndex];
      m_symbolicActor->SetPosition(pos);
      m_symbolicActor->Modified();

      setPosition(vtkMath::Floor((m_crosshair[m_planeIndex]/m_planeSpacing) + 0.5));
    }

    // update actors if needed
    if (needUpdate())
    {
      CacheNode *node = m_actualPos;
      for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
      {
        if ((node->actor != nullptr) && (node->creationTime != m_data->lastModified()))
        {
          if (node == m_actualPos)
          {
            m_actor->RemovePart(node->actor);
            if (m_symbolicActor != nullptr)
            {
              m_symbolicActor->GetProperty()->SetOpacity(opacity());
              m_actor->AddPart(m_symbolicActor);
            }
            m_actor->Modified();
          }
          node->actor = nullptr;
          node->worker = createTask(node->position);
          if (node->worker)
          {
            node->worker->setDescription(QString("Creating actor for slice %1 in plane %2").arg(node->position).arg(m_planeIndex));
            connect(node->worker, SIGNAL(finished()), this, SLOT(addActor()), Qt::QueuedConnection);
            node->worker->submit();
          }
        }
        node = node->next;
      }
    }

    m_actor->SetVisibility(valid && isVisible());
  }

  //-----------------------------------------------------------------------------
  RepresentationSettings *ChannelSliceCachedRepresentation::settingsWidget()
  {
    return new RepresentationEmptySettings();
  }

  //-----------------------------------------------------------------------------
  QList<vtkProp*> ChannelSliceCachedRepresentation::getActors()
  {
    QList<vtkProp *> list;
    list << m_actor;

    return list;
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::updateVisibility(bool visible)
  {
    if (m_actor != nullptr)
      m_actor->SetVisibility(visible);
  }

  //-----------------------------------------------------------------------------
  CachedRepresentationTask *ChannelSliceCachedRepresentation::createTask(int position, Priority priority)
  {
    Nm posNm = (static_cast<Nm>(position) - 0.5) * m_planeSpacing;
    if (!m_view || (m_min > posNm) || (m_max <= posNm))
      return nullptr;

    ChannelSliceCachedRepresentationTask *task = new ChannelSliceCachedRepresentationTask(m_scheduler);
    task->setPriority(priority);
    task->setHidden(true);
    task->setDescription(QString("Creating actor for position %1 in plane %2").arg(position).arg(m_planeIndex));
    task->setInput(m_data, posNm, toPlane(m_planeIndex), brightness(), contrast(), color());

    return task;
  }

  //-----------------------------------------------------------------------------
  void ChannelSliceCachedRepresentation::setView(View2D *view)
  {
    auto spacing = m_data->spacing();
    auto bounds = m_data->bounds();

    m_view = view;
    m_planeIndex = normalCoordinateIndex(view->plane());
    m_planeSpacing = spacing[m_planeIndex];
    m_min = bounds[2*m_planeIndex];
    m_max = bounds[2*m_planeIndex +1];

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

    m_actor = vtkSmartPointer<vtkAssembly>::New();
    m_actor->AddPart(m_symbolicActor);
    m_actor->Modified();

    m_view->addActor(m_actor);
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
  RepresentationSettings* SegmentationSliceCachedRepresentation::settingsWidget()
  {
    return new RepresentationEmptySettings();
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
    Representation::setColor(color);

    int position = m_actualPos->position;
    clearCache();
    fillCache(position);

    if (m_symbolicActor != nullptr)
    {
      m_symbolicActor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
      m_symbolicActor->Modified();
    }

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
    Bounds bounds{ '[', point[0], point[0], point[1], point[1], point[2], point[2], ']'};

    itkVolumeType::Pointer voxel = m_data->itkImage(bounds);

    return (SEG_VOXEL_VALUE == *(static_cast<unsigned char*>(voxel->GetBufferPointer())));
  }

  //-----------------------------------------------------------------------------
  bool SegmentationSliceCachedRepresentation::hasActor(vtkProp *actor) const
  {
    if (m_actor == nullptr || m_view == nullptr)
      return false;

    vtkPropCollection *props = nullptr;
    m_actor->GetActors(props);
    if (props != nullptr)
      for (int i = 0; i < props->GetNumberOfItems(); ++i)
          if (actor == props->GetNextProp())
            return true;

    return false;
  }

  //-----------------------------------------------------------------------------
  RepresentationSPtr SegmentationSliceCachedRepresentation::cloneImplementation(View2D* view)
  {
    SegmentationSliceCachedRepresentation *representation =  new SegmentationSliceCachedRepresentation(m_data, view, m_scheduler);
    representation->setView(view);

    return RepresentationSPtr(representation);
  }
  
  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::updateRepresentation()
  {
    setCrosshairPoint(m_view->crosshairPoint());

    Nm position = m_crosshair[m_planeIndex];

    // actual position range
    Nm min = (static_cast<Nm>(m_actualPos->position) - 0.5 )*m_planeSpacing;
    Nm max = min + m_planeSpacing;

    bool valid = (m_min <= position) && (m_max > position);

    // update slice if needed
    if (position < min || position > max)
    {
      if (!valid)
      {
        m_actor->SetVisibility(false);
        return;
      }

      // move symbolic actor into the correct depth position
      double pos[3];
      m_symbolicActor->GetPosition(pos);
      pos[m_planeIndex] = m_crosshair[m_planeIndex];
      m_symbolicActor->SetPosition(pos);
      m_symbolicActor->Modified();

      setPosition(vtkMath::Floor((m_crosshair[m_planeIndex]/m_planeSpacing) + 0.5));
    }

    // update actors if needed
    if (needUpdate())
    {
      CacheNode *node = m_actualPos;
      for(unsigned int i = 0; i < ((2*m_windowWidth) + 1); ++i)
      {
        if ((node->actor != nullptr) && (node->creationTime != m_data->lastModified()))
        {
          if (node == m_actualPos)
          {
            m_actor->RemovePart(node->actor);
            if (m_symbolicActor != nullptr)
            {
              m_symbolicActor->GetProperty()->SetOpacity(opacity());
              m_actor->AddPart(m_symbolicActor);
            }
            m_actor->Modified();
          }
          node->actor = nullptr;
          node->worker = createTask(node->position);
          if (node->worker)
          {
            node->worker->setDescription(QString("Creating actor for slice %1 in plane %2").arg(node->position).arg(m_planeIndex));
            connect(node->worker, SIGNAL(finished()), this, SLOT(addActor()), Qt::QueuedConnection);
            node->worker->submit();
          }
        }
        node = node->next;
      }
    }

    m_actor->SetVisibility(valid && isVisible());
  }
  
  //-----------------------------------------------------------------------------
  QList<vtkProp*> SegmentationSliceCachedRepresentation::getActors()
  {
    QList<vtkProp *> list;
    list << m_actor;

    return list;
  }
  
  //-----------------------------------------------------------------------------
  bool SegmentationSliceCachedRepresentation::needUpdate()
  {
    return m_actualPos->creationTime != m_data->lastModified();
  }
  
  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::updateVisibility(bool visible)
  {
    if (m_actor != nullptr)
      m_actor->SetVisibility(visible);
  }

  //-----------------------------------------------------------------------------
  CachedRepresentationTask *SegmentationSliceCachedRepresentation::createTask(int position, Priority priority)
  {
    Nm posNm = (static_cast<Nm>(position) - 0.5) * m_planeSpacing;

    if (!m_view || (m_min > posNm) || (m_max <= posNm))
      return nullptr;

    SegmentationSliceCachedRepresentationTask *task = new SegmentationSliceCachedRepresentationTask(m_scheduler);
    task->setPriority(priority);
    task->setHidden(true);
    task->setDescription(QString("Creating actor for slice %1 in plane %2").arg(position).arg(m_planeIndex));
    task->setInput(m_data, posNm, toPlane(m_planeIndex), 0.0, 0.0, color());

    return task;
  }

  //-----------------------------------------------------------------------------
  void SegmentationSliceCachedRepresentation::setView(View2D *view)
  {
    auto spacing = m_data->spacing();
    auto bounds = m_data->bounds();

    m_view = view;
    m_planeIndex = normalCoordinateIndex(view->plane());
    m_planeSpacing = spacing[m_planeIndex];
    m_min = bounds[2*m_planeIndex];
    m_max = bounds[2*m_planeIndex +1];

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

    m_actor = vtkSmartPointer<vtkAssembly>::New();
    m_actor->AddPart(m_symbolicActor);
    m_actor->Modified();

    m_view->addActor(m_actor);
    m_view->updateView();

    setPosition(vtkMath::Floor((bounds[2*m_planeIndex]/spacing[m_planeIndex] + 0.5)));
  }

} // namespace EspINA
