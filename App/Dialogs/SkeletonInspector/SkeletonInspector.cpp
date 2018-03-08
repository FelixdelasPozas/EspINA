/*

 Copyright (C) 2017 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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
#include <App/Dialogs/SkeletonInspector/SkeletonInspector.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Data/SkeletonDataUtils.h>
#include <Core/Analysis/Data/MeshData.h>
#include <Core/Utils/ListUtils.hxx>
#include <GUI/ColorEngines/CategoryColorEngine.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Managers/ConnectionsManager.h>
#include <GUI/Representations/Pipelines/SegmentationMeshPipeline.h>
#include <GUI/Representations/Pipelines/SegmentationSkeleton3DPipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>
#include <Support/Settings/Settings.h>
#include <Support/Representations/RepresentationFactory.h>
#include <Support/Representations/RepresentationUtils.h>

// VTK
#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkPoints.h>
#include <vtkCellArray.h>
#include <vtkLine.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkIntArray.h>
#include <vtkStringArray.h>
#include <vtkIdList.h>
#include <vtkTextProperty.h>
#include <vtkPointSetToLabelHierarchy.h>
#include <vtkLabelPlacementMapper.h>
#include <vtkActor2D.h>

// Qt
#include <QToolBar>
#include <QListWidgetItem>
#include <QApplication>

using namespace ESPINA;
using namespace Core;
using namespace GUI;
using namespace GUI::Model::Utils;
using namespace GUI::ColorEngines;
using namespace GUI::Representations;
using namespace GUI::Representations::Managers;
using namespace GUI::Representations::Settings;
using namespace Support::Representations::Utils;

const QString SETTINGS_GROUP = "Skeleton Inspector Dialog";

//--------------------------------------------------------------------
SkeletonInspector::SkeletonInspector(Support::Context& context)
: QDialog              {DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint}}
, Support::WithContext (context)
, m_view               {getViewState(), false, nullptr}
, m_segmentationSources(getViewState())
{
  setupUi(this);

  addSegmentations();

  initView3D(context.availableRepresentations());

  initTreeView();

  restoreGeometry();

  connectSignals();
}

//--------------------------------------------------------------------
void SkeletonInspector::closeEvent(QCloseEvent *event)
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  settings.setValue("size", size());
  settings.setValue("pos", pos());
  settings.setValue("size1", m_splitter->sizes().at(0));
  settings.setValue("size2", m_splitter->sizes().at(1));
  settings.endGroup();

  QDialog::closeEvent(event);

  m_segmentation->clearTemporalRepresentation();
  for(auto stroke: m_strokes) stroke.actor = nullptr;
  m_strokes.clear();
}

//--------------------------------------------------------------------
void SkeletonInspector::createSkeletonActors(const SegmentationAdapterSPtr segmentation)
{
  Q_ASSERT(hasSkeletonData(segmentation->output()));

  auto skeleton    = readLockSkeleton(segmentation->output())->skeleton();
  auto definition  = toSkeletonDefinition(skeleton);
  auto pathList    = paths(definition.nodes, definition.edges, definition.strokes);
  auto connections = getModel()->connections(segmentation);

  for(auto i = 0; i < pathList.size(); ++i)
  {
    auto path = pathList.at(i);

    struct StrokeInfo info;
    info.name   = path.note;
    info.length = path.length();
    info.used   = definition.strokes.at(path.stroke).useMeasure;
    info.hue    = definition.strokes.at(path.stroke).colorHue;

    double distance = 0;
    QString branchName;
    for(int i = 0; i < path.seen.size(); ++i)
    {
      auto node = path.seen.at(i);
      if(i != 0) distance += std::sqrt(vtkMath::Distance2BetweenPoints(node->position, path.seen.at(i-1)->position));

      switch(node->connections.size())
      {
        case 1:
          {
            for(auto connection: connections)
            {
              auto point = connection.point;
              if(node->position[0] == point[0] && node->position[1] == point[1] && node->position[2] == point[2])
              {
                ++info.connectionNum;
                auto text = tr("<b>%1</b> at point <a href=""%2"">%2</a>. ").arg(connection.item2->data().toString()).arg(connection.point.toString());
                info.connections += text;
              }
            }
          }
          break;
        case 2:
          continue;
          break;
        default:
          {
            int count = 0;
            for(auto value: node->connections.values())
            {
              if(value == path.edge) ++count;
            }
            if(count != 2) continue;

            for(auto connection: node->connections.keys())
            {
              auto otherEdgeIndex = node->connections[connection];
              if(otherEdgeIndex != path.edge)
              {
                ++info.numBranches;
                auto otherEdgeInfo = definition.edges.at(otherEdgeIndex);
                auto name = QString("%1 %2").arg(definition.strokes.at(otherEdgeInfo.strokeIndex).name).arg(otherEdgeInfo.strokeNumber);

                if(!branchName.isEmpty())
                {
                  info.branchDistances += QString("%1<->%2 = %3 nm. ").arg(branchName).arg(name).arg(distance);
                }

                branchName = name;
                distance = 0;

                double branchAngle = 181.0;
                for(auto otherConnection: node->connections.keys())
                {
                  if(node->connections[otherConnection] != path.edge) continue;
                  auto otherBranchAngle = Core::angle(node, connection, otherConnection);
                  if(otherBranchAngle < branchAngle) branchAngle = otherBranchAngle;
                }

                info.branchAngles += QString("%1 = %2 degrees. ").arg(name).arg(branchAngle);
              }
            }
          }
          break;
      }
    }

    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(path.seen.size());

    auto lines = vtkSmartPointer<vtkCellArray>::New();
    lines->SetNumberOfCells(path.seen.size() -1);

    for(vtkIdType i = 0; i < path.seen.size(); ++i)
    {
      points->SetPoint(i, path.seen.at(i)->position);

      if(i > 0)
      {
        auto line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, i-1);
        line->GetPointIds()->SetId(1, i);

        lines->InsertNextCell(line);
      }
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    auto hue   = definition.strokes.at(path.stroke).colorHue;
    auto color = QColor::fromHsv(hue, 255,255);
    info.actor = vtkSmartPointer<vtkActor>::New();
    info.actor->SetMapper(mapper);
    info.actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    info.actor->GetProperty()->SetLineWidth(1);
    info.actor->SetPickable(false);
    if(definition.strokes.at(path.stroke).type != 0)
    {
      info.actor->GetProperty()->SetLineStipplePattern(0xFF00);
    }

    m_strokes << info;
  }

  auto lessThan = [](const struct StrokeInfo &lhs, const struct StrokeInfo &rhs) { return lhs < rhs; };

  qSort(m_strokes.begin(), m_strokes.end(), lessThan);
}

//--------------------------------------------------------------------
void SkeletonInspector::showEvent(QShowEvent* event)
{
  QDialog::showEvent(event);

  m_view.resetCamera();

  emitSegmentationConnectionSignals();
}

//--------------------------------------------------------------------
void SkeletonInspector::updateWindowTitle(const QString& segmentationName)
{
  setWindowTitle(tr("Skeleton Inspector - %1").arg(segmentationName));
}

//--------------------------------------------------------------------
void SkeletonInspector::restoreGeometry()
{
  ESPINA_SETTINGS(settings);

  settings.beginGroup(SETTINGS_GROUP);
  resize(settings.value("size", QSize (1100, 800)).toSize());
  move  (settings.value("pos",  QPoint(200, 200)).toPoint());
  QList<int> pixelSizes;
  pixelSizes << settings.value("size1", 300).toInt() << settings.value("size2", 800).toInt();
  m_splitter->setSizes(pixelSizes);
  settings.endGroup();
}

//--------------------------------------------------------------------
void SkeletonInspector::initView3D(RepresentationFactorySList representations)
{
  m_view.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  m_view.setMinimumWidth(250);

  m_splitter->insertWidget(1, &m_view);

  RepresentationSwitchSList switches, toEnable;
  for (auto factory : representations)
  {
    auto representation = factory->createRepresentation(getContext(), ViewType::VIEW_3D);

    if(isStackRepresentation(representation)) continue;

    m_representations << representation;

    switches << representation.Switches;

    for (auto manager : representation.Managers)
    {
      m_view.addRepresentationManager(manager);

      for(auto pool: manager->pools())
      {
        pool->setPipelineSources(&m_segmentationSources);
      }

      if(manager->name() == "DisplayConnections")
      {
        auto conManager = std::dynamic_pointer_cast<ConnectionsManager>(manager);
        conManager->setConnectionsObject(this);
      }
    }
  }

  auto comparisonOp = [] (const RepresentationSwitchSPtr &left, const RepresentationSwitchSPtr &right) { if(left == nullptr || right == nullptr) return false; return left->groupWith() < right->groupWith(); };
  std::sort(switches.begin(), switches.end(), comparisonOp);

  auto toolbar = new QToolBar();
  for(auto repSwitch: switches)
  {
    auto id = repSwitch->id();

    if(id == "Skeleton3DSwitch" || id == "SegmentationMeshSwitch" || id == "ConnectionsSwitch")
    {
      toEnable << repSwitch;
    }
    else
    {
      if(id != "SegmentationVolumetricSwitch") continue;
    }

    for (auto action : repSwitch->actions())
    {
      toolbar->addAction(action);
    }
  }

  m_view.layout()->setMenuBar(toolbar);

  for(auto repSwitch: toEnable)
  {
    repSwitch->setChecked(true);
  }
}

//--------------------------------------------------------------------
void SkeletonInspector::addSegmentations()
{
  auto selection = getSelection()->segmentations();
  Q_ASSERT(selection.size() == 1);

  auto model     = getModel();
  m_segmentation = model->smartPointer(selection.first());

  createSkeletonActors(m_segmentation);

  auto connections = model->connections(m_segmentation);

  ViewItemAdapterList segmentations;
  segmentations << m_segmentation.get();

  for(auto connection: connections)
  {
    if(!segmentations.contains(connection.item2.get()))
    {
      segmentations << connection.item2.get();
      m_segmentations << connection.item2.get();
    }

    for(auto secondLevelConnection: model->connections(connection.item2))
    {
      if(secondLevelConnection.item2 == m_segmentation) continue;

      if(!segmentations.contains(secondLevelConnection.item2.get()))
      {
        segmentations << secondLevelConnection.item2.get();
        m_segmentations << secondLevelConnection.item2.get();
      }
    }
  }

  updateWindowTitle(m_segmentation->data().toString());

  auto frame = getViewState().createFrame();
  m_segmentationSources.addSource(segmentations, frame);

  qSort(m_segmentations.begin(), m_segmentations.end(), Core::Utils::lessThan<SegmentationAdapterPtr>);

  m_segmentation->setTemporalRepresentation(std::make_shared<SkeletonInspectorPipeline>(m_strokes));
}

//--------------------------------------------------------------------
void SkeletonInspector::initTreeView()
{
  auto model = new SkeletonInspectorTreeModel(m_segmentation, m_segmentations, m_strokes, m_treeView);
  m_treeView->setModel(model);
  m_treeView->setHeaderHidden(true);
  m_treeView->update();
  m_treeView->expandAll();

  connect(model, SIGNAL(invalidate(ViewItemAdapterList)),
          this,  SLOT(onRepresentationsInvalidated(ViewItemAdapterList)));
}

//--------------------------------------------------------------------
void SkeletonInspector::connectSignals()
{
  connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)),
          this,       SLOT(focusOnActor(QModelIndex)));

  connect(m_treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
          this,                         SLOT(onCurrentChanged(const QModelIndex &, const QModelIndex &)));

  connect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
          this,                 SLOT(onSelectionChanged(SegmentationAdapterList)));
}

//--------------------------------------------------------------------
void SkeletonInspector::focusOnActor(QModelIndex index)
{
  if(index.isValid())
  {
    auto parent = index.parent();
    NmVector3 point;

    if(parent.isValid())
    {
      if(parent.row() == 0)
      {
        double bounds[6];
        m_strokes.at(index.row()).actor->GetBounds(bounds);
        point = NmVector3{(bounds[1]+bounds[0])/2, (bounds[3]+bounds[2])/2, (bounds[5]+bounds[4])/2};
      }
      else
      {
        point = centroid(m_segmentations.at(index.row())->bounds());
      }

      getViewState().focusViewOn(point);
    }
    else
    {
      if(index.row() == 0)
      {
        point = centroid(m_segmentation->bounds());
        getViewState().focusViewOn(point);
      }
    }
  }
}

//--------------------------------------------------------------------
void SkeletonInspector::onCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
  if(previous.isValid())
  {
    if(!previous.parent().isValid())
    {
      if(previous.row() == 0)
      {
        getSelection()->clear();

        for(auto &stroke: m_strokes)
        {
          stroke.selected = false;
        }
      }
    }
    else
    {
      switch(previous.parent().row())
      {
        case 0:
          {
            m_strokes[previous.row()].selected = false;
          }
          break;
        default:
          getSelection()->clear();
          break;
      }
    }
  }

  if(current.isValid())
  {
    if(!current.parent().isValid())
    {
      if(current.row() == 0)
      {
        getSelection()->set(toViewItemList(m_segmentation.get()));
        for(auto &stroke: m_strokes)
        {
          stroke.selected = true;
        }
      }
    }
    else
    {
      switch(current.parent().row())
      {
        case 0:
          {
            getSelection()->set(toViewItemList(m_segmentation.get()));

            m_strokes[current.row()].selected = true;
          }
          break;
        default:
          getSelection()->set(toViewItemList(m_segmentations.at(current.row())));
          break;
      }
    }
  }

  m_segmentation->invalidateRepresentations();
  m_view.refresh();
}

//--------------------------------------------------------------------
void SkeletonInspector::onRepresentationsInvalidated(ViewItemAdapterList segmentations)
{
  getViewState().invalidateRepresentations(segmentations);
}

//--------------------------------------------------------------------
void SkeletonInspector::onSelectionChanged(SegmentationAdapterList segmentations)
{
  QModelIndex selectedIndex;

  for(int i = 0; i < m_segmentations.size(); ++i)
  {
    auto segmentation = m_segmentations.at(i);
    if(segmentations.contains(segmentation))
    {
      auto parent = m_treeView->model()->index(1,0, QModelIndex());
      selectedIndex = m_treeView->model()->index(i, 0, parent);
      break;
    }
  }

  if(selectedIndex.isValid())
  {
    m_treeView->selectionModel()->setCurrentIndex(selectedIndex, QItemSelectionModel::ClearAndSelect);
  }
}

//--------------------------------------------------------------------
SkeletonInspector::SkeletonInspectorPipeline::SkeletonInspectorPipeline(QList<struct StrokeInfo>& strokes)
: SegmentationSkeleton3DPipeline(std::make_shared<CategoryColorEngine>())
, m_strokes(strokes)
{
}

//--------------------------------------------------------------------
void SkeletonInspector::SkeletonInspectorPipeline::updateColors(RepresentationPipeline::ActorList& actors, ConstViewItemAdapterPtr item, const RepresentationState& state)
{
  auto segmentation = segmentationPtr(item);
  if(!segmentation) return;
  auto width = segmentation->isSelected() ? 2 : 0;
  width += SegmentationSkeletonPoolSettings::getWidth(state);
  double rgba[4];

  for(auto actor: actors)
  {
    auto actor2D = vtkActor2D::SafeDownCast(actor.Get());

    if(actor2D)
    {
      actor2D->SetVisibility(SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected());
      break;
    }
  }

  for(auto stroke: m_strokes)
  {
    auto color = m_colorEngine->color(segmentation);
    if(color.hue() != stroke.hue)
    {
      color = QColor::fromHsv(stroke.hue, 255,255);
    }

    s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

    double factor = stroke.selected ? 1.0 : 0.65;
    stroke.actor->GetProperty()->SetLineWidth(stroke.selected ? width + 2 : width);
    stroke.actor->GetProperty()->SetColor(rgba[0]*factor, rgba[1]*factor, rgba[2]*factor);
    stroke.actor->Modified();
  }
}

//--------------------------------------------------------------------
bool SkeletonInspector::SkeletonInspectorPipeline::pick(ConstViewItemAdapterPtr item, const NmVector3& point) const
{
  return false;
}

//--------------------------------------------------------------------
RepresentationPipeline::ActorList SkeletonInspector::SkeletonInspectorPipeline::createActors(ConstViewItemAdapterPtr item, const RepresentationState& state)
{
  auto segmentation = segmentationPtr(item);
  auto width = segmentation->isSelected() ? 2 : 0;
  width += SegmentationSkeletonPoolSettings::getWidth(state);
  double rgba[4];

  RepresentationPipeline::ActorList actors;
  for(auto stroke: m_strokes)
  {
    double factor = stroke.selected ? 1.0 : 0.65;
    auto   color  = m_colorEngine->color(segmentation);
    if(color.hue() != stroke.hue)
    {
      color = QColor::fromHsv(stroke.hue, 255, 255);
    }

    s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

    stroke.actor->GetProperty()->SetLineWidth(stroke.selected ? width + 2 : width);
    stroke.actor->GetProperty()->SetColor(rgba[0]*factor, rgba[1]*factor, rgba[2]*factor);

    actors << stroke.actor;
  }

  if(SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected())
  {
    auto data = readLockSkeleton(segmentation->output())->skeleton();
    auto color = m_colorEngine->color(segmentation);
    QStringList ids;
    auto edgeNumbers  = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("EdgeNumbers"));
    auto strokeNames  = vtkStringArray::SafeDownCast(data->GetPointData()->GetAbstractArray("StrokeName"));
    auto cellIndexes  = vtkIntArray::SafeDownCast(data->GetCellData()->GetAbstractArray("LineIndexes"));
    auto edgeIndexes  = vtkIntArray::SafeDownCast(data->GetPointData()->GetAbstractArray("EdgeIndexes"));

    if(!edgeNumbers || !strokeNames || !cellIndexes || !edgeIndexes)
    {
      return actors;
    }

    auto labelPoints = vtkSmartPointer<vtkPoints>::New();
    auto labelText   = vtkSmartPointer<vtkStringArray>::New();
    labelText->SetName("Labels");

    data->GetLines()->InitTraversal();
    for(int i = 0; i < data->GetNumberOfLines(); ++i)
    {
      auto idList = vtkSmartPointer<vtkIdList>::New();
      data->GetLines()->GetNextCell(idList);

      auto index = cellIndexes->GetValue(i);
      auto text = QString(strokeNames->GetValue(edgeIndexes->GetValue(index)).c_str()) + " " + QString::number(edgeNumbers->GetValue(index));
      if(ids.contains(text)) continue;
      ids << text;

      labelPoints->InsertNextPoint(data->GetPoint(idList->GetId(1)));
      labelText->InsertNextValue(text.toStdString().c_str());
    }

    auto labelsData = vtkSmartPointer<vtkPolyData>::New();
    labelsData->SetPoints(labelPoints);
    labelsData->GetPointData()->AddArray(labelText);

    auto property = vtkSmartPointer<vtkTextProperty>::New();
    property->SetBold(true);
    property->SetFontFamilyToArial();
    property->SetFontSize(SegmentationSkeletonPoolSettings::getAnnotationsSize(state));
    property->SetJustificationToCentered();

    auto labelFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
    labelFilter->SetInputData(labelsData);
    labelFilter->SetLabelArrayName("Labels");
    labelFilter->SetTextProperty(property);
    labelFilter->Update();

    auto labelMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
    labelMapper->SetInputConnection(labelFilter->GetOutputPort());
    labelMapper->SetGeneratePerturbedLabelSpokes(true);
    labelMapper->SetBackgroundColor(color.redF()*0.6, color.greenF()*0.6, color.blueF()*0.6);
    labelMapper->SetPlaceAllLabels(true);
    labelMapper->SetShapeToRoundedRect();
    labelMapper->SetStyleToFilled();

    auto labelActor = vtkSmartPointer<vtkActor2D>::New();
    labelActor->SetMapper(labelMapper);
    labelActor->SetVisibility(true);

    actors << labelActor;
  }

  return actors;
}

//--------------------------------------------------------------------
void SkeletonInspector::emitSegmentationConnectionSignals()
{
  emit aboutToBeReset();

  QApplication::processEvents();

  for(auto segmentation: m_segmentations)
  {
    auto segmentationSPtr = getModel()->smartPointer(segmentation);
    auto connections = getModel()->connections(segmentationSPtr);

    for(auto connection: connections) emit connectionAdded(connection);
  }
}
