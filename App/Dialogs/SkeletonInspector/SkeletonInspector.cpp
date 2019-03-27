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
#include <Extensions/SkeletonInformation/DendriteInformation.h>
#include <GUI/ColorEngines/CategoryColorEngine.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/Utils/SegmentationUtils.h>
#include <GUI/Representations/Managers/ConnectionsManager.h>
#include <GUI/Representations/Pipelines/SegmentationMeshPipeline.h>
#include <GUI/Representations/Pipelines/SegmentationSkeleton3DPipeline.h>
#include <GUI/Representations/Settings/PipelineStateUtils.h>
#include <GUI/Representations/Settings/SegmentationSkeletonPoolSettings.h>
#include <GUI/Widgets/Styles.h>
#include <Support/Settings/Settings.h>
#include <Support/Representations/RepresentationFactory.h>
#include <Support/Representations/RepresentationUtils.h>
#include <Support/Utils/xlsUtils.h>

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
#include <vtkGlyphSource2D.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkGlyph3DMapper.h>
#include <vtkTransform.h>
#include <vtkFollower.h>
#include <vtkFreeTypeLabelRenderStrategy.h>

// Qt
#include <QToolBar>
#include <QListWidgetItem>
#include <QApplication>

// C++
#include <random>
#include <chrono>
#include <functional>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Model::Utils;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::Representations::Settings;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Widgets::Styles;
using namespace ESPINA::Support::Representations::Utils;
using namespace xlslib_core;

const QString SETTINGS_GROUP = "Skeleton Inspector Dialog";

//--------------------------------------------------------------------
SkeletonInspector::SkeletonInspector(Support::Context& context)
: QDialog              {DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint}}
, Support::WithContext (context)
, m_view               {getViewState(), false, this}
, m_segmentationSources(getViewState())
, m_temporalPipeline   {nullptr}
{
  setupUi(this);

  addInitialSegmentations();

  initView3D(context.availableRepresentations());

  initTreeView();

  initSpinesTable();

  restoreGeometry();
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
  settings.setValue("tableEnabled", m_spinesButton->isChecked());
  settings.endGroup();

  QDialog::closeEvent(event);

  m_segmentation->clearTemporalRepresentation();
  for(auto &stroke: m_strokes) stroke.actors.clear();
  m_strokes.clear();
  m_definition.clear();
}

