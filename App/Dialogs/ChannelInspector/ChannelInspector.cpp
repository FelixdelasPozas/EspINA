/*
 * ChannelInspector.cpp
 *
 *  Created on: Dec 16, 2012
 *      Author: Felix de las Pozas Alvarez
 */

// ESPINA
#include <EspinaConfig.h>
#include "ChannelInspector.h"
#include <GUI/View/View2D.h>
#include <Support/ViewManager.h>
#include <GUI/Model/ModelAdapter.h>
#include <GUI/Model/ChannelAdapter.h>
#include <GUI/Model/SegmentationAdapter.h>
#include <Core/EspinaTypes.h>
#include <Core/Utils/NmVector3.h>
#include <Core/Analysis/Query.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>
#include <Extensions/EdgeDistances/EdgeDistance.h>
#include <Extensions/ExtensionUtils.h>
#include <GUI/Representations/Renderers/SliceRenderer.h>

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

typedef itk::ChangeInformationImageFilter<itkVolumeType> ChangeImageInformationFilter;

//------------------------------------------------------------------------
ChannelInspector::ChannelInspector(ChannelAdapterPtr channel, ModelAdapterSPtr model, SchedulerSPtr scheduler, QWidget *parent)
: QDialog(parent)
, m_spacingModified(false)
, m_edgesModified(false)
, m_channel(channel)
, m_model(model)
, m_scheduler(scheduler)
, m_view(new View2D(Plane::XY, this))
{
  setupUi(this);

  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowTitle(QString("Channel Inspector - ") + channel->data().toString());

  /// PROPERTIES TAB
  connect(okCancelBox, SIGNAL(accepted()), this, SLOT(acceptedChanges()));
  connect(okCancelBox, SIGNAL(rejected()), this, SLOT(rejectedChanges()));

  connect(unitsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(unitsChanged(int)));

  RendererSList renderers;
  renderers << RendererSPtr{new SliceRenderer()};
  m_view->setRenderers(renderers);
  m_view->add(channel);
  m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_view->setParent(this);

  mainLayout->insertWidget(0, m_view, 1);

  auto volume = volumetricData(channel->output());
  NmVector3 spacing = volume->spacing();

  // prefer dots instead of commas
  QLocale localeUSA = QLocale(QLocale::English, QLocale::UnitedStates);
  spacingXBox->setLocale(localeUSA);
  spacingYBox->setLocale(localeUSA);
  spacingZBox->setLocale(localeUSA);

  m_spacing = spacing;
  spacingXBox->setValue(spacing[0]);
  spacingYBox->setValue(spacing[1]);
  spacingZBox->setValue(spacing[2]);
  connect(spacingXBox, SIGNAL(valueChanged(double)), this, SLOT(spacingChanged(double)));
  connect(spacingYBox, SIGNAL(valueChanged(double)), this, SLOT(spacingChanged(double)));
  connect(spacingZBox, SIGNAL(valueChanged(double)), this, SLOT(spacingChanged(double)));

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
  connect(opacityCheck, SIGNAL(stateChanged(int)), this, SLOT(opacityCheckChanged(int)));
  connect(opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacityChanged(int)));

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

  connect(m_hueSelector, SIGNAL(newHsv(int,int,int)), this, SLOT(newHSV(int,int,int)));
  connect(hueBox, SIGNAL(valueChanged(int)), this, SLOT(newHSV(int)));
  connect(saturationSlider, SIGNAL(valueChanged(int)), this, SLOT(saturationChanged(int)));

  m_brightness = m_channel->brightness();
  m_contrast = m_channel->contrast();
  brightnessSlider->setValue(vtkMath::Round(m_channel->brightness() * 255.));
  brightnessBox->setValue(vtkMath::Round(m_channel->brightness() * 100.));
  contrastSlider->setValue((vtkMath::Round((m_channel->contrast() - 1.0) * 255.)));
  contrastBox->setValue(vtkMath::Round((m_channel->contrast() - 1.0) * 100.));
  connect(contrastSlider, SIGNAL(valueChanged(int)), this, SLOT(contrastChanged(int)));
  connect(contrastBox, SIGNAL(valueChanged(int)), this, SLOT(contrastChanged(int)));
  connect(brightnessSlider, SIGNAL(valueChanged(int)), this, SLOT(brightnessChanged(int)));
  connect(brightnessBox, SIGNAL(valueChanged(int)), this, SLOT(brightnessChanged(int)));

  // fix ruler initialization
  m_view->resetCamera();
  m_view->setRulerVisibility(true);
  m_view->updateView();

  /// EDGES TAB
  auto edgesExtension = retrieveOrCreateExtension<ChannelEdges>(channel);

  m_useDistanceToEdges = !edgesExtension->useDistanceToBounds();

  radioStackEdges->setChecked(!m_useDistanceToEdges);
  radioImageEdges->setChecked(m_useDistanceToEdges);
  colorBox->setEnabled(m_useDistanceToEdges);
  colorLabel->setEnabled(m_useDistanceToEdges);
  thresholdBox->setEnabled(m_useDistanceToEdges);
  thresholdLabel->setEnabled(m_useDistanceToEdges);

  connect(radioStackEdges, SIGNAL(toggled(bool)), this, SLOT(radioEdgesChanged(bool)));
  connect(radioImageEdges, SIGNAL(toggled(bool)), this, SLOT(radioEdgesChanged(bool)));

  m_backgroundColor = (edgesExtension == nullptr) ? 0 : edgesExtension->backgroundColor();
  m_threshold = (edgesExtension == nullptr) ? 50 : edgesExtension->threshold();
  colorBox->setValue(m_backgroundColor);
  thresholdBox->setValue(m_threshold);

  connect(colorBox, SIGNAL(valueChanged(int)), this, SLOT(changeEdgeDetectorBgColor(int)));
  connect(thresholdBox, SIGNAL(valueChanged(int)), this, SLOT(changeEdgeDetectorThreshold(int)));

  if (edgesExtension)
    changeEdgeDetectorBgColor(m_backgroundColor);

  tabWidget->setCurrentIndex(0);

