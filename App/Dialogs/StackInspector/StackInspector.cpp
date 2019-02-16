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
#include "StackInspector.h"

#include <EspinaConfig.h>

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Query.h>
#include <Core/Analysis/Filters/VolumetricStreamReader.h>
#include <Core/Utils/ListUtils.hxx>
#include <Core/Utils/SignalBlocker.h>
#include <Core/Utils/Vector3.hxx>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/SLIC/StackSLIC.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Representations/Managers/TemporalManager.h>
#include <GUI/Representations/Pipelines/ChannelSlicePipeline.h>
#include <GUI/Representations/Pools/BufferedRepresentationPool.h>
#include <GUI/View/View2D.h>
#include <GUI/Widgets/PixelValueSelector.h>
#include <GUI/Widgets/Styles.h>
#include <Support/Representations/SliceManager.h>
#include "SLICRepresentation2D.h"

#if USE_METADONA
  #include <Producer.h>
  #include <IRODS_Storage.h>
  #include <Utils.h>
  #include <Support/Metadona/Coordinator.h>
  #include <Support/Metadona/MetadataViewer.h>
  #include <Support/Metadona/StorageFactory.h>
#endif

// Qt
#include <QSizePolicy>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDialog>
#include <QLocale>
#include <QColorDialog>
#include <QBitmap>
#include <QPainter>

// VTK
#include <vtkMath.h>
#include <vtkImageData.h>

// ITK
#include <itkChangeInformationImageFilter.h>
#include <itkStatisticsImageFilter.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Representations::Managers;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Widgets::Styles;

typedef itk::ChangeInformationImageFilter<itkVolumeType> ChangeImageInformationFilter;