//--------------------------------------------------------------------
void SkeletonInspector::createSkeletonActors(const SegmentationAdapterSPtr segmentation)
{
  Q_ASSERT(hasSkeletonData(segmentation->output()));

  const auto skeleton    = readLockSkeleton(segmentation->output())->skeleton();
  m_definition           = toSkeletonDefinition(skeleton);
  const auto pathList    = paths(m_definition.nodes, m_definition.edges, m_definition.strokes);
  const auto strokes     = m_definition.strokes;
  const auto connections = getModel()->connections(segmentation);

  auto seed = std::chrono::system_clock::now().time_since_epoch().count();
  std::minstd_rand0 rngenerator(seed);

  auto assignRandomFromHue = [&rngenerator](const int hue)
  {
    auto value = hue;
    while(hue-20 < value && value < hue+20)
    {
      value = rngenerator() % 360;
    }

    return value;
  };

  auto hierarchyColor = [&strokes](const SkeletonStroke &stroke)
  {
    int position = 0;
    QSet<int> hueValues;

    for(int i = 0; i < strokes.size(); ++i)
    {
      auto &otherStroke = strokes.at(i);

      hueValues << otherStroke.colorHue;

      if(otherStroke == stroke) continue;

      // alphabetic to keep certain order, but can be altered by introducing more strokes.
      if((otherStroke.colorHue == stroke.colorHue) && (otherStroke.name < stroke.name)) ++position;
    }

    int finalHue = stroke.colorHue;
    while((position > 0) && (position < 20) && hueValues.contains(finalHue))
    {
      finalHue = (stroke.colorHue + (50*position)) % 360;

      ++position;
    }

    return finalHue;
  };

  for(auto i = 0; i < pathList.size(); ++i)
  {
    auto path = pathList.at(i);

    struct StrokeInfo info;
    info.name         = path.note;
    info.path         = path;
    info.selected     = true;
    info.length       = path.length();
    info.used         = strokes.at(path.stroke).useMeasure;
    info.hue          = strokes.at(path.stroke).colorHue;
    info.randomHue    = assignRandomFromHue(info.hue);
    info.hierarchyHue = hierarchyColor(strokes.at(path.stroke));

    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(path.seen.size());

    auto lines = vtkSmartPointer<vtkCellArray>::New();
    lines->SetNumberOfCells(path.seen.size() -1);

    for(vtkIdType j = 0; j < path.seen.size(); ++j)
    {
      points->SetPoint(j, path.seen.at(j)->position);

      if(j > 0)
      {
        auto line = vtkSmartPointer<vtkLine>::New();
        line->GetPointIds()->SetId(0, j-1);
        line->GetPointIds()->SetId(1, j);

        lines->InsertNextCell(line);
      }
    }

    Q_ASSERT(!path.begin->connections.isEmpty());
    auto otherNode = path.seen.at(1);
    info.labelPoint = NmVector3{(path.begin->position[0]+otherNode->position[0])/2.0,
                                (path.begin->position[1]+otherNode->position[1])/2.0,
                                (path.begin->position[2]+otherNode->position[2])/2.0};

    for(auto node: {path.end, path.begin})
    {
      if (node->isTerminal())
      {
        if(!node->flags.testFlag(SkeletonNodeProperty::TRUNCATED))
        {
          info.labelPoint = NmVector3{node->position};
        }
        else
        {
          Q_ASSERT(!node->connections.isEmpty());
          otherNode = node->connections.keys().first();
          info.labelPoint = NmVector3{(node->position[0]+otherNode->position[0])/2.0,
                                      (node->position[1]+otherNode->position[1])/2.0,
                                      (node->position[2]+otherNode->position[2])/2.0};
        }
      }
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    auto mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    mapper->SetInputData(polyData);

    auto hue   = m_definition.strokes.at(path.stroke).colorHue;
    auto color = QColor::fromHsv(hue, 255,255);
    auto actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(color.redF(), color.greenF(), color.blueF());
    actor->GetProperty()->SetLineWidth(1);
    actor->SetPickable(false);
    if(m_definition.strokes.at(path.stroke).type != 0)
    {
      actor->GetProperty()->SetLineStipplePattern(0xFF00);
    }

    info.actors << actor;

    for(auto node: {path.begin, path.end})
    {
      if(node->isTerminal() && node->flags.testFlag(SkeletonNodeProperty::TRUNCATED))
      {
        auto truncatedPoints = vtkSmartPointer<vtkPoints>::New();
        truncatedPoints->SetNumberOfPoints(1);
        truncatedPoints->SetPoint(0, node->position);
        truncatedPoints->Modified();

        auto truncatedData = vtkSmartPointer<vtkPolyData>::New();
        truncatedData->SetPoints(truncatedPoints);
        truncatedData->Modified();

        auto glyph2D = vtkSmartPointer<vtkGlyphSource2D>::New();
        glyph2D->SetGlyphTypeToSquare();
        glyph2D->SetFilled(false);
        glyph2D->SetCenter(0,0,0);
        glyph2D->SetScale(30);
        glyph2D->SetColor(1,0,0);
        glyph2D->Update();

        auto glyphMapper = vtkSmartPointer<vtkGlyph3DMapper>::New();
        glyphMapper->SetScalarVisibility(false);
        glyphMapper->SetSourceIndexing(false);
        glyphMapper->SetStatic(true);
        glyphMapper->SetInputData(truncatedData);
        glyphMapper->SetSourceData(glyph2D->GetOutput());
        glyphMapper->Update();

        auto truncatedActor = vtkSmartPointer<vtkFollower>::New();
        truncatedActor->SetMapper(glyphMapper);
        truncatedActor->SetDragable(false);
        truncatedActor->SetPickable(false);
        truncatedActor->SetOrigin(node->position);
        truncatedActor->SetPosition(0,0,0);
        truncatedActor->GetProperty()->SetColor(1, 0, 0);
        truncatedActor->GetProperty()->Modified();
        truncatedActor->Modified();

        info.actors << truncatedActor;
        break;
      }
    }

    m_strokes << info;
  }

  qSort(m_strokes);
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
  auto spinesTableVisible = settings.value("tableEnabled", false).toBool();
  m_spinesButton->setChecked(spinesTableVisible);
  m_tabWidget->setVisible(spinesTableVisible);
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

    for(auto repSwitch: representation.Switches)
    {
      if(repSwitch->id() == "Skeleton3DSwitch") continue;
      switches << repSwitch;
    }

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

      if(manager->name() == "DisplaySkeleton3DRepresentation")
      {
        SkeletonPoolSettingsSPtr skeletonSettings = nullptr;
        for (auto settings : representation.Settings)
        {
          auto repSettings =  std::dynamic_pointer_cast<SegmentationSkeletonPoolSettings>(settings);
          if (repSettings)
          {
            skeletonSettings = repSettings;
            break;
          }
        }
        Q_ASSERT(skeletonSettings);
        auto skeletonSwitch = std::make_shared<SkeletonInspectorRepresentationSwitch>(manager, skeletonSettings, getContext());
        switches << skeletonSwitch;
        representation.Switches << skeletonSwitch;

        connect(skeletonSwitch.get(), SIGNAL(coloringEnabled(bool)),
                this,                 SLOT(onColoringEnabled(bool)));

        connect(skeletonSwitch.get(), SIGNAL(randomColoringEnabled(bool)),
                this,                 SLOT(onRandomColoringEnabled(bool)));
      }
    }

    m_representations << representation;
  }

  auto comparisonOp = [] (const RepresentationSwitchSPtr &left, const RepresentationSwitchSPtr &right) { if(left == nullptr || right == nullptr) return false; return left->groupWith() < right->groupWith(); };
  std::sort(switches.begin(), switches.end(), comparisonOp);

  auto toolbar = new QToolBar();
  for(auto repSwitch: switches)
  {
    auto id = repSwitch->id();

    if(id == "SegmentationMeshSwitch" || id == "ConnectionsSwitch" || id == "SkeletonInspectorRepresentationSwitch")
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
void SkeletonInspector::addInitialSegmentations()
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
    }

    for(auto secondLevelConnection: model->connections(connection.item2))
    {
      if(secondLevelConnection.item2 == m_segmentation) continue;

      if(!segmentations.contains(secondLevelConnection.item2.get()))
      {
        segmentations << secondLevelConnection.item2.get();
      }
    }
  }

  updateWindowTitle(m_segmentation->data().toString());

  auto frame = getViewState().createFrame();
  m_segmentationSources.addSource(segmentations, frame);

  m_temporalPipeline = std::make_shared<SkeletonInspectorPipeline>(m_strokes);
  m_segmentation->setTemporalRepresentation(m_temporalPipeline);
}

