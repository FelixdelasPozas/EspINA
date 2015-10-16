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
#include "ChannelInspector.h"

#include <EspinaConfig.h>

#include <Core/Analysis/Channel.h>
#include <Core/Analysis/Query.h>
#include <Core/Utils/Vector3.hxx>
#include <Core/Utils/ListUtils.hxx>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <Extensions/ExtensionUtils.h>
#include <GUI/Dialogs/DefaultDialogs.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <GUI/Model/Utils/QueryAdapter.h>
#include <GUI/Representations/Pipelines/ChannelSlicePipeline.h>
#include <GUI/Representations/Pools/BufferedRepresentationPool.h>
#include <GUI/View/View2D.h>
#include <GUI/Widgets/PixelValueSelector.h>
#include <GUI/Widgets/Styles.h>
#include <Support/Representations/SliceManager.h>

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
using namespace ESPINA::Core::Utils;
using namespace ESPINA::GUI;
using namespace ESPINA::GUI::Widgets;
using namespace ESPINA::GUI::Widgets::Styles;

typedef itk::ChangeInformationImageFilter<itkVolumeType> ChangeImageInformationFilter;

//------------------------------------------------------------------------
ChannelInspector::ChannelInspector(ChannelAdapterSPtr channel, Support::Context &context)
: QDialog(DefaultDialogs::defaultParentWidget())
, WithContext(context)
, m_spacingModified   {false}
, m_edgesModified     {false}
, m_pixelSelector     {new PixelValueSelector(this)}
, m_channel           {channel}
, m_sources           {m_viewState}
, m_view              {new View2D(m_viewState, Plane::XY)}
{
  setupUi(this);

  setWindowTitle(tr("Channel Inspector - %1").arg(channel->data().toString()));

  /// PROPERTIES TAB
  connect(okCancelBox, SIGNAL(accepted()),
          this,        SLOT(onChangesAccepted()));
  connect(okCancelBox, SIGNAL(rejected()),
          this,        SLOT(onChangesRejected()));

  connect(unitsBox, SIGNAL(currentIndexChanged(int)),
          this,     SLOT(unitsChanged()));

  initPropertiesTab();

  /// EDGES TAB
  auto edgesExtension = retrieveOrCreateExtension<ChannelEdges>(channel->extensions());

  initPixelValueSelector();

  m_useDistanceToEdges = !edgesExtension->useDistanceToBounds();

  radioStackEdges->setChecked(!m_useDistanceToEdges);
  radioImageEdges->setChecked(m_useDistanceToEdges);
  colorLabel->setEnabled(m_useDistanceToEdges);
  m_pixelSelector->setEnabled(m_useDistanceToEdges);
  thresholdBox->setEnabled(m_useDistanceToEdges);
  thresholdLabel->setEnabled(m_useDistanceToEdges);

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

  tabWidget->setCurrentIndex(0);

#if USE_METADONA
  tabWidget->addTab(new MetadataViewer(channel, getScheduler(), this), tr("Metadata"));
#endif // USE_METADONA
}

//------------------------------------------------------------------------
ChannelInspector::~ChannelInspector()
{
}

//------------------------------------------------------------------------
void ChannelInspector::unitsChanged()
{
  spacingXBox->setSuffix(unitsBox->currentText());
  spacingYBox->setSuffix(unitsBox->currentText());
  spacingZBox->setSuffix(unitsBox->currentText());
  onSpacingChanged();
}

//------------------------------------------------------------------------
void ChannelInspector::onSpacingChanged()
{
  m_spacingModified = true;

  updateStackPreview();

  m_view->resetCamera();
  applyModifications();
}

//------------------------------------------------------------------------
void ChannelInspector::onOpacityCheckChanged(int value)
{
  opacityBox->setEnabled(!value);
  opacitySlider->setEnabled(!value);

  if (value)
  {
    auto opacity = 1.0/getModel()->channels().size();
    m_channel->setOpacity(opacity);
  }
  else
    m_channel->setOpacity(opacityBox->value()/100.);

  applyModifications();
}

//------------------------------------------------------------------------
void ChannelInspector::onOpacityChanged(int value)
{
  m_channel->setOpacity(value/100.);
  applyModifications();
}

//------------------------------------------------------------------------
void ChannelInspector::newHSV(int h, int s, int v)
{
  hueBox->setValue(h-1);

  if (h == 0)
  {
    saturationBox->setEnabled(false);
    saturationSlider->setEnabled(false);
  }
  else
  {
    if (0.0 == m_channel->saturation() && !saturationBox->isEnabled())
    {
      saturationBox->setValue(100);
      m_channel->setSaturation(1);
    }
    saturationBox->setEnabled(true);
    saturationSlider->setEnabled(true);
  }

  double value = ((h-1) == -1) ? -1 : ((h-1)/359.);
  m_channel->setHue(value);
  applyModifications();
}

//------------------------------------------------------------------------
void ChannelInspector::newHSV(int h)
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
  m_channel->setHue(value);
  applyModifications();
}

//------------------------------------------------------------------------
void ChannelInspector::saturationChanged(int value)
{
  m_channel->setSaturation(value/100.);
  applyModifications();
}