//------------------------------------------------------------------------
StackInspector::StackInspector(ChannelAdapterSPtr channel, Support::Context &context)
: QDialog(DefaultDialogs::defaultParentWidget(), Qt::WindowFlags{Qt::WindowMinMaxButtonsHint|Qt::WindowCloseButtonHint})
, WithContext(context)
, m_spacingModified   {false}
, m_edgesModified     {false}
, m_pixelSelector     {new PixelValueSelector(this)}
, m_stack           {channel}
, m_sources           {m_viewState}
, m_view              {new View2D(m_viewState, Plane::XY)}
{
  setupUi(this);

  setWindowTitle(tr("Stack Inspector - %1").arg(channel->data().toString()));

  /// PROPERTIES TAB
  connect(okCancelBox, SIGNAL(accepted()),
          this,        SLOT(onChangesAccepted()));

  connect(okCancelBox, SIGNAL(rejected()),
          this,        SLOT(onChangesRejected()));

  connect(unitsBox, SIGNAL(currentIndexChanged(int)),
          this,     SLOT(unitsChanged()));

  connect(dataStreaming, SIGNAL(stateChanged(int)),
          this,          SLOT(onStreamingChanged(int)));

  initPropertiesTab();

  /// EDGES TAB
  auto edgesExtension = retrieveOrCreateStackExtension<ChannelEdges>(channel, context.factory());

  initPixelValueSelector();

  m_useDistanceToEdges = !edgesExtension->useDistanceToBounds();

  radioStackEdges->setChecked(!m_useDistanceToEdges);
  radioImageEdges->setChecked(m_useDistanceToEdges);
  colorLabel->setEnabled(m_useDistanceToEdges);
  m_pixelSelector->setEnabled(m_useDistanceToEdges);
  thresholdBox->setEnabled(m_useDistanceToEdges);
  thresholdLabel->setEnabled(m_useDistanceToEdges);
  computeOptimal->setChecked(false);

  connect(computeOptimal, SIGNAL(stateChanged(int)), this, SLOT(onOptimalStateChanged(int)));

  connect(radioStackEdges, SIGNAL(toggled(bool)), this, SLOT(radioEdgesChanged(bool)));
  connect(radioImageEdges, SIGNAL(toggled(bool)), this, SLOT(radioEdgesChanged(bool)));

  m_backgroundColor = (edgesExtension == nullptr) ? 0 : edgesExtension->backgroundColor();
  m_threshold = (edgesExtension == nullptr) ? 50 : edgesExtension->threshold();
  m_pixelSelector->setValue(m_backgroundColor);
  thresholdBox->setValue(m_threshold);

  connect(m_pixelSelector, SIGNAL(newValue(int)), this, SLOT(changeEdgeDetectorBgColor(int)));
  connect(thresholdBox, SIGNAL(valueChanged(int)), this, SLOT(changeEdgeDetectorThreshold(int)));

  if (edgesExtension)
  {
    changeEdgeDetectorBgColor(m_backgroundColor);
  }


  //// TODO: Create extension on demand
  //auto extensions = channel->readonleyextension();
  //if(extension->hasExtension(StackSLIC::TYPE))
  ////

  /// SLIC TAB
  //TODO: Load opacity/useColors from extension
  int opacity = 30;
  bool useColors = true;
  slicPreviewColorsCheck->setChecked(useColors);
  slicPreviewOpacitySlider->setValue(opacity);
  auto slicExtension = retrieveOrCreateStackExtension<StackSLIC>(channel, context.factory());

  connect(slicActionButton, SIGNAL(released()), this, SLOT(onSLICActionButtonPressed()));
  connect(this, SIGNAL(computeSLIC(unsigned char, unsigned char, Extensions::StackSLIC::SLICVariant, unsigned int, double)), slicExtension.get(), SLOT(onComputeSLIC(unsigned char, unsigned char, Extensions::StackSLIC::SLICVariant, unsigned int, double)));
  connect(this, SIGNAL(SLICAborted()), slicExtension.get(), SLOT(onAbortSLIC()));
  connect(slicExtension.get(), SIGNAL(computeFinished()), this, SLOT(onSLICComputed()));

  auto representation2d = std::make_shared<SLICRepresentation2D>(slicExtension, opacity/100.0, useColors);
  connect(representation2d.get(), SIGNAL(cloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)), this, SLOT(onSLICRepresentationCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr)));
  m_slicRepresentation = std::make_shared<TemporalPrototypes>(representation2d, TemporalRepresentation3DSPtr(), "SLIC Representation");

  slicPreviewSVCountLabel->setText(QString("%1").arg(slicExtension->getSupervoxelCount()));
  if(slicExtension->isRunning())
  {
    slicPreviewStatusLabel->setText("Computing...");
    slicProgressBar->setEnabled(true);
  }
  else
  {
    slicProgressBar->setEnabled(false);
    if (slicExtension->isComputed())
    {
      slicPreviewStatusLabel->setText("Computed");
    }
    else
    {
      slicPreviewStatusLabel->setText("Not computed");
    }
  }

  spatialDistanceBox->setValue((int) slicExtension->getSupervoxelSize());
  colorDistanceBox->setValue((int) slicExtension->getColorWeight());
  maxIterationsBox->setValue((int) slicExtension->getIterations());
  toleranceBox->setValue((double) slicExtension->getTolerance());

  switch(slicExtension->getVariant())
  {
    case StackSLIC::SLICVariant::SLICO:
      slicoRadio->setChecked(true);
      break;
    case StackSLIC::SLICVariant::ASLIC:
      aslicRadio->setChecked(true);
      break;
    default:
      slicRadio->setChecked(true);
      break;
  }

  connect(slicExtension.get(), SIGNAL(progress(int)), slicProgressBar, SLOT(setValue(int)));

  tabWidget->setCurrentIndex(0);

#if USE_METADONA
  tabWidget->addTab(new MetadataViewer(channel.get(), getScheduler(), this), tr("Metadata"));
