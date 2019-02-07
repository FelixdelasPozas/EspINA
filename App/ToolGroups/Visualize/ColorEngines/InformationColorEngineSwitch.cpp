/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// ESPINA
#include "InformationColorEngineSwitch.h"
#include <GUI/ColorEngines/InformationColorEngine.h>
#include <Core/Utils/ListUtils.hxx>
#include <Core/Utils/EspinaException.h>
#include <GUI/Widgets/ColorBar.h>
#include <GUI/Widgets/InformationSelector.h>
#include <GUI/Widgets/Styles.h>
#include <GUI/Widgets/ToolButton.h>
#include <GUI/Utils/Format.h>
#include <GUI/Utils/ColorRange.h>
#include <GUI/View/RenderView.h>

// Qt
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QThread>

// VTK
#include <vtkLookupTable.h>
#include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <vtkLegendBoxActor.h>
#include <vtkGlyphSource2D.h>
#include "../../../../GUI/Dialogs/RangeDefinitionDialog/ColorEngineRangeDefinitionDialog.h"
#include "../../../../GUI/Dialogs/RangeLimitsDialog/ColorEngineRangeLimitsDialog.h"

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::ColorEngines;
using namespace ESPINA::GUI::Utils;
using namespace ESPINA::GUI::Utils::Format;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::Support;

const QString EXTENSION_KEY        = "Extension";
const QString INFORMATION_KEY      = "Key";
const QString MAXIMUM_VALUE        = "Maximum range value";
const QString MINIMUM_VALUE        = "Minimum range value";
const QString RANGE_WIDGET_VISIBLE = "Range widget is visible";
const QString WIDGET_POSITION      = "Widget position";
const QString WIDGET_ORIENTATION   = "Widget orientation";
const QString WIDGET_TEXT_POSITION = "Widget text position";
const QString WIDGET_NUM_LABELS    = "Widget number of labels";
const QString WIDGET_WIDTH         = "Widget relative width";
const QString WIDGET_HEIGHT        = "Widget relative height";
const QString WIDGET_BAR_RATIO     = "Widget bar ratio";
const QString WIDGET_NUM_DECIMALS  = "Widget number of decimals";

const QList<QVariant::Type> NUMERICAL_TYPES = { QVariant::Int, QVariant::UInt, QVariant::LongLong, QVariant::ULongLong, QVariant::Double, QVariant::Bool };

//-----------------------------------------------------------------------------
UpdateColorEngineTask::UpdateColorEngineTask(const SegmentationExtension::InformationKey &key,
                                             SegmentationAdapterList                      segmentations,
                                             ModelFactorySPtr                             factory,
                                             SchedulerSPtr                                scheduler)
: Task           {scheduler}
, m_key          {key}
, m_segmentations{segmentations}
, m_factory      {factory}
, m_error        {"No error"}
, m_failed       {false}
, m_invalid      {0}
, m_min          {std::numeric_limits<double>::max()}
, m_max          {std::numeric_limits<double>::min()}
{
  setDescription(tr("Coloring by %1").arg(m_key.value()));
}

//-----------------------------------------------------------------------------
void UpdateColorEngineTask::run()
{
  double i     = 0;
  double total = m_segmentations.size();

  auto extensionType = m_factory->createSegmentationExtension(m_key.extension());

  for(auto segmentation : m_segmentations)
  {
    if (!canExecute() || m_failed) return;

    auto category = segmentation->category()->classificationName();

    if(extensionType->validCategory(category) && extensionType->validData(segmentation->output()))
    {
      try
      {
        auto extension = retrieveOrCreateSegmentationExtension(segmentation, m_key.extension(), m_factory);

        auto info = extension->information(m_key);

        if (info.isValid())
        {
          if(NUMERICAL_TYPES.contains(info.type()))
          {
            auto value = info.toDouble();

            m_min = std::min(m_min, value);
            m_max = std::max(m_max, value);
          }
          else
          {
            if(info.type() == QVariant::String)
            {
              auto value = info.toString();
              if(!m_categories.contains(value)) m_categories << value;
            }
            else
            {
              ++m_invalid;
            }
          }
        }
      }
      catch(const EspinaException &e)
      {
        m_failed = true;
        m_error = e.details();

        reportProgress(100);
        return;
      }
    }
    else
    {
      ++m_invalid;
    }

    reportProgress(i++/total*100);
  }

  qSort(m_categories);
}