//------------------------------------------------------------------------
void ChannelInspector::contrastChanged(int value)
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
  m_channel->setContrast(((value * tick)/255.0)+1);

  applyModifications();
}

//------------------------------------------------------------------------
void ChannelInspector::brightnessChanged(int value)
{
  if (QString("brightnessSlider").compare(sender()->objectName()) == 0)
  {
    brightnessBox->blockSignals(true);
    brightnessBox->setValue(vtkMath::Round(value*(100./255.)));
    brightnessBox->blockSignals(false);
    m_channel->setBrightness(value/255.);
  }
  else
  {
    brightnessSlider->blockSignals(true);
    brightnessSlider->setValue(vtkMath::Round(value*(255./100.)));
    brightnessSlider->blockSignals(false);
    m_channel->setBrightness(value/100.);
  }

  applyModifications();
}

//------------------------------------------------------------------------
void ChannelInspector::applyModifications()
{
  if (isVisible())
  {
    updateSceneState(m_viewState, toViewItemSList(m_channel));
    invalidateChannelRepresentation();
  }
}

//------------------------------------------------------------------------
void ChannelInspector::invalidateChannelRepresentation()
{
  auto frame = m_viewState.createFrame();

  m_sources.updateRepresentation(toViewItemList(m_channel.get()), frame);
}

//------------------------------------------------------------------------
void ChannelInspector::onChangesAccepted()
{
  if (hueBox->value() == -1)
  {
    m_channel->setSaturation(0.0);
  }

  if (m_spacingModified)
  {
    changeStackSpacing();

    m_spacingModified = false;

    emit spacingUpdated();
  }

  if (m_edgesModified)
  {
    applyEdgesChanges();
  }

  updateSceneState(getViewState(), toViewItemSList(m_channel));
  getViewState().invalidateRepresentations(m_channel.get());
}

//------------------------------------------------------------------------
void ChannelInspector::onChangesRejected()
{
  bool modified = false;

  auto spacing = m_channel->output()->spacing();

  if (m_spacing[0] != spacing[0] || m_spacing[1] != spacing[1] || m_spacing[2] != spacing[2])
  {
    modified = true;
    m_channel->output()->setSpacing(m_spacing);

    for (auto segmentation : QueryAdapter::segmentationsOnChannelSample(m_channel))
    {
      segmentation->output()->setSpacing(m_spacing);
    }
  }

  if (m_opacity != m_channel->opacity())
  {
    modified = true;
    m_channel->setOpacity(m_opacity);
  }

  if (m_hue != m_channel->hue())
  {
    modified = true;
    m_channel->setHue(m_hue);
  }

  if (m_saturation != m_channel->saturation())
  {
    modified = true;
    m_channel->setSaturation(m_saturation);
  }

  if (m_brightness != m_channel->brightness())
  {
    modified = true;
    m_channel->setBrightness(m_brightness);
  }

  if (m_contrast != m_channel->contrast())
  {
    modified = true;
    m_channel->setContrast(m_contrast);
  }

  if (modified)
  {
    invalidateChannelRepresentation();
  }
}

//------------------------------------------------------------------------
void ChannelInspector::closeEvent(QCloseEvent *event)
{
  onChangesRejected();

  QDialog::closeEvent(event);
}

//------------------------------------------------------------------------
void ChannelInspector::radioEdgesChanged(bool value)
{
  if (sender() == radioStackEdges)
  {
    radioImageEdges->setChecked(!value);
  }
  else
  {
    radioStackEdges->setChecked(!value);
  }

  colorLabel->setEnabled(radioImageEdges->isChecked());
  m_pixelSelector->setEnabled(radioImageEdges->isChecked());
  thresholdLabel->setEnabled(radioImageEdges->isChecked());
  thresholdBox->setEnabled(radioImageEdges->isChecked());

  changeEdgeDetectorBgColor(m_backgroundColor);

  m_edgesModified = (radioImageEdges->isChecked() != m_useDistanceToEdges) ||
                    (radioImageEdges->isChecked() && (m_backgroundColor != m_pixelSelector->value())) ||
                    (radioImageEdges->isChecked() && (m_threshold != thresholdBox->value()));
}

//------------------------------------------------------------------------
void ChannelInspector::changeEdgeDetectorBgColor(int value)
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
void ChannelInspector::changeEdgeDetectorThreshold(int value)
{
  bool enabled = (radioImageEdges->isChecked() != m_useDistanceToEdges) ||
                 (radioImageEdges->isChecked() && (m_threshold != thresholdBox->value()));

  m_edgesModified = enabled;
}

//------------------------------------------------------------------------
void ChannelInspector::applyEdgesChanges()
{
  auto edgesExtension = retrieveOrCreateExtension<ChannelEdges>(m_channel->extensions());

  for (auto segmentation: getModel()->segmentations())
  {
    auto extensions = segmentation->readOnlyExtensions();

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
    m_backgroundColor = m_pixelSelector->value();
    m_threshold = thresholdBox->value();
  }
  else
  {
    m_useDistanceToEdges = false;
  }

  edgesExtension->setUseDistanceToBounds(!m_useDistanceToEdges);

  m_edgesModified = false;
}