#endif // USE_METADONA

  connect(tabWidget, SIGNAL(currentChanged(int)),
          this,      SLOT(onCurrentTabChanged(int)));
}

//------------------------------------------------------------------------
void StackInspector::unitsChanged()
{
  spacingXBox->setSuffix(unitsBox->currentText());
  spacingYBox->setSuffix(unitsBox->currentText());
  spacingZBox->setSuffix(unitsBox->currentText());
  onSpacingChanged();
}

//------------------------------------------------------------------------
void StackInspector::onSpacingChanged()
{
  m_spacingModified = true;

  updateStackPreview();

  m_view->resetCamera();
  applyModifications();
}

//------------------------------------------------------------------------
void StackInspector::onOpacityCheckChanged(int value)
{
  opacityBox->setEnabled(!value);
  opacitySlider->setEnabled(!value);

  if (value)
  {
    auto opacity = 1.0/getModel()->channels().size();
    m_stack->setOpacity(opacity);
  }
  else
  {
    m_stack->setOpacity(opacityBox->value()/100.);
  }

  applyModifications();
}

//------------------------------------------------------------------------
void StackInspector::onOpacityChanged(int value)
{
  m_stack->setOpacity(value/100.);
  applyModifications();
}

//------------------------------------------------------------------------
void StackInspector::newHSV(int h, int s, int v)
{
  hueBox->setValue(h-1);

  if (h == 0)
  {
    saturationBox->setEnabled(false);
    saturationSlider->setEnabled(false);
  }
  else
  {
    if (0.0 == m_stack->saturation() && !saturationBox->isEnabled())
    {
      saturationBox->setValue(100);
      m_stack->setSaturation(1);
    }
    saturationBox->setEnabled(true);
    saturationSlider->setEnabled(true);
  }

  double value = ((h-1) == -1) ? -1 : ((h-1)/359.);
  m_stack->setHue(value);
  applyModifications();
}

//------------------------------------------------------------------------
void StackInspector::newHSV(int h)
{
  m_hueSelector->setHueValue(h+1);

  if (h+1 == 0)
  {
    saturationBox->setEnabled(false);
    saturationSlider->setEnabled(false);
  }
  else
  {
    saturationBox->setEnabled(true);
    saturationSlider->setEnabled(true);
  }

  double value = (h == -1) ? -1 : (h/359.);
  m_stack->setHue(value);
  applyModifications();
}

//------------------------------------------------------------------------
void StackInspector::saturationChanged(int value)
{
  m_stack->setSaturation(value/100.);
  applyModifications();
}

//------------------------------------------------------------------------
void StackInspector::contrastChanged(int value)
{
  double tick;

  if (QString("contrastSlider").compare(sender()->objectName()) == 0)
  {
    tick = 1.0;
    contrastBox->blockSignals(true);
    contrastBox->setValue(vtkMath::Round(value*(100./255.)));
    contrastBox->blockSignals(false);
  }
  else
  {
    tick = 255.0 / 100.0;
    contrastSlider->blockSignals(true);
    contrastSlider->setValue(vtkMath::Round(value*(255./100.)));
    contrastSlider->blockSignals(false);
  }
  m_stack->setContrast(((value * tick)/255.0)+1);

  applyModifications();
}

//------------------------------------------------------------------------
void StackInspector::brightnessChanged(int value)
{
  if (QString("brightnessSlider").compare(sender()->objectName()) == 0)
  {
    brightnessBox->blockSignals(true);
    brightnessBox->setValue(vtkMath::Round(value*(100./255.)));
    brightnessBox->blockSignals(false);
    m_stack->setBrightness(value/255.);
  }
  else
  {
    brightnessSlider->blockSignals(true);
    brightnessSlider->setValue(vtkMath::Round(value*(255./100.)));
    brightnessSlider->blockSignals(false);
    m_stack->setBrightness(value/100.);
  }

  applyModifications();
}