//-----------------------------------------------------------------------------
const double UpdateColorEngineTask::min() const
{
  if(m_failed) return 0.;

  if(isCategorical()) return 0;

  return m_min;
}

//-----------------------------------------------------------------------------
const double UpdateColorEngineTask::max() const
{
  if(m_failed) return -1.;

  if(isCategorical()) return m_categories.size() - 1;

  return m_max;
}

//-----------------------------------------------------------------------------
const bool UpdateColorEngineTask::isCategorical() const
{
  return !m_categories.isEmpty();
}

//-----------------------------------------------------------------------------
const QStringList UpdateColorEngineTask::categories() const
{
  return m_categories;
}

//-----------------------------------------------------------------------------
InformationColorEngineSwitch::InformationColorEngineSwitch(Context& context)
: ColorEngineSwitch{std::make_shared<InformationColorEngine>(), ":espina/color_engine_switch_property.svg", context}
, m_key            {informationColorEngine()->information()}
, m_needUpdate     {true}
, m_minimum        {std::numeric_limits<double>::max()}
, m_maximum        {std::numeric_limits<double>::min()}
, m_repFactory     {nullptr}
{
  createWidgets();

  connect(this, SIGNAL(toggled(bool)),
          this, SLOT(onToolToggled(bool)));
}