//--------------------------------------------------------------------
void SkeletonInspector::initTreeView()
{
  auto model = new SkeletonInspectorTreeModel(m_segmentation, getModel(), m_strokes, m_treeView);
  m_treeView->setModel(model);
  m_treeView->setHeaderHidden(true);
  m_treeView->update();
  m_treeView->expandAll();

  m_treeView->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectItems);
  m_treeView->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  m_treeView->setExpandsOnDoubleClick(false);
  m_treeView->setCurrentIndex(model->index(0,0));

  connect(model, SIGNAL(invalidate(ViewItemAdapterList)),
          this,  SLOT(onRepresentationsInvalidated(ViewItemAdapterList)));

  connect(model, SIGNAL(segmentationsShown(const SegmentationAdapterList)),
          this,  SLOT(onSegmentationsShown(const SegmentationAdapterList)));

  connect(m_conDistance, SIGNAL(valueChanged(int)),
          this,          SLOT(onDistanceChanged(int)));

  connect(m_treeView, SIGNAL(doubleClicked(QModelIndex)),
          this,       SLOT(focusOnActor(QModelIndex)));

  connect(m_treeView->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)),
          this,                         SLOT(onCurrentChanged(const QModelIndex &, const QModelIndex &)));

  connect(getSelection().get(), SIGNAL(selectionChanged(SegmentationAdapterList)),
          model,                SLOT(onSelectionChanged(SegmentationAdapterList)));
}

//--------------------------------------------------------------------
void SkeletonInspector::focusOnActor(QModelIndex index)
{
  if(index.isValid())
  {
    auto dataNode = static_cast<SkeletonInspectorTreeModel::TreeNode *>(index.internalPointer());

    Bounds focusBounds;

    switch(dataNode->type)
    {
      case SkeletonInspectorTreeModel::TreeNode::Type::SEGMENTATION:
        focusBounds = dataNode->connection()->bounds();
        break;
      case SkeletonInspectorTreeModel::TreeNode::Type::STROKE:
        {
          auto stroke = dataNode->stroke();
          focusBounds = Bounds{stroke->actors.first()->GetBounds()};
        }
        break;
      case SkeletonInspectorTreeModel::TreeNode::Type::ROOT:
      default:
        // nothing to be done
        break;
    }

    if(focusBounds.areValid())
    {
      getViewState().focusViewOn(centroid(focusBounds));
    }
  }
}