//------------------------------------------------------------------------
void StackInspector::applyModifications()
{
  if (isVisible())
  {
    auto crosshair = m_viewState.crosshair();

    if(m_spacingModified)
    {
      auto oldSpacing = m_stack->output()->spacing();
      const NmVector3 newSpacing{spacingXBox->value(), spacingYBox->value(), spacingZBox->value()};
      for(auto i: {0,1,2})
      {
        crosshair[i] = crosshair[i] * (newSpacing[i]/oldSpacing[i]);
      }
    }

    updateSceneState(crosshair, m_viewState, toViewItemSList(m_stack));
    invalidateStackRepresentation();
  }
}

//------------------------------------------------------------------------
void StackInspector::invalidateStackRepresentation()
{
  auto frame = m_viewState.createFrame();

  m_sources.updateRepresentation(toViewItemList(m_stack.get()), frame);
}

//------------------------------------------------------------------------
void StackInspector::onChangesAccepted()
{
  if (hueBox->value() == -1)
  {
    m_stack->setSaturation(0.0);
  }

  if (m_spacingModified)
  {
    changeStackSpacing();

    emit spacingUpdated();
  }

  if (m_edgesModified)
  {
    applyEdgesChanges();
  }

  auto crosshair = getViewState().crosshair();

  if(m_spacingModified)
  {
    auto oldSpacing = m_stack->output()->spacing();
    const NmVector3 newSpacing{spacingXBox->value(), spacingYBox->value(), spacingZBox->value()};
    for(auto i: {0,1,2})
    {
      crosshair[i] = crosshair[i] * (newSpacing[i]/oldSpacing[i]);
    }
  }

  updateSceneState(crosshair, getViewState(), toViewItemSList(m_stack));
  m_stack->invalidateRepresentations();
}

//------------------------------------------------------------------------
void StackInspector::onChangesRejected()
{
  bool modified = false;

  auto spacing = m_stack->output()->spacing();

  if (m_spacing[0] != spacing[0] || m_spacing[1] != spacing[1] || m_spacing[2] != spacing[2])
  {
    modified = true;
    m_stack->output()->setSpacing(m_spacing);

    for (auto segmentation : QueryAdapter::segmentationsOnChannelSample(m_stack))
    {
      segmentation->output()->setSpacing(m_spacing);
    }
  }

  if (m_opacity != m_stack->opacity())
  {
    modified = true;
    m_stack->setOpacity(m_opacity);
  }

  if (m_hue != m_stack->hue())
  {
    modified = true;
    m_stack->setHue(m_hue);
  }

  if (m_saturation != m_stack->saturation())
  {
    modified = true;
    m_stack->setSaturation(m_saturation);
  }

  if (m_brightness != m_stack->brightness())
  {
    modified = true;
    m_stack->setBrightness(m_brightness);
  }

  if (m_contrast != m_stack->contrast())
  {
    modified = true;
    m_stack->setContrast(m_contrast);
  }

  if (modified)
  {
    invalidateStackRepresentation();
  }
}

//------------------------------------------------------------------------
void StackInspector::closeEvent(QCloseEvent *event)
{
  onChangesRejected();

  QDialog::closeEvent(event);
}

//------------------------------------------------------------------------
void StackInspector::radioEdgesChanged(bool value)
{
  if (sender() == radioStackEdges)
  {
    radioImageEdges->setChecked(!value);
  }
  else
  {
    radioStackEdges->setChecked(!value);
  }

  auto enabled = radioImageEdges->isChecked() && !computeOptimal->isChecked();

  colorLabel->setEnabled(enabled);
  m_pixelSelector->setEnabled(enabled);
  thresholdLabel->setEnabled(enabled);
  thresholdBox->setEnabled(enabled);
  computeOptimal->setEnabled(radioImageEdges->isChecked());

  changeEdgeDetectorBgColor(m_backgroundColor);

  m_edgesModified = (radioImageEdges->isChecked() != m_useDistanceToEdges) ||
                    (radioImageEdges->isChecked() && (m_backgroundColor != m_pixelSelector->value())) ||
                    (radioImageEdges->isChecked() && (m_threshold != thresholdBox->value())) ||
                    (radioImageEdges->isChecked() && computeOptimal->isChecked());
}

