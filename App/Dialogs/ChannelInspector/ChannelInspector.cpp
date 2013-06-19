/*
 * ChannelInspector.cpp
 *
 *  Created on: Dec 16, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include <GUI/QtWidget/SliceView.h>
#include <GUI/ViewManager.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/Channel.h>
#include <Core/Model/Segmentation.h>
#include <Core/EspinaTypes.h>
#include <Core/Filters/ChannelReader.h>
#include <Core/Extensions/EdgeDistances/AdaptiveEdges.h>
#include <Core/Extensions/EdgeDistances/EdgeDistance.h>

#include "ChannelInspector.h"

// Qt
#include <QSizePolicy>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDialog>
#include <QLocale>
#include <QColorDialog>
#include <qbitmap.h>
#include <QPainter>

// VTK
#include <vtkMath.h>
#include <vtkImageData.h>

// ITK
#include <itkChangeInformationImageFilter.h>

using namespace EspINA;

typedef itk::ChangeInformationImageFilter<itkVolumeType> ChangeImageInformationFilter;

//------------------------------------------------------------------------
ChannelInspector::ChannelInspector(Channel *channel, EspinaModel *model, QWidget *parent)
: QDialog(parent)
, m_spacingModified(false)
, m_edgesModified(false)
, m_channel(channel)
, m_viewManager(new ViewManager())
, m_model(model)
, m_view(new SliceView(m_model->factory(), m_viewManager, AXIAL))
{
  setupUi(this);

  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowTitle(QString("Channel Inspector - ") + channel->data().toString());

  /// PROPERTIES TAB
  connect(okCancelBox, SIGNAL(accepted()), this, SLOT(acceptedChanges()));
  connect(okCancelBox, SIGNAL(rejected()), this, SLOT(rejectedChanges()));

  connect(unitsBox, SIGNAL(currentIndexChanged(int)), this, SLOT(unitsChanged(int)));

  m_view->addChannel(channel);
  m_view->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_view->setParent(this);

  mainLayout->insertWidget(0, m_view, 1);

  double spacing[3];
  channel->volume()->spacing(spacing);

  // prefer dots instead of commas
  QLocale localeUSA = QLocale(QLocale::English, QLocale::UnitedStates);
  spacingXBox->setLocale(localeUSA);
  spacingYBox->setLocale(localeUSA);
  spacingZBox->setLocale(localeUSA);

  memcpy(m_spacing, spacing, sizeof(double)*3);
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

  vtkImageData *image = vtkImageData::SafeDownCast(m_channel->volume()->toVTK()->GetProducer()->GetOutputDataObject(0));
  image->GetScalarRange(m_range);

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
  Channel::ExtensionPtr edgesExtension = channel->extension(EspINA::AdaptiveEdgesID);
  AdaptiveEdges *adaptiveExtension = NULL;
  if (edgesExtension)
    adaptiveExtension = reinterpret_cast<AdaptiveEdges*>(edgesExtension);

  m_adaptiveEdgesEnabled = (edgesExtension != NULL) && adaptiveExtension->usesAdaptiveEdges();

  radioStackEdges->setChecked(!m_adaptiveEdgesEnabled);
  radioImageEdges->setChecked(m_adaptiveEdgesEnabled);
  colorBox->setEnabled(m_adaptiveEdgesEnabled);
  colorLabel->setEnabled(m_adaptiveEdgesEnabled);
  thresholdBox->setEnabled(m_adaptiveEdgesEnabled);
  thresholdLabel->setEnabled(m_adaptiveEdgesEnabled);

  connect(radioStackEdges, SIGNAL(toggled(bool)), this, SLOT(radioEdgesChanged(bool)));
  connect(radioImageEdges, SIGNAL(toggled(bool)), this, SLOT(radioEdgesChanged(bool)));

  m_backgroundColor = (edgesExtension == NULL) ? 0 : adaptiveExtension->backgroundColor();
  m_threshold = (edgesExtension == NULL) ? 50 : adaptiveExtension->threshold();
  colorBox->setValue(m_backgroundColor);
  thresholdBox->setValue(m_threshold);

  connect(colorBox, SIGNAL(valueChanged(int)), this, SLOT(changeEdgeDetectorBgColor(int)));
  connect(thresholdBox, SIGNAL(valueChanged(int)), this, SLOT(changeEdgeDetectorThreshold(int)));

  if (edgesExtension)
    changeEdgeDetectorBgColor(m_backgroundColor);

  tabWidget->setCurrentIndex(0);
}

//------------------------------------------------------------------------
ChannelInspector::~ChannelInspector()
{
  delete m_view;
  delete m_viewManager;
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

  itkVolumeType::SpacingType spacing;
  spacing[0] = spacingXBox->value()*pow(1000,unitsBox->currentIndex());
  spacing[1] = spacingYBox->value()*pow(1000,unitsBox->currentIndex());
  spacing[2] = spacingZBox->value()*pow(1000,unitsBox->currentIndex());

  QApplication::setOverrideCursor(Qt::WaitCursor);

  ChannelReader* reader = dynamic_cast<ChannelReader *>(m_channel->filter().get());
  Q_ASSERT(reader);
  reader->setSpacing(spacing);

  ChannelList channels;
  channels << m_channel;

  SegmentationList updatedSegmentations;
  foreach(ModelItemSPtr item, m_channel->relatedItems(EspINA::RELATION_OUT, Channel::LINK))
  {
    if (EspINA::SEGMENTATION == item->type())
    {
      SegmentationSPtr seg = segmentationPtr(item);
      SegmentationVolumeSPtr segVolume = segmentationVolume(seg->output());

      ChangeImageInformationFilter::Pointer changer = ChangeImageInformationFilter::New();
      changer->SetInput(segVolume->toITK().GetPointer());
      changer->ChangeSpacingOn();
      changer->SetOutputSpacing(spacing);
      changer->Update();

      segVolume->setVolume(changer->GetOutput());
      segVolume->toITK()->Update();
      segVolume->markAsModified(true);
      updatedSegmentations << seg.get();
    }
  }

  m_viewManager->updateSegmentationRepresentations(updatedSegmentations);
  m_viewManager->updateChannelRepresentations(channels);

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
  int rangediff = m_range[1]-m_range[0];
  double tick;

  if (QString("contrastSlider").compare(sender()->objectName()) == 0)
  {
    tick = rangediff / 255.0;
    contrastBox->blockSignals(true);
    contrastBox->setValue(vtkMath::Round(value*(100./255.)));
    contrastBox->blockSignals(false);
  }
  else
  {
    tick = rangediff / 100.0;
    contrastSlider->blockSignals(true);
    contrastSlider->setValue(vtkMath::Round(value*(255./100.)));
    contrastSlider->blockSignals(false);
  }
  m_channel->setContrast(((value * tick)/rangediff)+1);

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

  m_view->updateChannelRepresentation(m_channel);
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

  double spacing[3];
  m_channel->volume()->spacing(spacing);

  if (m_spacing[0] != spacing[0] || m_spacing[1] != spacing[1] || m_spacing[2] != spacing[2])
  {
    modified = true;
    itkVolumeType::SpacingType newSpacing;
    newSpacing[0] = m_spacing[0];
    newSpacing[1] = m_spacing[1];
    newSpacing[2] = m_spacing[2];

    /// WARNING: This won't work with preprocessing
    ChannelReader *reader = dynamic_cast<ChannelReader *>(m_channel->filter().get());
    Q_ASSERT(reader);
    reader->setSpacing(newSpacing);
    reader->update(0);

    SegmentationList updatedSegmentations;
    foreach(ModelItemSPtr item, m_channel->relatedItems(EspINA::RELATION_OUT, Channel::LINK))
    {
      if (EspINA::SEGMENTATION == item->type())
      {
        SegmentationSPtr seg = segmentationPtr(item);
        SegmentationVolumeSPtr segVolume = segmentationVolume(seg->output());
        ChangeImageInformationFilter::Pointer changer = ChangeImageInformationFilter::New();
        changer->SetInput(segVolume->toITK());
        changer->ChangeSpacingOn();
        changer->SetOutputSpacing(newSpacing);
        changer->Update();

        segVolume->setVolume(changer->GetOutput());
        segVolume->toITK()->Update();
        segVolume->markAsModified(true);
        updatedSegmentations << seg.get();
      }
    }

    m_view->updateSceneBounds();  // needed to update thumbnail values without triggering volume()->markAsModified()
    m_viewManager->updateSegmentationRepresentations(updatedSegmentations);
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
    m_viewManager->updateChannelRepresentations();
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

  m_edgesModified = (radioImageEdges->isChecked() != m_adaptiveEdgesEnabled) ||
                    (radioImageEdges->isChecked() && (m_backgroundColor != colorBox->value())) ||
                    (radioImageEdges->isChecked() && (m_threshold != thresholdBox->value()));
}

//------------------------------------------------------------------------
void ChannelInspector::changeEdgeDetectorBgColor(int value)
{
  bool enabled = (radioImageEdges->isChecked() != m_adaptiveEdgesEnabled) ||
                 (radioImageEdges->isChecked() && (m_backgroundColor != colorBox->value()));

  QPixmap image(":espina/edges-image.png");
  QPixmap bg(image.size());
  bg.fill(QColor(value, value, value));
  image.setMask(image.createMaskFromColor(Qt::black, Qt::MaskInColor));
  QPainter painter(&bg);
  painter.drawPixmap(0,0, image);

  adaptiveExample->setPixmap(bg);

  m_edgesModified = enabled;
}

//------------------------------------------------------------------------
void ChannelInspector::changeEdgeDetectorThreshold(int value)
{
  bool enabled = (radioImageEdges->isChecked() != m_adaptiveEdgesEnabled) ||
                 (radioImageEdges->isChecked() && (m_threshold != thresholdBox->value()));

  m_edgesModified = enabled;
}

//------------------------------------------------------------------------
void ChannelInspector::applyEdgesChanges()
{
  Channel::ExtensionPtr extension = m_channel->extension(EspINA::AdaptiveEdgesID);
  if (extension != NULL)
  {
    AdaptiveEdges *adaptiveEdges = reinterpret_cast<AdaptiveEdges *>(extension);
    adaptiveEdges->invalidate(m_channel);

    foreach(SegmentationSPtr seg, m_model->segmentations())
      if (seg->hasInformationExtension(EdgeDistanceID))
      {
        Segmentation::InformationExtension ext = seg->informationExtension(EdgeDistanceID);
        ext->invalidate(seg.get());

      }

    m_channel->deleteExtension(extension);
  }

  if (radioImageEdges->isChecked())
  {
    m_adaptiveEdgesEnabled = true;
    m_backgroundColor = colorBox->value();
    m_threshold = thresholdBox->value();

    m_channel->addExtension(new AdaptiveEdges(true, m_backgroundColor, m_threshold));
  }
  else
    m_adaptiveEdgesEnabled = false;

  m_edgesModified = false;
}