//--------------------------------------------------------------------
void SkeletonInspector::focusOnActor(int row)
{
  Bounds focusBounds;
  auto name = m_table->item(row, 0)->data(Qt::DisplayRole);
  auto equalOp = [&name](const StrokeInfo &stroke) {return (stroke.name == name); };
  auto it = std::find_if(m_strokes.constBegin(), m_strokes.constEnd(), equalOp);

  if(it != m_strokes.constEnd())
  {
    auto focusBounds = Bounds{(*it).actors.first()->GetBounds()};

    if(focusBounds.areValid()) getViewState().focusViewOn(centroid(focusBounds));
  }
}

//--------------------------------------------------------------------
void SkeletonInspector::onCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
  auto selection = getSelection()->segmentations();
  bool invalidated = false;

  if(previous.isValid())
  {
    auto dataNode = static_cast<SkeletonInspectorTreeModel::TreeNode *>(previous.internalPointer());

    switch(dataNode->type)
    {
      case SkeletonInspectorTreeModel::TreeNode::Type::ROOT:
        if(previous.row() == 0)
        {
          for(auto &stroke: m_strokes) stroke.selected = false;
          invalidated = true;
          selection.removeAll(m_segmentation.get());
        }
        else
        {
          for(auto item: m_segmentationSources.sources(ItemAdapter::Type::SEGMENTATION)) selection.removeAll(segmentationPtr(item));
        }
        break;
      case SkeletonInspectorTreeModel::TreeNode::Type::STROKE:
        dataNode->stroke()->selected = false;
        invalidated = true;
        break;
      case SkeletonInspectorTreeModel::TreeNode::Type::SEGMENTATION:
        selection.removeAll(dataNode->connection());
        break;
      default:
        break;
    }
  }

  if(current.isValid())
  {
    auto dataNode = static_cast<SkeletonInspectorTreeModel::TreeNode *>(current.internalPointer());

    switch(dataNode->type)
    {
      case SkeletonInspectorTreeModel::TreeNode::Type::ROOT:
        if(current.row() == 0)
        {
          for(auto &stroke: m_strokes) stroke.selected = true;
          invalidated = true;
          selection << m_segmentation.get();
        }
        else
        {
          for(auto item: m_segmentationSources.sources(ItemAdapter::Type::SEGMENTATION))  selection << segmentationPtr(item);
        }
        break;
      case SkeletonInspectorTreeModel::TreeNode::Type::STROKE:
        {
          dataNode->stroke()->selected = true;
          auto name = dataNode->stroke()->name;
          m_table->blockSignals(true);
          for(int i = 0; i < m_table->rowCount(); ++i)
          {
            if(m_table->item(i,0)->data(Qt::DisplayRole) == name)
            {
              m_table->selectRow(i);
              break;
            }
          }
          m_table->blockSignals(false);
          invalidated = true;
        }
        break;
      case SkeletonInspectorTreeModel::TreeNode::Type::SEGMENTATION:
        selection << dataNode->connection();
        break;
      default:
        break;
    }
  }

  if(!selection.isEmpty()) getSelection()->set(selection);
  if(invalidated) m_segmentation->invalidateRepresentations();
  m_view.refresh();
}

//--------------------------------------------------------------------
void SkeletonInspector::onRepresentationsInvalidated(ViewItemAdapterList segmentations)
{
  getViewState().invalidateRepresentations(segmentations);
}

//--------------------------------------------------------------------
void SkeletonInspector::onColoringEnabled(bool value)
{
  if(m_temporalPipeline)
  {
    m_temporalPipeline->setHierarchyColors(value);

    m_segmentation->invalidateRepresentations();
    m_view.refresh();
  }

  auto treeModel = dynamic_cast<SkeletonInspectorTreeModel *>(m_treeView->model());
  if(treeModel) treeModel->setHierarchyTreeColoring(value);
}

//--------------------------------------------------------------------
void SkeletonInspector::onRandomColoringEnabled(bool value)
{
  if(m_temporalPipeline)
  {
    m_temporalPipeline->setRandomColors(value);

    m_segmentation->invalidateRepresentations();
    m_view.refresh();
  }

  auto treeModel = dynamic_cast<SkeletonInspectorTreeModel *>(m_treeView->model());
  if(treeModel) treeModel->setRandomTreeColoring(value);
}