//------------------------------------------------------------------------
void StackInspector::changeEdgeDetectorBgColor(int value)
{
  bool enabled = (radioImageEdges->isChecked() != m_useDistanceToEdges) ||
                 (radioImageEdges->isChecked() && (m_backgroundColor != m_pixelSelector->value()));

  QColor color;
  if (value != -1)
  {
    color.setRgb(value, value, value);
    m_backgroundColor = value;
  }
  else
  {
    color = QColor(0,0,0);
    m_backgroundColor = 0;
  }

  QPixmap image(":espina/edges-image.png");
  QPixmap bg(image.size());
  bg.fill(color);
  image.setMask(image.createMaskFromColor(Qt::black, Qt::MaskInColor));

  QPainter painter(&bg);
  painter.drawPixmap(0,0, image);

  adaptiveExample->setPixmap(bg);

  m_edgesModified = enabled;
}

//------------------------------------------------------------------------
void StackInspector::changeEdgeDetectorThreshold(int value)
{
  bool enabled = (radioImageEdges->isChecked() != m_useDistanceToEdges) ||
                 (radioImageEdges->isChecked() && (m_threshold != thresholdBox->value()));

  m_edgesModified = enabled;
}

//------------------------------------------------------------------------
void StackInspector::applyEdgesChanges()
{
  auto edgesExtension = retrieveExtension<ChannelEdges>(m_stack->extensions());

  for (auto segmentation: getModel()->segmentations())
  {
    auto extensions = segmentation->extensions();

    if (extensions->hasExtension(EdgeDistance::TYPE))
    {
      auto distanceExtension = retrieveExtension<EdgeDistance>(extensions);
      Q_ASSERT(distanceExtension);
      distanceExtension->invalidate();
    }
  }

  if (radioImageEdges->isChecked())
  {
    m_useDistanceToEdges = true;

    if(computeOptimal->isChecked())
    {
      m_backgroundColor    = -1;
      m_threshold          = -1;
    }
    else
    {
      m_backgroundColor    = m_pixelSelector->value();
      m_threshold          = thresholdBox->value();
    }
  }
  else
  {
    m_useDistanceToEdges = false;
  }

  edgesExtension->invalidate();
  edgesExtension->setAnalisysValues(!m_useDistanceToEdges, m_backgroundColor, m_threshold);

  m_edgesModified = false;
}

//------------------------------------------------------------------------
void StackInspector::initPropertiesTab()
{
  initSliceView();

  initSpacingSettings();

  initOpacitySettings();

  initColorSettings();

  initMiscSettings();
}

//------------------------------------------------------------------------
void StackInspector::initSliceView()
{
  updateSceneState(NmVector3{0,0,0}, m_viewState, toViewItemSList(m_stack));

  auto frame = m_viewState.createFrame();
  m_sources.addSource(toViewItemList(m_stack.get()), frame);

  auto pipelineXY = std::make_shared<ChannelSlicePipeline>(Plane::XY);
  auto poolXY     = std::make_shared<BufferedRepresentationPool>(ItemAdapter::Type::CHANNEL, Plane::XY, pipelineXY, getScheduler(), 10);

  poolXY->setPipelineSources(&m_sources);

  auto sliceManager = std::make_shared<SliceManager>(poolXY, RepresentationPoolSPtr(), RepresentationPoolSPtr());

  m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_view->setParent(this);
  m_view->setScaleVisibility(true);
  mainLayout->insertWidget(0, m_view.get(), 1);

  m_view->addRepresentationManager(sliceManager);
  sliceManager->show(m_viewState.createFrame());
}