//------------------------------------------------------------------------
void ChannelInspector::initPropertiesTab()
{
  initSliceView();

  initSpacingSettings();

  initOpacitySettings();

  initColorSettings();
}

//------------------------------------------------------------------------
void ChannelInspector::initSliceView()
{
  auto frame = m_viewState.createFrame();
  m_sources.addSource(toViewItemList(m_channel.get()), frame);

  auto pipelineXY = std::make_shared<ChannelSlicePipeline>(Plane::XY);
  auto poolXY     = std::make_shared<BufferedRepresentationPool>(ItemAdapter::Type::CHANNEL, Plane::XY, pipelineXY, getScheduler(), 10);

  poolXY->setPipelineSources(&m_sources);

  auto sliceManager = std::make_shared<SliceManager>(poolXY, RepresentationPoolSPtr(), RepresentationPoolSPtr());

  m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_view->setParent(this);
  m_view->setScaleVisibility(true);
  mainLayout->insertWidget(0, m_view.get(), 1);

  m_view->addRepresentationManager(sliceManager);

  sliceManager->show(sliceManager->lastFrame());

  updateSceneState(m_viewState, toViewItemSList(m_channel));
}

//------------------------------------------------------------------------
void ChannelInspector::initSpacingSettings()
{
  auto volume  = readLockVolume(m_channel->output());
  auto spacing = volume->bounds().spacing();

  // prefer dots instead of commas
  QLocale localeUSA = QLocale(QLocale::English, QLocale::UnitedStates);
  spacingXBox->setLocale(localeUSA);
  spacingYBox->setLocale(localeUSA);
  spacingZBox->setLocale(localeUSA);

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
}

//------------------------------------------------------------------------
void ChannelInspector::initOpacitySettings()
{
  m_opacity = m_channel->opacity();

  if (m_channel->opacity() == -1.0)
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
    opacitySlider->setValue(vtkMath::Round(m_channel->opacity() * 100.));
  }

  connect(opacityCheck,  SIGNAL(stateChanged(int)),
          this,          SLOT(onOpacityCheckChanged(int)));
  connect(opacitySlider, SIGNAL(valueChanged(int)),
          this,          SLOT(onOpacityChanged(int)));
}

//------------------------------------------------------------------------
void ChannelInspector::initColorSettings()
{
  m_hueSelector = new HueSelector();
  m_hueSelector->setFixedHeight(20);
  hueGroupBox->layout()->addWidget(m_hueSelector);

  m_hue = m_channel->hue();

  if (m_channel->hue() == -1.0)
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
    m_saturation = m_channel->saturation();
    hueBox->setValue(vtkMath::Round(m_channel->hue() * 359.));
    m_hueSelector->setHueValue(vtkMath::Round(m_channel->hue() * 359.));
    saturationBox->setValue(vtkMath::Round(m_channel->saturation() * 100.));
  }

  connect(m_hueSelector, SIGNAL(newHsv(int,int,int)),
          this,          SLOT(newHSV(int,int,int)));
  connect(hueBox, SIGNAL(valueChanged(int)),
          this,   SLOT(newHSV(int)));
  connect(saturationSlider, SIGNAL(valueChanged(int)),
          this,             SLOT(saturationChanged(int)));

  m_brightness = m_channel->brightness();
  m_contrast   = m_channel->contrast();

  brightnessSlider->setValue(vtkMath::Round(m_channel->brightness() * 255.));
  brightnessBox   ->setValue(vtkMath::Round(m_channel->brightness() * 100.));
  contrastSlider  ->setValue(vtkMath::Round((m_channel->contrast() - 1.0) * 255.));
  contrastBox     ->setValue(vtkMath::Round((m_channel->contrast() - 1.0) * 100.));

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
void ChannelInspector::updateStackPreview()
{
  // TODO: Make a copy of the channel and update only the preview, not
  //       the channel itself
  m_channel->output()->setSpacing(currentSpacing());
}

//------------------------------------------------------------------------
void ChannelInspector::changeStackSpacing()
{
  WaitingCursor cursor;

  getModel()->changeSpacing(m_channel, currentSpacing());

  auto segmentations = toRawList<ViewItemAdapter>(QueryAdapter::segmentationsOnChannelSample(m_channel));

  //FIXME getContext().representationInvalidator().invalidateRepresentations(segmentations);
}

//------------------------------------------------------------------------
void ChannelInspector::initPixelValueSelector()
{
  m_pixelSelector->setFixedHeight(24);

  auto layout = new QHBoxLayout();
  layout->setMargin(0);
  layout->addWidget(m_pixelSelector);
  m_colorFrame->setLayout(layout);
}

//------------------------------------------------------------------------
NmVector3 ChannelInspector::currentSpacing() const
{
  NmVector3 spacing;

  spacing[0] = spacingXBox->value()*pow(1000,unitsBox->currentIndex());
  spacing[1] = spacingYBox->value()*pow(1000,unitsBox->currentIndex());
  spacing[2] = spacingZBox->value()*pow(1000,unitsBox->currentIndex());

  return spacing;
}