//--------------------------------------------------------------------
SkeletonInspector::SkeletonInspectorPipeline::SkeletonInspectorPipeline(QList<struct StrokeInfo>& strokes)
: SegmentationSkeleton3DPipeline(std::make_shared<CategoryColorEngine>())
, m_strokes(strokes)
, m_randomColoring   {false}
, m_hierarchyColoring{false}
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

  for(auto &stroke: m_strokes)
  {
    QColor color;
    if(!m_randomColoring && !m_hierarchyColoring)
    {
      color = m_colorEngine->color(segmentation);
      if(color.hue() != stroke.hue)
      {
        color = QColor::fromHsv(stroke.hue, 255,255);
      }
    }
    else
    {
      if(m_randomColoring) color = QColor::fromHsv(stroke.randomHue, 255,255);
      else                 color = QColor::fromHsv(stroke.hierarchyHue, 255,255);
    }

    s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

    double factor = stroke.selected ? 1.0 : 0.65;
    auto actor = stroke.actors.first();
    actor->GetProperty()->SetLineWidth(stroke.selected ? width + 2 : width);
    actor->GetProperty()->SetColor(rgba[0]*factor, rgba[1]*factor, rgba[2]*factor);
    actor->Modified();
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

  // line actors.
  for(auto &stroke: m_strokes)
  {
    if(!stroke.actors.first()->GetVisibility()) continue;

    double factor = stroke.selected ? 1.0 : 0.65;
    QColor color;
    if(!m_randomColoring && !m_hierarchyColoring)
    {
      color = m_colorEngine->color(segmentation);
      if(color.hue() != stroke.hue)
      {
        color = QColor::fromHsv(stroke.hue, 255,255);
      }
    }
    else
    {
      if(m_randomColoring) color = QColor::fromHsv(stroke.randomHue, 255,255);
      else                 color = QColor::fromHsv(stroke.hierarchyHue, 255,255);
    }

    s_highlighter.lut(color, item->isSelected())->GetTableValue(1,rgba);

    stroke.actors.first()->GetProperty()->SetLineWidth(stroke.selected ? width + 2 : width);
    stroke.actors.first()->GetProperty()->SetColor(rgba[0]*factor, rgba[1]*factor, rgba[2]*factor);

    actors << stroke.actors.first();
  }

  // truncated points actors.
  for(auto &stroke: m_strokes)
  {
    if(!stroke.actors.first()->GetVisibility()) continue;

    if(stroke.actors.size() > 1)
    {
      actors << stroke.actors.last();
    }
  }

  // label actors
  if(SegmentationSkeletonPoolSettings::getShowAnnotations(state) && item->isSelected())
  {
    auto color       = m_colorEngine->color(segmentation);
    auto labelPoints = vtkSmartPointer<vtkPoints>::New();
    auto labelText   = vtkSmartPointer<vtkStringArray>::New();
    labelText->SetName("Labels");

    auto labelsData = vtkSmartPointer<vtkPolyData>::New();
    labelsData->SetPoints(labelPoints);
    labelsData->GetPointData()->AddArray(labelText);

    auto textSize = SegmentationSkeletonPoolSettings::getAnnotationsSize(state);

    auto property = vtkSmartPointer<vtkTextProperty>::New();
    property->SetBold(true);
    property->SetFontFamilyToArial();
    property->SetFontSize(textSize);
    property->SetJustificationToCentered();

    auto labelFilter = vtkSmartPointer<vtkPointSetToLabelHierarchy>::New();
    labelFilter->SetInputData(labelsData);
    labelFilter->SetLabelArrayName("Labels");
    labelFilter->GetTextProperty()->SetFontSize(textSize);
    labelFilter->GetTextProperty()->SetBold(true);
    labelFilter->Update();

    auto strategy = vtkSmartPointer<vtkFreeTypeLabelRenderStrategy>::New();
    strategy->SetDefaultTextProperty(property);

    auto labelMapper = vtkSmartPointer<vtkLabelPlacementMapper>::New();
    labelMapper->SetInputConnection(labelFilter->GetOutputPort());
    labelMapper->SetGeneratePerturbedLabelSpokes(true);
    labelMapper->SetBackgroundColor(color.redF()*0.6, color.greenF()*0.6, color.blueF()*0.6);
    labelMapper->SetBackgroundOpacity(0.5);
    labelMapper->SetPlaceAllLabels(true);
    labelMapper->SetShapeToRoundedRect();
    labelMapper->SetStyleToFilled();

    auto labelActor = vtkSmartPointer<vtkActor2D>::New();
    labelActor->SetMapper(labelMapper);
    labelActor->SetVisibility(true);

    for(auto &stroke: m_strokes)
    {
      if(!stroke.actors.first()->GetVisibility()) continue;

      labelPoints->InsertNextPoint(stroke.labelPoint[0], stroke.labelPoint[1], stroke.labelPoint[2]);
      labelText->InsertNextValue(stroke.name.toStdString().c_str());
    }

    if(labelPoints->GetNumberOfPoints() > 0)
    {
      labelsData->Modified();
      labelFilter->Update();
      labelMapper->Update();
      labelActor->Modified();
      actors << labelActor;
    }
  }

  return actors;
}