#if USE_METADONA
  tabWidget->addTab(new MetadataViewer(channel, m_scheduler, this), tr("Metadata"));
#endif // USE_METADONA
}

//------------------------------------------------------------------------
ChannelInspector::~ChannelInspector()
{
  delete m_view;
}

//------------------------------------------------------------------------
void ChannelInspector::unitsChanged(int value)
{
  spacingXBox->setSuffix(unitsBox->currentText());
  spacingYBox->setSuffix(unitsBox->currentText());
  spacingZBox->setSuffix(unitsBox->currentText());
  spacingChanged();
}

//------------------------------------------------------------------------
void ChannelInspector::spacingChanged(double unused)
{
  m_spacingModified = true;
}

//------------------------------------------------------------------------
void ChannelInspector::changeSpacing()
{
  if (spacingXBox->value() == 0 || spacingYBox->value() == 0 || spacingZBox->value() == 0)
  {
    QMessageBox msgBox;
    msgBox.setText("The spacing specified is invalid and cannot be applied to the channel.");
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setStandardButtons(QMessageBox::Ok);
    QString detailedText = QString("Spacing was set to:\n  X: ") + QString().number(spacingXBox->value())
        + unitsBox->currentText() + QString("\n  Y: ")  + QString().number(spacingYBox->value())
        + unitsBox->currentText() + QString("\n  Z: ")  + QString().number(spacingZBox->value())
        + unitsBox->currentText();
    msgBox.setDetailedText (detailedText);
    msgBox.exec();

    return;
  }

  NmVector3 spacing;
  spacing[0] = spacingXBox->value()*pow(1000,unitsBox->currentIndex());
  spacing[1] = spacingYBox->value()*pow(1000,unitsBox->currentIndex());
  spacing[2] = spacingZBox->value()*pow(1000,unitsBox->currentIndex());

  QApplication::setOverrideCursor(Qt::WaitCursor);

  m_channel->output()->setSpacing(spacing);

  auto relatedItems = m_model->relatedItems(m_channel, RelationType::RELATION_OUT);
  for(auto item: relatedItems)
  {
    switch(item->type())
    {
      case ItemAdapter::Type::SEGMENTATION:
        {
          auto segAdapter = std::dynamic_pointer_cast<ViewItemAdapter>(item);
          Q_ASSERT(segAdapter);
          segAdapter->output()->setSpacing(spacing);
        }
        break;
      default:
        continue;
        break;
    }
  }

  m_view->updateSceneBounds();
  m_view->updateRepresentations();
  m_view->resetCamera();
  m_spacingModified = false;
  applyModifications();

  QApplication::restoreOverrideCursor();
}