//------------------------------------------------------------------------
void StackInspector::initSpacingSettings()
{
  auto volume  = readLockVolume(m_stack->output());
  auto spacing = volume->bounds().spacing();

  // prefer dots instead of commas
  QLocale localeUSA = QLocale(QLocale::English, QLocale::UnitedStates);
  spacingXBox->setLocale(localeUSA);
  spacingXBox->setToolTip(QObject::tr("Spacing value for X coordinate"));
  spacingYBox->setLocale(localeUSA);
  spacingYBox->setToolTip(QObject::tr("Spacing value for Y coordinate"));
  spacingZBox->setLocale(localeUSA);
  spacingZBox->setToolTip(QObject::tr("Spacing value for Z coordinate"));

  spacingXBox->setMinimum(1);
  spacingYBox->setMinimum(1);
  spacingZBox->setMinimum(1);

  m_spacing = spacing;
  spacingXBox->setValue(spacing[0]);
  spacingYBox->setValue(spacing[1]);
  spacingZBox->setValue(spacing[2]);

  connect(spacingXBox, SIGNAL(valueChanged(double)),
          this,        SLOT(onSpacingChanged()));
  connect(spacingYBox, SIGNAL(valueChanged(double)),
          this,        SLOT(onSpacingChanged()));
  connect(spacingZBox, SIGNAL(valueChanged(double)),
          this,        SLOT(onSpacingChanged()));

  // TODO: this is temporary, user should be able to change stack spacing even if the stack has segmentations.
  if(!QueryAdapter::segmentationsOnChannel(m_stack).isEmpty())
  {
    spacingXBox->setEnabled(false);
    spacingXBox->setToolTip(QObject::tr("Cannot change spacing for X coordinate, stack has segmentations."));
    spacingYBox->setEnabled(false);
    spacingYBox->setToolTip(QObject::tr("Cannot change spacing for Y coordinate, stack has segmentations."));
    spacingZBox->setEnabled(false);
    spacingZBox->setToolTip(QObject::tr("Cannot change spacing for Z coordinate, stack has segmentations."));

    unitsBox->setEnabled(false);
    unitsBox->setToolTip(QObject::tr("Cannot change spacing units, stack has segmentations."));
  }
}

//------------------------------------------------------------------------
void StackInspector::initOpacitySettings()
{
  m_opacity = m_stack->opacity();

  if (m_stack->opacity() == -1.0)
  {
    opacityBox->setEnabled(false);
    opacityCheck->setChecked(true);
    opacitySlider->setEnabled(false);
    opacitySlider->setValue(100);
  }
  else
  {
    opacityBox->setEnabled(true);
    opacityCheck->setChecked(false);
    opacitySlider->setValue(vtkMath::Round(m_stack->opacity() * 100.));
  }

  connect(opacityCheck,  SIGNAL(stateChanged(int)),
          this,          SLOT(onOpacityCheckChanged(int)));
  connect(opacitySlider, SIGNAL(valueChanged(int)),
          this,          SLOT(onOpacityChanged(int)));
}