//--------------------------------------------------------------------
void SkeletonInspector::onSpinesButtonClicked(bool checked)
{
  m_tabWidget->setVisible(checked);
}

//--------------------------------------------------------------------
void SkeletonInspector::onSpineSelected(const QModelIndex& index)
{
  if(!index.isValid()) return;

  auto row = index.row();
  auto name = m_table->item(row, 0)->data(Qt::DisplayRole).toString();

  for(auto &stroke: m_strokes)
  {
    stroke.selected = (stroke.name == name);
  }

  m_segmentation->invalidateRepresentations();
  m_view.refresh();
}

//--------------------------------------------------------------------
void SkeletonInspector::onSaveButtonPressed()
{
  auto dendriteName = m_segmentation->data().toString();
  auto title        = tr("Export %1's spines information").arg(dendriteName);
  auto suggestion   = QString("%1-spines.xls").arg(dendriteName).replace(' ','_');
  auto formats      = SupportedFormats().addExcelFormat().addCSVFormat();
  auto fileName     = DefaultDialogs::SaveFile(title, formats, QDir::homePath(), ".xls", suggestion, this);

  if (fileName.isEmpty()) return;

  // some users are used to not enter an extension, and expect a default xls output.
  if(!fileName.endsWith(".csv", Qt::CaseInsensitive) && !fileName.endsWith(".xls", Qt::CaseInsensitive))
  {
    fileName += tr(".xls");
  }

  if (fileName.endsWith(".csv", Qt::CaseInsensitive))
  {
    try
    {
      saveToCSV(fileName);
    }
    catch(const EspinaException &e)
    {
      auto message = tr("Couldn't export %1").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, e.details(), this);
    }
  }
  else if (fileName.endsWith(".xls", Qt::CaseInsensitive))
  {
    try
    {
      saveToXLS(fileName);
    }
    catch(const EspinaException &e)
    {
      auto message = tr("Couldn't export %1").arg(fileName);
      DefaultDialogs::InformationMessage(message, title, e.details(), this);
    }
  }
}

//--------------------------------------------------------------------
void SkeletonInspector::emitSegmentationConnectionSignals()
{
  emit aboutToBeReset();

  QApplication::processEvents();

  for(auto item: m_segmentationSources.sources(ItemAdapter::Type::SEGMENTATION))
  {
    auto segmentation = segmentationPtr(item);
    auto segmentationSPtr = getModel()->smartPointer(segmentation);
    auto connections = getModel()->connections(segmentationSPtr);

    for(auto connection: connections) emit connectionAdded(connection);
  }
}

//--------------------------------------------------------------------
SkeletonInspector::SkeletonInspectorRepresentationSwitch::SkeletonInspectorRepresentationSwitch(RepresentationManagerSPtr manager, SkeletonPoolSettingsSPtr settings, Support::Context &context)
: SegmentationSkeletonSwitch{"SkeletonInspectorRepresentationSwitch", manager, settings, ViewType::VIEW_3D, context}
{
  m_coloring = Styles::createToolButton(":/espina/skeletonColors.svg", QObject::tr("Enable/Disable hierarchy stroke coloring."));
  m_coloring->setCheckable(true);
  m_coloring->setChecked(false);

  connect(m_coloring, SIGNAL(toggled(bool)), this, SLOT(onButtonPressed(bool)));

  addSettingsWidget(m_coloring);

  m_coloringRandom = Styles::createToolButton(":/espina/skeletonColorsRandom.svg", QObject::tr("Enable/Disable stroke random coloring."));
  m_coloringRandom->setCheckable(true);
  m_coloringRandom->setChecked(false);

  connect(m_coloringRandom, SIGNAL(toggled(bool)), this, SLOT(onButtonPressed(bool)));

  addSettingsWidget(m_coloringRandom);

  setOrder("1-2","0-Representations");
}

//--------------------------------------------------------------------
void SkeletonInspector::SkeletonInspectorRepresentationSwitch::onButtonPressed(bool value)
{
  auto button = dynamic_cast<ToolButton *>(sender());
  if(button && (button == m_coloring || button == m_coloringRandom))
  {
    if(button == m_coloring)
    {
      if(m_coloringRandom->isChecked())
      {
        m_coloringRandom->blockSignals(true);
        m_coloringRandom->setChecked(false);
        m_coloringRandom->blockSignals(false);
        emit randomColoringEnabled(false);
      }
      emit coloringEnabled(value);
    }
    else
    {
      if(m_coloring->isChecked())
      {
        m_coloring->blockSignals(true);
        m_coloring->setChecked(false);
        m_coloring->blockSignals(false);
        emit coloringEnabled(false);
      }
      emit randomColoringEnabled(value);
    }
  }
}

