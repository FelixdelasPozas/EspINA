/*
 * ChannelInspector.cpp
 *
 *  Created on: Dec 16, 2012
 *      Author: Félix de las Pozas Álvarez
 */

// EspINA
#include <GUI/QtWidget/SliceView.h>
#include <GUI/ViewManager.h>
#include <Core/Model/Channel.h>
#include <Core/Model/Segmentation.h>
#include <Core/EspinaTypes.h>
#include <Filters/ChannelReader.h>

#include "ChannelInspector.h"

// Qt
#include <QSizePolicy>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDialog>
#include <QLocale>

// VTK
#include <vtkMath.h>

using namespace EspINA;

//------------------------------------------------------------------------
ChannelInspector::ChannelInspector(Channel *channel, QWidget *parent)
: QDialog(parent)
, m_spacingModified(false)
, m_channel(channel)
, m_viewManager(new ViewManager())
, m_view(new SliceView(m_viewManager, AXIAL))
{
  setupUi(this);

  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowTitle(QString("Channel Inspector - ") + channel->data().toString());

  connect(okCancelBox, SIGNAL(accepted()), this, SLOT(acceptedChanges()));
  connect(okCancelBox, SIGNAL(rejected()), this, SLOT(rejectedChanges()));
  
  connect(applyButton, SIGNAL(clicked()), this, SLOT(changeSpacing()));
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
  applyButton->setEnabled(true);
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

  /// WARNING: This won't work with preprocessing
  ChannelReader *reader = dynamic_cast<ChannelReader *>(m_channel->filter().data());
  Q_ASSERT(reader);
  reader->setSpacing(spacing);
  reader->update(0);

  foreach(ModelItemSPtr item, m_channel->relatedItems(EspINA::OUT, Channel::LINK))
  {
    if (EspINA::SEGMENTATION == item->type())
    {
      SegmentationSPtr seg = segmentationPtr(item);
      double oldSpacing[3];
      seg->volume()->spacing(oldSpacing);
      seg->volume()->toITK()->SetSpacing(spacing);
      itkVolumeType::PointType origin = seg->volume()->toITK()->GetOrigin();
      for (int i=0; i < 3; i++)
        origin[i] = origin[i]/oldSpacing[i]*spacing[i];
      seg->volume()->toITK()->SetOrigin(origin);
      seg->volume()->update();
    }
  }

  m_channel->volume()->update();
  m_view->updateSceneBounds();  // needed to update thumbnail values without triggering volume()->markAsModified()
  m_view->resetCamera();
  m_spacingModified = false;
  applyButton->setEnabled(false);
  applyModifications();
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

  m_view->updateChannel(m_channel);
  m_view->updateView();
}

//------------------------------------------------------------------------
void ChannelInspector::acceptedChanges()
{
  if (hueBox->value() == -1)
    m_channel->setSaturation(0.0);

  if (m_spacingModified)
  {
    QMessageBox msgBox;
    msgBox.setText("The spacing has been modified but the changes have not been applied to the channel.");
    msgBox.setInformativeText("Do you want to apply your changes?");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setStandardButtons(QMessageBox::Apply | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Apply);
    QString detailedText = QString("Spacing changed to:\n  X: ") + QString().number(spacingXBox->value())
        + unitsBox->currentText() + QString("\n  Y: ")  + QString().number(spacingYBox->value())
        + unitsBox->currentText() + QString("\n  Z: ")  + QString().number(spacingZBox->value())
        + unitsBox->currentText();
    msgBox.setDetailedText (detailedText);
    int ret = msgBox.exec();

    switch(ret)
    {
      case QMessageBox::Apply:
        changeSpacing();
        emit spacingUpdated();
        break;
      default:
        break;
    }
  }

  double spacing[3];
  m_channel->volume()->spacing(spacing);
  if (m_spacing[0] != spacing[0] || m_spacing[1] != spacing[1] || m_spacing[2] != m_spacing[2])
    emit spacingUpdated();

  m_channel->volume()->update();
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
    ChannelReader *reader = dynamic_cast<ChannelReader *>(m_channel->filter().data());
    Q_ASSERT(reader);
    reader->setSpacing(newSpacing);
    reader->update(0);

    foreach(ModelItemSPtr item, m_channel->relatedItems(EspINA::OUT, Channel::LINK))
    {
      if (EspINA::SEGMENTATION == item->type())
      {
        SegmentationSPtr seg = segmentationPtr(item);
        double oldSpacing[3];
        seg->volume()->spacing(oldSpacing);
        seg->volume()->toITK()->SetSpacing(newSpacing);
        itkVolumeType::PointType origin = seg->volume()->toITK()->GetOrigin();
        for (int i=0; i < 3; i++)
        origin[i] = origin[i]/oldSpacing[i]*newSpacing[i];
        seg->volume()->toITK()->SetOrigin(origin);
        seg->volume()->update();
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
    m_channel->volume()->update();
  }
}

//------------------------------------------------------------------------
void ChannelInspector::closeEvent(QCloseEvent *event)
{
  rejectedChanges();
  QDialog::closeEvent(event);
}