//------------------------------------------------------------------------
void StackInspector::initColorSettings()
{
  m_hueSelector = new HueSelector();
  m_hueSelector->setFixedHeight(20);
  hueGroupBox->layout()->addWidget(m_hueSelector);

  m_hue = m_stack->hue();

  if (m_stack->hue() == -1.0)
  {
    m_saturation = 0.0;
    hueBox->setValue(-1);
    m_hueSelector->setHueValue(0);
    saturationBox->setValue(0);
    saturationBox->setEnabled(false);
    saturationSlider->setEnabled(false);
  }
  else
  {
    m_saturation = m_stack->saturation();
    hueBox->setValue(vtkMath::Round(m_stack->hue() * 359.));
    m_hueSelector->setHueValue(vtkMath::Round(m_stack->hue() * 359.));
    saturationBox->setValue(vtkMath::Round(m_stack->saturation() * 100.));
  }

  connect(m_hueSelector, SIGNAL(newHsv(int,int,int)),
          this,          SLOT(newHSV(int,int,int)));
  connect(hueBox, SIGNAL(valueChanged(int)),
          this,   SLOT(newHSV(int)));
  connect(saturationSlider, SIGNAL(valueChanged(int)),
          this,             SLOT(saturationChanged(int)));

  m_brightness = m_stack->brightness();
  m_contrast   = m_stack->contrast();

  brightnessSlider->setValue(vtkMath::Round(m_stack->brightness() * 255.));
  brightnessBox   ->setValue(vtkMath::Round(m_stack->brightness() * 100.));
  contrastSlider  ->setValue(vtkMath::Round((m_stack->contrast() - 1.0) * 255.));
  contrastBox     ->setValue(vtkMath::Round((m_stack->contrast() - 1.0) * 100.));

  connect(contrastSlider,   SIGNAL(valueChanged(int)),
          this,             SLOT(contrastChanged(int)));
  connect(contrastBox,      SIGNAL(valueChanged(int)),
          this,             SLOT(contrastChanged(int)));
  connect(brightnessSlider, SIGNAL(valueChanged(int)),
          this,             SLOT(brightnessChanged(int)));
  connect(brightnessBox,    SIGNAL(valueChanged(int)),
          this,             SLOT(brightnessChanged(int)));
}

//------------------------------------------------------------------------
void StackInspector::updateStackPreview()
{
  // TODO: Make a copy of the stack and update only the preview, not
  //       the stack itself
  m_stack->output()->setSpacing(currentSpacing());
}

//------------------------------------------------------------------------
void StackInspector::changeStackSpacing()
{
  WaitingCursor cursor;

  getModel()->changeSpacing(m_stack, currentSpacing());

  auto segmentations = toRawList<ViewItemAdapter>(QueryAdapter::segmentationsOnChannelSample(m_stack));

  getViewState().invalidateRepresentations(segmentations);
}

//------------------------------------------------------------------------
void StackInspector::initPixelValueSelector()
{
  m_pixelSelector->setFixedHeight(24);

  auto layout = new QHBoxLayout();
  layout->setMargin(0);
  layout->addWidget(m_pixelSelector);
  m_colorFrame->setLayout(layout);
}

//------------------------------------------------------------------------
void StackInspector::onCurrentTabChanged(int index)
{
  switch(index)
  {
    case 0:
      if(mainLayout->indexOf(m_view.get()) == -1)
      {
        m_viewState.removeTemporalRepresentations(m_slicRepresentation);
        slicTabLayout->removeWidget(m_view.get());
        mainLayout->insertWidget(0, m_view.get(), 1);
      }
      break;
    case 2:
      if(slicTabLayout->indexOf(m_view.get()) == -1)
      {
        m_viewState.addTemporalRepresentations(m_slicRepresentation);
        mainLayout->removeWidget(m_view.get());
        slicTabLayout->insertWidget(0, m_view.get(), 1);
      }
      break;
    default:
      // nothing
      break;
  }
}

//------------------------------------------------------------------------
void StackInspector::onSLICComputed()
{
  qDebug() << "called finished computing SLIC";

  slicActionButton->setText("Compute");
  slicProgressBar->setValue(0);
  slicProgressBar->setEnabled(false);
  slicPreviewStatusLabel->setText("Computed");

  auto slicExtension = retrieveOrCreateStackExtension<StackSLIC>(m_stack, this->getContext().factory());

  slicPreviewSVCountLabel->setText(QString("%1").arg(slicExtension->getSupervoxelCount()));

  if(m_view)
  {
    // force a refresh.
    m_stack->invalidateRepresentations();
  }
}