//--------------------------------------------------------------------
void SkeletonInspector::onDistanceChanged(int distance)
{
  auto model = dynamic_cast<SkeletonInspectorTreeModel *>(m_treeView->model());

  if(model)
  {
    model->computeConnectionDistances(distance);

    m_treeView->expand(model->index(1,0, QModelIndex()));

    m_segmentation->invalidateRepresentations();
    m_view.refresh();
  }
}

//--------------------------------------------------------------------
void SkeletonInspector::onSegmentationsShown(const SegmentationAdapterList segmentations)
{
  addSegmentations(segmentations);

  m_view.refresh();
}

//--------------------------------------------------------------------
void SkeletonInspector::addSegmentations(const SegmentationAdapterList &segmentations)
{
  auto currentSources = m_segmentationSources.sources(ItemAdapter::Type::SEGMENTATION);
  currentSources.removeOne(m_segmentation.get());

  ViewItemAdapterList addedSegmentations;

  for(auto segmentation: segmentations)
  {
    if(currentSources.contains(segmentation))
    {
      currentSources.removeOne(segmentation);
    }
    else
    {
      addedSegmentations << segmentation;
    }
  }

  if(!addedSegmentations.isEmpty()) m_segmentationSources.addSource(addedSegmentations, getViewState().createFrame());
  if(!currentSources.isEmpty())     m_segmentationSources.removeSource(currentSources, getViewState().createFrame());

  if(!addedSegmentations.isEmpty() || !currentSources.isEmpty())
  {
    emitSegmentationConnectionSignals();
  }
}

//--------------------------------------------------------------------
void SkeletonInspector::initSpinesTable()
{
  m_tabWidget->setTabText(0, tr("%1's spines").arg(m_segmentation->data().toString()));

  auto iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  m_saveButton->setIcon(iconSave);
  m_saveButton->setIconSize(QSize(iconSize(), iconSize()));
  m_saveButton->setMaximumSize(buttonSize(), buttonSize());
  m_saveButton->setMinimumSize(buttonSize()*0.66, buttonSize()*0.66);

  connect(m_saveButton, SIGNAL(pressed()), this, SLOT(onSaveButtonPressed()));

  if(!m_segmentation->category()->classificationName().startsWith("Dendrite"))
  {
    m_spinesButton->setVisible(false);
    m_tabWidget->setVisible(false);
    return;
  }

  m_spinesButton->setChecked(false);

  connect(m_spinesButton, SIGNAL(clicked(bool)), this, SLOT(onSpinesButtonClicked(bool)));
  connect(m_table, SIGNAL(cellClicked(int, int)), this, SLOT(onCellClicked(int)));
  connect(m_table, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(focusOnActor(int)));

  auto extension = retrieveOrCreateSegmentationExtension<DendriteSkeletonInformation>(m_segmentation, getFactory());

  auto spines = extension->spinesInformation();

  auto headers = QString("Name;Complete;Branched;Length (Nm);Num of synapses;Num asymmetric;Num asymmetric on head;Num asymmetric on neck;");
  headers     += QString("Num symmetric;Num symmetric on head;Num symmetric on neck;Num of contacted axons;Num of inhibitory axons;Num of excitatory axons");

  m_table->setEditTriggers(QAbstractItemView::EditTrigger::NoEditTriggers);
  m_table->setColumnCount(14);
  m_table->setRowCount(spines.size());
  m_table->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
  m_table->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  m_table->setHorizontalHeaderLabels(headers.split(";"));
  m_table->sortByColumn(0, Qt::AscendingOrder);
  m_table->setSortingEnabled(false);

  for(int i = 0; i < spines.size(); ++i)
  {
    auto spine = spines.at(i);
    m_table->setItem(i, 0,new Item(spine.name));
    m_table->setItem(i, 1,new Item(spine.complete ? "yes":"no"));
    m_table->setItem(i, 2,new Item(spine.branched ? "yes":"no"));
    m_table->setItem(i, 3,new Item(QString::number(spine.length)));
    m_table->setItem(i, 4,new Item(QString::number(spine.numSynapses)));
    m_table->setItem(i, 5,new Item(QString::number(spine.numAsymmetric)));
    m_table->setItem(i, 6,new Item(QString::number(spine.numAsymmetricHead)));
    m_table->setItem(i, 7,new Item(QString::number(spine.numAsymmetricNeck)));
    m_table->setItem(i, 8,new Item(QString::number(spine.numSymmetric)));
    m_table->setItem(i, 9,new Item(QString::number(spine.numSymmetricHead)));
    m_table->setItem(i,10,new Item(QString::number(spine.numSymmetricNeck)));
    m_table->setItem(i,11,new Item(QString::number(spine.numAxons)));
    m_table->setItem(i,12,new Item(QString::number(spine.numAxonsInhibitory)));
    m_table->setItem(i,13,new Item(QString::number(spine.numAxonsExcitatory)));
  }

  connect(m_table->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex &, const QModelIndex &)),
          this,                      SLOT(onSpineSelected(const QModelIndex &)));

  m_table->resizeColumnsToContents();
  m_table->adjustSize();
  m_table->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  m_table->setSortingEnabled(true);

  m_tabWidget->setVisible(false);
}