//------------------------------------------------------------------------
void ChannelInspector::opacityCheckChanged(int value)
{
  opacityBox->setEnabled(!value);
  opacitySlider->setEnabled(!value);

  if (value)
    m_channel->setOpacity(-1.0);
  else
    m_channel->setOpacity(opacityBox->value()/100.);

  applyModifications();
}

//------------------------------------------------------------------------
void ChannelInspector::opacityChanged(int value)
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
  if (!this->isVisible())
    return;

  ChannelAdapterList channels;
  channels << m_channel;
  m_view->updateRepresentations(channels);
  m_view->updateView();
}

//------------------------------------------------------------------------
void ChannelInspector::acceptedChanges()
{
  if (hueBox->value() == -1)
    m_channel->setSaturation(0.0);

  if (m_spacingModified)
  {
    changeSpacing();
    emit spacingUpdated();
  }

  if (m_edgesModified)
    applyEdgesChanges();
}

//------------------------------------------------------------------------
void ChannelInspector::rejectedChanges()
{
  bool modified = false;

  auto volume = volumetricData(m_channel->output());
  NmVector3 spacing = volume->spacing();

  if (m_spacing[0] != spacing[0] || m_spacing[1] != spacing[1] || m_spacing[2] != spacing[2])
  {
    modified = true;
    m_channel->output()->setSpacing(m_spacing);

    auto relatedItems = m_model->relatedItems(m_channel, RelationType::RELATION_OUT);
    for(auto item: relatedItems)
    {
      switch(item->type())
      {
        case ItemAdapter::Type::SEGMENTATION:
          {
            auto segAdapter = std::dynamic_pointer_cast<ViewItemAdapter>(item);
            Q_ASSERT(segAdapter);
            segAdapter->output()->setSpacing(m_spacing);
          }
          break;
        default:
          continue;
          break;
      }
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
    m_view->updateSceneBounds();  // needed to update thumbnail values without triggering volume()->markAsModified()
    m_view->updateRepresentations();
  }
}

//------------------------------------------------------------------------
void ChannelInspector::closeEvent(QCloseEvent *event)
{
  rejectedChanges();
  QDialog::closeEvent(event);
}

//------------------------------------------------------------------------
void ChannelInspector::radioEdgesChanged(bool value)
{
  if (sender() == radioStackEdges)
    radioImageEdges->setChecked(!value);
  else
    radioStackEdges->setChecked(!value);

  colorLabel->setEnabled(radioImageEdges->isChecked());
  colorBox->setEnabled(radioImageEdges->isChecked());
  thresholdLabel->setEnabled(radioImageEdges->isChecked());
  thresholdBox->setEnabled(radioImageEdges->isChecked());

  changeEdgeDetectorBgColor(m_backgroundColor);

  m_edgesModified = (radioImageEdges->isChecked() != m_useDistanceToEdges) ||
                    (radioImageEdges->isChecked() && (m_backgroundColor != colorBox->value())) ||
                    (radioImageEdges->isChecked() && (m_threshold != thresholdBox->value()));
}

//------------------------------------------------------------------------
void ChannelInspector::changeEdgeDetectorBgColor(int value)
{
  bool enabled = (radioImageEdges->isChecked() != m_useDistanceToEdges) ||
                 (radioImageEdges->isChecked() && (m_backgroundColor != colorBox->value()));

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
  ChannelEdgesSPtr edgesExtension = retrieveOrCreateExtension<ChannelEdges>(m_channel);

  for (auto segmentation: m_model->segmentations())
  {
    if (segmentation->hasExtension(EdgeDistance::TYPE))
    {
      auto distanceExtension = retrieveExtension<EdgeDistance>(segmentation);
      Q_ASSERT(distanceExtension);
      distanceExtension->invalidate();
    }
  }


  if (radioImageEdges->isChecked())
  {
    m_useDistanceToEdges = true;
    m_backgroundColor = colorBox->value();
    m_threshold = thresholdBox->value();
  }
  else
    m_useDistanceToEdges = false;

  edgesExtension->setUseDistanceToBounds(!m_useDistanceToEdges);

  m_edgesModified = false;
}