//------------------------------------------------------------------------
void StackInspector::onSLICRepresentationCloned(GUI::Representations::Managers::TemporalRepresentation2DSPtr clone)
{
  auto slicExtension = retrieveOrCreateStackExtension<StackSLIC>(m_stack, getFactory());

  connect(slicExtension.get(), SIGNAL(progress(int)), clone.get(), SLOT(setSLICComputationProgress(int)));
  connect(slicPreviewOpacitySlider, SIGNAL(valueChanged(int)), clone.get(), SLOT(opacityChanged(int)));
  connect(slicPreviewColorsCheck, SIGNAL(stateChanged(int)), clone.get(), SLOT(colorModeCheckChanged(int)));
}

//------------------------------------------------------------------------
NmVector3 StackInspector::currentSpacing() const
{
  NmVector3 spacing;

  spacing[0] = spacingXBox->value()*pow(1000,unitsBox->currentIndex());
  spacing[1] = spacingYBox->value()*pow(1000,unitsBox->currentIndex());
  spacing[2] = spacingZBox->value()*pow(1000,unitsBox->currentIndex());

  return spacing;
}

//------------------------------------------------------------------------
void StackInspector::onOptimalStateChanged(int unused)
{
  auto state = !computeOptimal->isChecked();

  colorLabel->setEnabled(state);
  m_pixelSelector->setEnabled(state);
  thresholdBox->setEnabled(state);
  thresholdLabel->setEnabled(state);

  m_edgesModified |= !state;
}

//------------------------------------------------------------------------
void StackInspector::onStreamingChanged(int state)
{
  auto value = (state == Qt::Checked);
  auto filter = std::dynamic_pointer_cast<Core::VolumetricStreamReader>(m_stack->filter());

  if(value != filter->streamingEnabled())
  {
    auto output = m_stack->output();
    SignalBlocker<OutputSPtr> block(output, false);
    filter->setStreaming(value);

    Task::submit(filter);
  }
}

//------------------------------------------------------------------------
void StackInspector::initMiscSettings()
{
  auto filter = std::dynamic_pointer_cast<Core::VolumetricStreamReader>(m_stack->filter());
  if(filter)
  {
    dataStreaming->setChecked(filter->streamingEnabled());
  }
  else
  {
    m_miscBox->setEnabled(false);
    dataStreaming->setChecked(false);
  }
}

//------------------------------------------------------------------------
void StackInspector::computeSLIC()
{
  slicActionButton->setText("Abort");
  slicProgressBar->setValue(0);
  slicProgressBar->setEnabled(true);
  slicPreviewStatusLabel->setText("Computing...");

  auto variant = Extensions::StackSLIC::SLICVariant::SLIC;
  char spatial_distance = 10;
  char color_distance = 20;
  int iterations = 10;
  double tolerance = 0;
  //Get parameters and start task
  if(slicoRadio->isChecked())
    variant = Extensions::StackSLIC::SLICVariant::SLICO;
  else if(aslicRadio->isChecked())
    variant = Extensions::StackSLIC::SLICVariant::ASLIC;
  spatial_distance = spatialDistanceBox->value();
  color_distance = colorDistanceBox->value();
  iterations = maxIterationsBox->value();
  tolerance = toleranceBox->value();

  emit computeSLIC(spatial_distance, color_distance, variant, iterations, tolerance);
}

//------------------------------------------------------------------------
void StackInspector::abortSLIC()
{
  slicActionButton->setText("Compute");
  slicProgressBar->setValue(0);
  slicProgressBar->setEnabled(false);
  slicPreviewStatusLabel->setText("Aborted");

  emit SLICAborted();
}

//------------------------------------------------------------------------
void StackInspector::onSLICActionButtonPressed()
{
  auto button = qobject_cast<QPushButton *>(sender());
  if(button)
  {
    if(button->text() == "Compute")
    {
      computeSLIC();
    }
    else
    {
      abortSLIC();
    }
  }
}