//--------------------------------------------------------------------
void SkeletonInspector::saveToCSV(const QString& filename) const
{
  QFile file(filename);

  if(!file.open(QIODevice::WriteOnly|QIODevice::Text) || !file.isWritable() || !file.setPermissions(QFile::ReadOwner|QFile::WriteOwner|QFile::ReadOther|QFile::WriteOther))
  {
    auto what    = tr("exportToCSV: can't save file '%1'.").arg(filename);
    auto details = tr("Cause of failure: %1").arg(file.errorString());

    throw EspinaException(what, details);
  }

  QTextStream out(&file);

  for (int c = 0; c < m_table->columnCount(); c++)
  {
    out << m_table->horizontalHeaderItem(c)->data(Qt::DisplayRole).toString() << ",";
  }
  out << "Parent dendrite\n";

  auto dendriteName = m_segmentation->data().toString();
  for (int r = 0; r < m_table->rowCount(); r++)
  {
    for (int c = 0; c < m_table->columnCount(); c++)
    {
      out << m_table->item(r, c)->data(Qt::DisplayRole).toString().remove('\'') << ",";
    }
    out << dendriteName << "\n";
  }
  file.close();
}

//--------------------------------------------------------------------
void SkeletonInspector::onCellClicked(int row)
{
  auto name = m_table->item(row,0)->data(Qt::DisplayRole).toString();

  std::function<const QModelIndex (const QModelIndex &index, const QAbstractItemModel *model)> checkIndex = [&name, &checkIndex](const QModelIndex &index, const QAbstractItemModel *model)
  {
    if(index.isValid() && index.data(Qt::DisplayRole).toString() == name)
    {
      return index;
    }

    if(model->hasChildren(index))
    {
      for(auto i = 0; i < model->rowCount(index); ++i)
      {
        for(auto j = 0; j < model->columnCount(index); ++j)
        {
          auto child = checkIndex(model->index(i,j, index), model);
          if(child != QModelIndex()) return child;
        }
      }
    }

    return QModelIndex();
  };

  auto index = checkIndex(m_treeView->rootIndex(), m_treeView->model());
  if(index != QModelIndex())
  {
    m_treeView->blockSignals(true);
    m_treeView->setCurrentIndex(index);
    m_treeView->blockSignals(false);
  }
}

//--------------------------------------------------------------------
void SkeletonInspector::saveToXLS(const QString& filename) const
{
  workbook wb;

  auto excelSheet = wb.sheet(m_segmentation->data().toString().replace(' ','_').toStdString());

  for (int c = 0; c < m_table->columnCount(); ++c)
  {
    createCell(excelSheet, 0, c, m_table->horizontalHeaderItem(c)->data(Qt::DisplayRole).toString());
  }
  createCell(excelSheet, 0, m_table->columnCount(), tr("Parent dendrite"));

  auto dendriteName = m_segmentation->data().toString();
  for (int r = 0; r < m_table->rowCount(); ++r)
  {
    for (int c = 0; c < m_table->columnCount(); ++c)
    {
      createCell(excelSheet, r+1, c, m_table->item(r, c)->data(Qt::DisplayRole).toString().remove('\''));
    }
    createCell(excelSheet, r+1, m_table->columnCount(), dendriteName);
  }

  auto result = wb.Dump(filename.toStdString());

  if(result != NO_ERRORS)
  {
    auto what    = tr("exportToXLS: can't save file '%1'.").arg(filename);
    auto details = tr("Cause of failure: %1").arg(result == FILE_ERROR ? "file error" : "general error");

    throw EspinaException(what, details);
  }
}