//-----------------------------------------------------------------------------
InformationColorEngineSwitch::~InformationColorEngineSwitch()
{
  disconnect(m_property, SIGNAL(linkActivated(QString)),
             this,       SLOT(changeProperty()));

  disconnect(this, SIGNAL(toggled(bool)),
             this, SLOT(onToolToggled(bool)));

  if(isChecked()) onToolToggled(false);

  abortTask();

  m_widgets.clear();

  if(m_properties.colors)
  {
    delete m_properties.colors;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::restoreSettings(std::shared_ptr< QSettings > settings)
{
  auto extension            = settings->value(EXTENSION_KEY,        "MorphologicalInformation").toString();
  auto key                  = settings->value(INFORMATION_KEY,      "Size").toString();
  m_minimum                 = settings->value(MINIMUM_VALUE,        std::numeric_limits<double>::max()).toDouble();
  m_maximum                 = settings->value(MAXIMUM_VALUE,        std::numeric_limits<double>::min()).toDouble();
  auto widgetsVisible       = settings->value(RANGE_WIDGET_VISIBLE, false).toBool();
  m_properties.numLabels    = settings->value(WIDGET_NUM_LABELS,    4).toUInt();
  m_properties.width        = settings->value(WIDGET_WIDTH,         0.1).toDouble();
  m_properties.height       = settings->value(WIDGET_HEIGHT,        0.5).toDouble();
  m_properties.barRatio     = settings->value(WIDGET_BAR_RATIO,     0.25).toDouble();
  m_properties.decimals     = settings->value(WIDGET_NUM_DECIMALS,  6).toUInt();
  m_properties.position     = static_cast<GUI::ColorEngineRangeDefinitionDialog::Position>(settings->value(WIDGET_POSITION, 1).toInt());
  m_properties.orientation  = static_cast<GUI::ColorEngineRangeDefinitionDialog::Orientation>(settings->value(WIDGET_ORIENTATION, 1).toInt());
  m_properties.textPosition = static_cast<GUI::ColorEngineRangeDefinitionDialog::TextPosition>(settings->value(WIDGET_TEXT_POSITION, 0).toInt());

  // Check if extension key is valid, could be that the extension is from a plugin not loaded/present right now.
  if(!getContext().factory()->availableSegmentationExtensions().contains(extension))
  {
    extension = "MorphologicalInformation";
    key       = "Size";

    m_limitsButton->setEnabled(false);
    m_properties.title = key;
  }

  m_key = InformationKey(extension, key);

  if(widgetsVisible)
  {
    if(!m_repFactory) createRepresentationFactory();

    updateWidgetsSettings();
  }

  restoreCheckedState(settings);
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::saveSettings(std::shared_ptr< QSettings > settings)
{
  saveCheckedState(settings);

  settings->setValue(EXTENSION_KEY,        m_key.extension());
  settings->setValue(INFORMATION_KEY,      m_key.value());
  settings->setValue(MINIMUM_VALUE,        m_minimum);
  settings->setValue(MAXIMUM_VALUE,        m_maximum);
  settings->setValue(RANGE_WIDGET_VISIBLE, m_repFactory != nullptr);
  settings->setValue(WIDGET_POSITION,      static_cast<int>(m_properties.position));
  settings->setValue(WIDGET_ORIENTATION,   static_cast<int>(m_properties.orientation));
  settings->setValue(WIDGET_TEXT_POSITION, static_cast<int>(m_properties.textPosition));
  settings->setValue(WIDGET_NUM_LABELS,    m_properties.numLabels);
  settings->setValue(WIDGET_WIDTH,         m_properties.width);
  settings->setValue(WIDGET_HEIGHT,        m_properties.height);
  settings->setValue(WIDGET_BAR_RATIO,     m_properties.barRatio);
  settings->setValue(WIDGET_NUM_DECIMALS,  m_properties.decimals);
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::createPropertySelector()
{
  auto label = new QLabel(tr("Color by:"));
  m_property = new QLabel();

  m_property->setOpenExternalLinks(false);

  connect(m_property, SIGNAL(linkActivated(QString)),
          this,       SLOT(changeProperty()));


  addSettingsWidget(label);
  addSettingsWidget(m_property);
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::createColorRange()
{
  m_rangeButton = Styles::createToolButton(QIcon(), tr("Coloring range"));
  m_rangeButton->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
  m_rangeButton->setMaximumWidth(Styles::mediumBarWidth()+4);
  m_rangeButton->setMinimumWidth(Styles::mediumBarWidth()+4);
  m_rangeButton->setIconSize(QSize{Styles::mediumBarWidth(), 20});

  auto image = ColorBar::rangeImage(informationColorEngine()->colorRange(), Styles::mediumBarWidth()-2, 20-2);
  m_rangeButton->setIcon(QPixmap::fromImage(image));

  connect(m_rangeButton, SIGNAL(clicked(bool)), this, SLOT(onRangeButtonClicked()));

  addSettingsWidget(m_rangeButton);
}

//-----------------------------------------------------------------------------
InformationColorEngine* InformationColorEngineSwitch::informationColorEngine() const
{
  return dynamic_cast<InformationColorEngine *>(colorEngine().get());
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::update()
{
  updateRange(m_key);

  updateLink();

  m_needUpdate = false;
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::updateLink()
{
  m_property->setText(createLink(m_key.value()));
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::changeProperty()
{
  auto segmentations = toRawList<SegmentationAdapter>(getModel()->segmentations());
  auto available     = availableInformation(segmentations, getFactory());

  auto selection = InformationSelector::GroupedInfo();

  selection[m_key.extension()] << m_key;

  InformationSelector propertySelector(available, selection, tr("Select property to color by"), true);

  if (propertySelector.exec() == QDialog::Accepted)
  {
    auto extension  = selection.keys().first();
    auto value      = selection[extension].first();

    auto key = InformationKey(extension, value);

    m_needUpdate = true;

    updateRange(key);
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::updateCurrentRange()
{
  updateRange(m_key);
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::updateRange(const InformationKey &key)
{
  if (m_task && m_task->isRunning())
  {
    m_task->abort();
  }

  m_limitsButton->setEnabled(false);

  auto segmentations = toRawList<SegmentationAdapter>(getModel()->segmentations());

  m_task = std::make_shared<UpdateColorEngineTask>(key,
                                                   segmentations,
                                                   getFactory(),
                                                   getScheduler());
  showTaskProgress(m_task);

  connect(m_task.get(), SIGNAL(finished()), this, SLOT(onTaskFinished()));

  Task::submit(m_task);
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::updateWidgetsSettings()
{
  for(auto widget: m_widgets)
  {
    auto base = std::dynamic_pointer_cast<InformationColorEngineWidgetRepresentationImplementation>(widget);
    if(base)
    {
      base->setProperties(m_properties);
    }
  }

  getViewState().refresh();
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onTaskFinished()
{
  auto task = qobject_cast<UpdateColorEngineTask *>(sender());
  bool success = true;

  if(task && (task == m_task.get()) && !task->isAborted())
  {
    if(task->hasFailed())
    {
      auto title   = tr("Coloring by %1").arg(task->property());
      auto message = tr("Couldn't color segmentations by property '%1'").arg(task->property());
      auto details = task->error();
      DefaultDialogs::ErrorMessage(message, title, details);

      success = false;
    }

    if(!task->hasFailed() && (task->invalid() != 0))
    {
      auto title   = tr("Coloring by %1").arg(task->property());
      auto message = tr("Some segmentations don't have the selected property '%1' and will be colored gray.").arg(task->property());
      auto details = tr("A total of %1 segmentations don't have the property '%2' (can't provide information of type '%3').").arg(task->invalid()).arg(task->property()).arg(task->extension());
      DefaultDialogs::InformationMessage(message, title, details);
    }
  }

  m_needUpdate = false;
  m_limitsButton->setEnabled(true);

  if(success)
  {
    m_key = InformationKey(task->extension(), task->property());

    m_minimum = std::numeric_limits<double>::max();
    m_maximum = std::numeric_limits<double>::min();

    updateLink();
    m_properties.title = task->property();

    auto extensionType = getFactory()->createSegmentationExtension(m_key.extension());
    informationColorEngine()->setExtension(extensionType);

    auto min = m_minimum != std::numeric_limits<double>::max() ? m_minimum : task->min();
    auto max = m_maximum != std::numeric_limits<double>::min() ? m_maximum : task->max();

    m_properties.colors->setMinimumValue(min);
    m_properties.colors->setMaximumValue(max);
    m_properties.categories = task->categories();

    if(task->isCategorical())
    {
      Q_ASSERT(!task->categories().isEmpty());

      informationColorEngine()->setInformation(m_key, task->categories());
    }
    else
    {
      informationColorEngine()->setInformation(m_key, min, max);
    }

    m_limitsButton->setEnabled(!task->isCategorical());

    if(m_repFactory)
    {
      updateWidgetsSettings();
    }
  }

  if(m_task.get() == task)
  {
    m_task = nullptr;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onToolToggled(bool checked)
{
  if (checked)
  {
    if (m_needUpdate)
    {
      update();
    }

    if(m_repFactory)
    {
      getViewState().addTemporalRepresentations(m_repFactory);
    }

    connect(getModel().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
            this,             SLOT(updateCurrentRange()));

    connect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
            this,             SLOT(updateCurrentRange()));
  }
  else
  {
    disconnect(getModel().get(), SIGNAL(segmentationsAdded(ViewItemAdapterSList)),
               this,             SLOT(updateCurrentRange()));

    disconnect(getModel().get(), SIGNAL(segmentationsRemoved(ViewItemAdapterSList)),
               this,             SLOT(updateCurrentRange()));

    if(m_repFactory)
    {
      getViewState().removeTemporalRepresentations(m_repFactory);
    }
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::createLimitsButton()
{
  m_limitsButton = Styles::createToolButton(":/espina/ruler-max.svg", tr("Specify the numerical limits of the range."));
  m_limitsButton->setCheckable(false);

  connect(m_limitsButton, SIGNAL(pressed()), this, SLOT(onLimitsButtonPressed()));

  addSettingsWidget(m_limitsButton);
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::abortTask()
{
  if(m_task != nullptr)
  {
    disconnect(m_task.get(), SIGNAL(finished()), this, SLOT(onTaskFinished()));

    m_task->abort();

    if(!m_task->thread()->wait(100))
    {
      m_task->thread()->terminate();
    }

    m_task = nullptr;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::createWidgets()
{
  createPropertySelector();

  createColorRange();

  createLimitsButton();
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onLimitsButtonPressed()
{
  auto min = (m_minimum != std::numeric_limits<double>::max() ? m_minimum : informationColorEngine()->colorRange()->minimumValue());
  auto max = (m_maximum != std::numeric_limits<double>::min() ? m_maximum : informationColorEngine()->colorRange()->maximumValue());

  ColorEngineRangeLimitsDialog dialog{min, max, m_key.value()};

  if(dialog.exec() == QDialog::Accepted)
  {
    m_minimum = (dialog.min() == informationColorEngine()->colorRange()->minimumValue() ? std::numeric_limits<double>::max() : dialog.min());
    m_maximum = (dialog.max() == informationColorEngine()->colorRange()->maximumValue() ? std::numeric_limits<double>::min() : dialog.max());

    m_needUpdate = true;

    updateRange(m_key);
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::createRepresentationFactory()
{
  auto rep2d = std::make_shared<InformationColorEngineWidgetRepresentation2D>();
  auto rep3d = std::make_shared<InformationColorEngineWidgetRepresentation3D>();

  connect(rep2d.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)),
          this,        SLOT(onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)));
  connect(rep3d.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation3DSPtr)),
          this,        SLOT(onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation3DSPtr)));

  m_repFactory = std::make_shared<TemporalPrototypes>(rep2d, rep3d, tr("InformationColorEngineWidget"));
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onRangeButtonClicked()
{
  auto range = informationColorEngine()->colorRange();
  auto widgetsVisible = (m_repFactory != nullptr);

  ColorEngineRangeDefinitionDialog dialog;
  dialog.setNumerical(m_properties.categories.isEmpty());
  dialog.setRangeColors(range->minimumColor().hue(), range->maximumColor().hue());
  dialog.setShowRangeInViews(widgetsVisible);
  dialog.setWidgetPosition(m_properties.position);
  dialog.setWidgetOrientation(m_properties.orientation);
  dialog.setTitlePosition(m_properties.textPosition);
  dialog.setNumberOfLabels(m_properties.numLabels);
  dialog.setBarRatio(m_properties.barRatio);
  dialog.setWidthRatio(m_properties.width);
  dialog.setHeightRatio(m_properties.height);
  dialog.setNumberOfDecimals(m_properties.decimals);

  connect(&dialog, SIGNAL(widgetsEnabled(int)),         this, SLOT(onWidgetsActivated(int)), Qt::DirectConnection);
  connect(&dialog, SIGNAL(widgetsPropertiesModified()), this, SLOT(onWidgetPropertiesModified()));

  if(dialog.exec() == QDialog::Accepted)
  {
    range->setMinimumColor(QColor::fromHsv(dialog.minimum(), 255, 255));
    range->setMaximumColor(QColor::fromHsv(dialog.maximum(), 255, 255));

    auto image = ColorBar::rangeImage(informationColorEngine()->colorRange(), Styles::mediumBarWidth()-2, 20-2);
    m_rangeButton->setIcon(QPixmap::fromImage(image));

    m_needUpdate = true;

    updateRange(m_key);
  }
  else
  {
    // user cancelled the dialog, restore previous settings.
    onWidgetsActivated(widgetsVisible ? Qt::Checked : Qt::Unchecked);
  }
}

//-----------------------------------------------------------------------------
InformationColorEngineWidgetRepresentationImplementation::InformationColorEngineWidgetRepresentationImplementation()
: m_active     {false}
, m_view       {nullptr}
, m_margin     {40}
, m_lut        {nullptr}
, m_scalarBar  {nullptr}
, m_legend     {nullptr}
{
}

//-----------------------------------------------------------------------------
InformationColorEngineWidgetRepresentationImplementation::~InformationColorEngineWidgetRepresentationImplementation()
{
  if(m_view)
  {
    baseUninitialize();
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentationImplementation::baseInitialize(RenderView* view)
{
  if(view && !m_view)
  {
    m_view = view;

    createActors();

    updateActors();
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentationImplementation::baseUninitialize()
{
  baseHide();

  m_view = nullptr;
  m_active = false;

  m_lut       = nullptr;
  m_scalarBar = nullptr;
  m_legend    = nullptr;
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentationImplementation::baseShow()
{
  if(m_view && !m_active)
  {
    if(!m_scalarBar && !m_legend) createActors();

    updateActors();

    if(m_scalarBar) m_view->addActor(m_scalarBar);
    if(m_legend)    m_view->addActor(m_legend);

    m_active = true;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentationImplementation::baseHide()
{
  if(m_view && m_active)
  {
    if(m_scalarBar) m_view->removeActor(m_scalarBar);
    if(m_legend)    m_view->removeActor(m_legend);

    m_active = false;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentationImplementation::baseDisplay(const GUI::Representations::FrameCSPtr& frame)
{
  /** empty */
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentationImplementation::createActors()
{
  if(m_view)
  {
    if(m_scalarBar)
    {
      if(m_active) m_view->removeActor(m_scalarBar);
      m_lut = nullptr;
      m_scalarBar = nullptr;
    }

    if(m_legend)
    {
      if(m_active) m_view->removeActor(m_legend);
      m_legend = nullptr;
    }

    if(m_properties.categories.isEmpty())
    {
      m_lut = vtkSmartPointer<vtkLookupTable>::New();
      m_lut->SetNumberOfColors(256);
      m_lut->SetIndexedLookup(false);

      m_scalarBar = vtkSmartPointer<vtkScalarBarActor>::New();
      m_scalarBar->SetLookupTable(m_lut);
      m_scalarBar->SetMaximumNumberOfColors(256);
      m_scalarBar->SetAnnotationTextScaling(false);
      m_scalarBar->SetDrawTickLabels(true);
      m_scalarBar->SetTitleRatio(0.5);
      m_scalarBar->SetBarRatio(0.25);
      m_scalarBar->SetDragable(false);
      m_scalarBar->SetPickable(false);
    }
    else
    {
      m_legend = vtkSmartPointer<vtkLegendBoxActor>::New();
      m_legend->SetDragable(false);
      m_legend->SetPickable(false);
      m_legend->SetBorder(false);
      m_legend->SetScalarVisibility(false);
      m_legend->SetLockBorder(true);
    }
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentationImplementation::updateActors()
{
  if(m_view)
  {
    if(m_properties.categories.isEmpty())
    {
      auto min       = m_properties.colors->minimumValue();
      auto max       = m_properties.colors->maximumValue();
      auto increment = (max-min)/256.;

      m_lut->SetTableRange(min, max);
      double i = min;
      for(int j = 0; j < 256; ++j, i += increment)
      {
        auto color = m_properties.colors->color(i);
        m_lut->SetTableValue(j, color.redF(), color.greenF(), color.blueF());
      }
      m_lut->Modified();

      m_scalarBar->SetTitle(m_properties.title.toStdString().c_str());
      m_scalarBar->SetOrientation(static_cast<int>(m_properties.orientation));
      m_scalarBar->SetNumberOfLabels(m_properties.numLabels);
      m_scalarBar->SetBarRatio(m_properties.barRatio);
      m_scalarBar->SetTextPosition(static_cast<int>(m_properties.textPosition));
      auto decimals = QString("%.%1f").arg(m_properties.decimals);
      m_scalarBar->SetLabelFormat(decimals.toStdString().c_str());
    }
    else
    {
      vtkSmartPointer<vtkPolyData> icon{nullptr};
      m_legend->SetNumberOfEntries(m_properties.categories.size());
      for(auto category: m_properties.categories)
      {
        auto pos = m_properties.categories.indexOf(category);
        auto color = m_properties.colors->color(pos);
        double rgb[3]{color.redF(), color.greenF(), color.blueF()};

        m_legend->SetEntry(pos, icon, category.toStdString().c_str(), rgb);
      }
      m_legend->Modified();
    }

    placeWidget();
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone)
{
  auto widget = std::dynamic_pointer_cast<GUI::Representations::Managers::TemporalRepresentation>(clone);
  if(widget)
  {
    connect(widget.get(), SIGNAL(destroyed(QObject *)), this, SLOT(onWidgetDestroyed(QObject *)));

    m_widgets << widget;

    auto base = std::dynamic_pointer_cast<InformationColorEngineWidgetRepresentationImplementation>(widget);
    if(base)
    {
      base->setProperties(m_properties);
    }
    else
    {
      qWarning() << "Couldn't cast to base" << __FILE__ << __LINE__;
    }
  }
  else
  {
    qWarning() << "Unable to identify widget 2d" << __FILE__ << __LINE__;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onWidgetCloned(GUI::Representations::Managers::TemporalRepresentation3DSPtr clone)
{
  auto widget = std::dynamic_pointer_cast<GUI::Representations::Managers::TemporalRepresentation>(clone);
  if(widget)
  {
    connect(widget.get(), SIGNAL(destroyed(QObject *)), this, SLOT(onWidgetDestroyed(QObject *)));

    m_widgets << widget;

    auto base = std::dynamic_pointer_cast<InformationColorEngineWidgetRepresentationImplementation>(widget);
    if(base)
    {
      base->setProperties(m_properties);
    }
    else
    {
      qWarning() << "Couldn't cast to base" << __FILE__ << __LINE__;
    }
  }
  else
  {
    qWarning() << "Unable to identify widget 3d" << __FILE__ << __LINE__;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onWidgetDestroyed(QObject* object)
{
  for(auto widget: m_widgets)
  {
    auto qtObject = qobject_cast<QObject *>(widget.get());
    if(qtObject == object)
    {
      disconnect(widget.get(), SIGNAL(destroyed(QObject *)), this, SLOT(onWidgetDestroyed(QObject *)));
      m_widgets.removeAll(widget);
      return;
    }
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentationImplementation::setProperties(const Properties& properties)
{
  m_properties = properties;

  if((m_scalarBar && !m_properties.categories.isEmpty()) || (m_legend && m_properties.categories.isEmpty()))
  {
    createActors();
    updateActors();
    if(m_active)
    {
      if(m_scalarBar) m_view->addActor(m_scalarBar);
      if(m_legend)    m_view->addActor(m_legend);
    }
  }
  else
  {
    if(m_scalarBar || m_legend) updateActors();
  }
}

//-----------------------------------------------------------------------------
const InformationColorEngineWidgetRepresentationImplementation::Properties InformationColorEngineWidgetRepresentationImplementation::getProperties() const
{
  return m_properties;
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentationImplementation::placeWidget()
{
  if(m_view)
  {
    auto windowSize = m_view->mainRenderer()->GetRenderWindow()->GetSize();
    auto width      = (windowSize[0] - 2 * m_margin) * m_properties.width;
    auto height     = (windowSize[1] - 2 * m_margin) * m_properties.height;

    int x, y;
    switch(m_properties.position)
    {
      case ColorEngineRangeDefinitionDialog::Position::BOTTOM_LEFT:
        x = m_margin;
        y = m_margin;
        break;
      case ColorEngineRangeDefinitionDialog::Position::BOTTOM_RIGHT:
        x = windowSize[0] - m_margin - width;
        y = m_margin;
        break;
      case ColorEngineRangeDefinitionDialog::Position::TOP_RIGHT:
        x = windowSize[0] - m_margin - width;
        y = windowSize[1] - m_margin - height;
        break;
      default:
      case ColorEngineRangeDefinitionDialog::Position::TOP_LEFT:
        x = m_margin;
        y = windowSize[1] - m_margin - height;
        break;
    }

    if(m_properties.categories.isEmpty())
    {
      m_scalarBar->SetMaximumWidthInPixels(width);
      m_scalarBar->SetMaximumHeightInPixels(height);
      m_scalarBar->SetWidth(width/windowSize[0]);
      m_scalarBar->SetHeight(height/windowSize[1]);
      m_scalarBar->SetDisplayPosition(x,y);
      m_scalarBar->Modified();
    }
    else
    {
      m_legend->GetPositionCoordinate()->SetCoordinateSystemToDisplay();
      m_legend->GetPositionCoordinate()->SetValue(x, y);
      m_legend->GetPosition2Coordinate()->SetCoordinateSystemToDisplay();
      m_legend->GetPosition2Coordinate()->SetValue(width, height);
      m_legend->Modified();
    }
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::removeAndDestroyRepresentationFactory()
{
  if(m_repFactory)
  {
    getViewState().removeTemporalRepresentations(m_repFactory);
    m_repFactory = nullptr;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onWidgetsActivated(int value)
{
  switch(value)
  {
    case Qt::Checked:
      if(!m_repFactory) createRepresentationFactory();
      getViewState().addTemporalRepresentations(m_repFactory);
      break;
    default:
    case Qt::Unchecked:
      if(m_repFactory) removeAndDestroyRepresentationFactory();
      break;
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineSwitch::onWidgetPropertiesModified()
{
  if(m_repFactory)
  {
    auto dialog = qobject_cast<ColorEngineRangeDefinitionDialog *>(sender());
    if(dialog)
    {
      m_properties.title        = m_key.value();
      m_properties.position     = dialog->getWidgetPosition();
      m_properties.orientation  = dialog->getWidgetOrientation();
      m_properties.textPosition = dialog->getTitlePosition();
      m_properties.numLabels    = dialog->getNumberOfLabels();
      m_properties.width        = dialog->getWidthRatio();
      m_properties.height       = dialog->getHeightRatio();
      m_properties.barRatio     = dialog->getBarRatio();
      m_properties.decimals     = dialog->getNumberOfDecimals();

      m_properties.colors->setMinimumColor(QColor::fromHsv(dialog->minimum(), 255, 255));
      m_properties.colors->setMaximumColor(QColor::fromHsv(dialog->maximum(), 255, 255));

      auto min = (m_minimum != std::numeric_limits<double>::max() ? m_minimum : informationColorEngine()->colorRange()->minimumValue());
      auto max = (m_maximum != std::numeric_limits<double>::min() ? m_maximum : informationColorEngine()->colorRange()->maximumValue());

      m_properties.colors->setMinimumValue(min);
      m_properties.colors->setMaximumValue(max);

      updateWidgetsSettings();
    }
    else
    {
      qWarning() << "Unable to cast to ColorEngineRangeDefinitionDialog" << __FILE__ << __LINE__;
    }
  }
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentation2D::initialize(RenderView* view)
{
  baseInitialize(view);

  if(view) connect(view, SIGNAL(viewResized(QSize)), this, SLOT(onViewResized()));
}

//-----------------------------------------------------------------------------
void InformationColorEngineWidgetRepresentation3D::initialize(RenderView* view)
{
  baseInitialize(view);

  if(view) connect(view, SIGNAL(viewResized(QSize)), this, SLOT(onViewResized()));
}
