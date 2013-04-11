/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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
#include "SeedGrowSegmentationTool.h"
#include <Toolbars/Segmentation/ThresholdAction.h>
#include <Toolbars/Segmentation/DefaultVOIAction.h>
#include <Toolbars/Segmentation/SeedGrowSegmentationSettings.h>
#include <FilterInspectors/SeedGrowSegmentation/SGSFilterInspector.h>
#include <Undo/SeedGrowSegmentationCommand.h>
#include <Core/Model/Channel.h>
#include <Core/Model/EspinaModel.h>
#include <Core/Model/EspinaFactory.h>
#include <Core/Model/Sample.h>
#include <Core/Model/Segmentation.h>
#include <GUI/QtWidget/EspinaRenderView.h>
#include <GUI/ViewManager.h>
#include <Core/Model/PickableItem.h>
#include <Core/Model/Taxonomy.h>
#include <Core/EspinaSettings.h>
#include <GUI/Pickers/PixelSelector.h>
#include <Filters/SeedGrowSegmentationFilter.h>

// Qt
#include <QApplication>
#include <QWheelEvent>
#include <QMessageBox>
#include <QSettings>

// VTK
#include <vtkImageActor.h>
#include <vtkLookupTable.h>
#include <vtkImageMapper3D.h>
#include <vtkImageResliceToColors.h>
#include <vtkMatrix4x4.h>
#include <vtkMath.h>


using namespace EspINA;

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::SeedGrowSegmentationTool(EspinaModel *model,
                                                   QUndoStack  *undoStack,
                                                   ViewManager *viewManager,
                                                   ThresholdAction  *th,
                                                   DefaultVOIAction *voi,
                                                   SeedGrowSegmentationSettings *settings,
                                                   IPickerSPtr picker)
: m_model(model)
, m_undoStack(undoStack)
, m_viewManager(viewManager)
, m_settings(settings)
, m_defaultVOI(voi)
, m_threshold(th)
, m_picker(NULL)
, m_inUse(false)
, m_enabled(true)
, m_validPos(false)
, connectFilter(NULL)
, m_actor(NULL)
, m_viewOfPreview(NULL)
{
  Q_ASSERT(m_threshold);
  setChannelPicker(picker);
}

//-----------------------------------------------------------------------------
QCursor SeedGrowSegmentationTool::cursor() const
{
  QCursor cursor(Qt::ArrowCursor);

  if (m_picker && m_inUse)
  {
    if (m_validPos)
      cursor = m_picker->cursor();
    else
      cursor = QCursor(Qt::ForbiddenCursor);
  }

  return cursor;
}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationTool::filterEvent(QEvent* e, EspinaRenderView *view)
{
  if (!m_enabled)
    return false;

  if (e->type() == QEvent::FocusOut)
  {
    removePreview(view);
    return false;
  }

  if (e->type() == QEvent::KeyRelease)
  {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Shift)
      removePreview(view);
  }
  else
    if (e->type() == QEvent::KeyPress)
    {
      QKeyEvent *ke = static_cast<QKeyEvent *>(e);
      if (ke->key() == Qt::Key_Shift)
        addPreview(view);
    }
    else
      if (e->type() == QEvent::Wheel)
      {
        QWheelEvent* we = dynamic_cast<QWheelEvent*>(e);
        if (we->modifiers() == Qt::CTRL)
        {
          int numSteps = we->delta() / 8 / 15; //Refer to QWheelEvent doc.
          m_threshold->setLowerThreshold(m_threshold->lowerThreshold() + numSteps); //Using stepBy highlight the input text

          if (we->modifiers() == Qt::ShiftModifier)
            addPreview(view);

          return true;
        }
        else
            removePreview(view);
      }
      else
        if (e->type() == QEvent::MouseMove)
        {
          QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);

          if (m_viewManager->voi())
          {
            IVOI::Region currentVOI = m_viewManager->voiRegion();
            double cursorPos[3];
            view->worldCoordinates(me->pos(), cursorPos);

            m_validPos = currentVOI[0] <= cursorPos[0] && cursorPos[0] <= currentVOI[1] && currentVOI[2] <= cursorPos[1]
                && cursorPos[1] <= currentVOI[3] && currentVOI[4] <= cursorPos[2] && cursorPos[2] <= currentVOI[5];
          }
          else
            m_validPos = true;

          if (me->modifiers() == Qt::ShiftModifier)
            addPreview(view);
          else
            removePreview(view);
        }
        else
          if (e->type() == QEvent::MouseButtonPress)
          {
            QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
            if (me->modifiers() != Qt::CTRL && m_picker && m_validPos)
            {
              removePreview(view);
              return m_picker->filterEvent(e, view);
            }
          }

  return false;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setEnabled(bool enable)
{
  if (m_enabled != enable)
    m_enabled = enable;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setInUse(bool value)
{
  if (m_inUse != value)
  {
    m_inUse = value;
    if (!m_inUse || !m_viewManager->activeTaxonomy())
      emit segmentationStopped();
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setChannelPicker(IPickerSPtr picker)
{
  if (picker == m_picker)
    return;

  if (m_picker)
  {
    disconnect(m_picker.data(), SIGNAL(itemsPicked(ISelector::PickList)),
               this, SLOT(startSegmentation(ISelector::PickList)));
  }

  m_picker = picker;

  if (m_picker)
  {
    m_picker->setPickable(ISelector::CHANNEL);
    m_picker->setPickable(ISelector::SEGMENTATION, false);
    connect(m_picker.data(), SIGNAL(itemsPicked(ISelector::PickList)),
            this, SLOT(startSegmentation(ISelector::PickList)));
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::startSegmentation(ISelector::PickList pickedItems)
{
  if (pickedItems.size() != 1)
    return;

  ISelector::PickedItem element = pickedItems.first();
  PickableItemPtr input = element.second;

  Q_ASSERT(element.first->GetNumberOfPoints() == 1); // with one pixel
  Nm seedPoint[3];
  element.first->GetPoint(0, seedPoint);

  Q_ASSERT(EspINA::CHANNEL == input->type());
  ChannelPtr channel = m_viewManager->activeChannel();

  if (!channel)
    return;
  itkVolumeType::IndexType seed = channel->volume()->index(seedPoint[0], seedPoint[1], seedPoint[2]);
  if (seed[0] < 0 || seed[1] < 0 || seed[2] < 0)
    return;

  double spacing[3];
  channel->volume()->spacing(spacing);

  TaxonomyElementPtr tax = m_viewManager->activeTaxonomy();
  Q_ASSERT(tax);

  Nm voiBounds[6];
  IVOI::Region currentVOI = m_viewManager->voiRegion();
  if (currentVOI)
  {
    memcpy(voiBounds, currentVOI, 6*sizeof(double));
  }
  else if (m_defaultVOI->useDefaultVOI())
  {
    voiBounds[0] = seed[0]*spacing[0];
    voiBounds[1] = seed[0]*spacing[0];
    voiBounds[2] = seed[1]*spacing[1];
    voiBounds[3] = seed[1]*spacing[1];
    voiBounds[4] = seed[2]*spacing[2];
    voiBounds[5] = seed[2]*spacing[2];

    if (!tax->properties().contains(TaxonomyElement::X_DIM) ||
        !tax->properties().contains(TaxonomyElement::X_DIM) ||
        !tax->properties().contains(TaxonomyElement::X_DIM))
    {
      tax->addProperty(TaxonomyElement::X_DIM, QVariant(m_settings->xSize()));
      tax->addProperty(TaxonomyElement::Y_DIM, QVariant(m_settings->ySize()));
      tax->addProperty(TaxonomyElement::Z_DIM, QVariant(m_settings->zSize()));
    }

    QVariant xTaxSize = tax->property(TaxonomyElement::X_DIM);
    QVariant yTaxSize = tax->property(TaxonomyElement::Y_DIM);
    QVariant zTaxSize = tax->property(TaxonomyElement::Z_DIM);

    Nm xSize, ySize, zSize;

    if (m_settings->taxonomicalVOI() && xTaxSize.isValid() && yTaxSize.isValid() && zTaxSize.isValid())
    {
      xSize = xTaxSize.toDouble();
      ySize = yTaxSize.toDouble();
      zSize = zTaxSize.toDouble();
    }
    else
    {
      xSize = m_settings->xSize();
      ySize = m_settings->ySize();
      zSize = m_settings->zSize();
    }

    voiBounds[0] -= xSize;
    voiBounds[1] += xSize;
    voiBounds[2] -= ySize;
    voiBounds[3] += ySize;
    voiBounds[4] -= zSize;
    voiBounds[5] += zSize;

  } else
  {
    channel->volume()->bounds(voiBounds);
  }

  int voiExtent[6];
  for (int i=0; i<6; i++)
    voiExtent[i] = vtkMath::Round(voiBounds[i] / spacing[i/2]);

  Q_ASSERT(m_threshold->isSymmetrical());
  if (m_threshold->isSymmetrical())
  {
    Q_ASSERT(m_threshold->lowerThreshold() == m_threshold->upperThreshold());
    Q_ASSERT(m_threshold->lowerThreshold() >= 0);
    Q_ASSERT(m_threshold->lowerThreshold() <= 255);
  }

  if (voiExtent[0] <= seed[0] && seed[0] <= voiExtent[1] &&
    voiExtent[2] <= seed[1] && seed[1] <= voiExtent[3] &&
    voiExtent[4] <= seed[2] && seed[2] <= voiExtent[5])
  {
    SegmentationSList createdSegmentations;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    m_undoStack->beginMacro(tr("Seed Grow Segmentation"));
    m_undoStack->push(new SeedGrowSegmentationCommand(channel,
                                                      seed,
                                                      voiExtent,
                                                      m_threshold->lowerThreshold(),
                                                      m_threshold->upperThreshold(),
                                                      m_settings->closing(),
                                                      m_viewManager->activeTaxonomy(),
                                                      m_model,
                                                      m_viewManager,
                                                      createdSegmentations));
    m_model->emitSegmentationAdded(createdSegmentations);
    m_undoStack->endMacro();
    QApplication::restoreOverrideCursor();
  }
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::removePreview(EspinaRenderView *view)
{
  if (m_actor == NULL)
    return;

  m_viewOfPreview->removeActor(m_actor);
  connectFilter = NULL;
  i2v = NULL;
  m_actor = NULL;
  m_viewOfPreview->updateView();
  m_viewOfPreview = NULL;
}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::addPreview(EspinaRenderView *view)
{
  ISelector::DisplayRegionList regions;
  QPolygon singlePixel;

  int xPos, yPos;
  view->eventPosition(xPos, yPos);

  singlePixel << QPoint(xPos,yPos);
  regions << singlePixel;

  QSet<QString> filter;
  filter << ISelector::CHANNEL;

  ISelector::PickList pickList = view->pick(filter, regions);

  if (pickList.empty() || (pickList.first().second->type() != EspINA::CHANNEL))
  {
    removePreview(view);
    return;
  }

  PixelSelector *selector = dynamic_cast<PixelSelector*>(m_picker.data());
  if (!selector)
    return;

  double *point = selector->getPickPoint(view);
  if (point == NULL)
  {
    delete[] point;
    removePreview(view);
    return;
  }

  if (view != m_viewOfPreview)
    removePreview(view);

  EspinaVolume::Pointer pickedChannel = pickList.first().second->volume();
  double spacing[3];
  pickedChannel->spacing(spacing);

  itkVolumeType::IndexType seed;
  seed[0] = point[0]/spacing[0];
  seed[1] = point[1]/spacing[1];
  seed[2] = point[2]/spacing[2];

  itkVolumeType::IndexType index;
  itkVolumeType::SizeType size;

  double bounds[6];
  int extent[6];
  view->previewBounds(bounds);
  if (!m_viewManager->voi())
  {
    extent[0] = bounds[0]/spacing[0];
    extent[1] = bounds[1]/spacing[0];
    extent[2] = bounds[2]/spacing[1];
    extent[3] = bounds[3]/spacing[1];
    extent[4] = bounds[4]/spacing[2];
    extent[5] = bounds[5]/spacing[2];

    if (m_defaultVOI->useDefaultVOI())
    {
      int voiExtent[6];
      TaxonomyElementPtr tax = m_viewManager->activeTaxonomy();
      Q_ASSERT(tax);

      if (!tax->properties().contains(TaxonomyElement::X_DIM) ||
          !tax->properties().contains(TaxonomyElement::X_DIM) ||
          !tax->properties().contains(TaxonomyElement::X_DIM))
      {
        tax->addProperty(TaxonomyElement::X_DIM, QVariant(m_settings->xSize()));
        tax->addProperty(TaxonomyElement::Y_DIM, QVariant(m_settings->ySize()));
        tax->addProperty(TaxonomyElement::Z_DIM, QVariant(m_settings->zSize()));
      }

      QVariant xTaxSize = tax->property(TaxonomyElement::X_DIM);
      QVariant yTaxSize = tax->property(TaxonomyElement::Y_DIM);
      QVariant zTaxSize = tax->property(TaxonomyElement::Z_DIM);

      if (m_settings->taxonomicalVOI() && xTaxSize.isValid() && yTaxSize.isValid() && zTaxSize.isValid())
      {
        voiExtent[0] = seed[0] - vtkMath::Round(xTaxSize.toDouble()/spacing[0]);
        voiExtent[1] = seed[0] + vtkMath::Round(xTaxSize.toDouble()/spacing[0]);
        voiExtent[2] = seed[1] - vtkMath::Round(yTaxSize.toDouble()/spacing[1]);
        voiExtent[3] = seed[1] + vtkMath::Round(yTaxSize.toDouble()/spacing[1]);
        voiExtent[4] = seed[2] - vtkMath::Round(zTaxSize.toDouble()/spacing[2]);
        voiExtent[5] = seed[2] + vtkMath::Round(zTaxSize.toDouble()/spacing[2]);
      }
      else
      {
        voiExtent[0] = seed[0] - vtkMath::Round(m_settings->xSize()/spacing[0]);
        voiExtent[1] = seed[0] + vtkMath::Round(m_settings->xSize()/spacing[0]);
        voiExtent[2] = seed[1] - vtkMath::Round(m_settings->ySize()/spacing[1]);
        voiExtent[3] = seed[1] + vtkMath::Round(m_settings->ySize()/spacing[1]);
        voiExtent[4] = seed[2] - vtkMath::Round(m_settings->zSize()/spacing[2]);
        voiExtent[5] = seed[2] + vtkMath::Round(m_settings->zSize()/spacing[2]);
      }

      extent[0] = (voiExtent[0] > extent[0]) ? voiExtent[0] : extent[0];
      extent[1] = (voiExtent[1] < extent[1]) ? voiExtent[1] : extent[1];
      extent[2] = (voiExtent[2] > extent[2]) ? voiExtent[2] : extent[2];
      extent[3] = (voiExtent[3] < extent[3]) ? voiExtent[3] : extent[3];
      extent[4] = (voiExtent[4] > extent[4]) ? voiExtent[4] : extent[4];
      extent[5] = (voiExtent[5] < extent[5]) ? voiExtent[5] : extent[5];
    }
  }
  else
  {
    if (!m_validPos)
    {
      removePreview(view);
      return;
    }

    IVOI::Region currentVOI = m_viewManager->voiRegion();

    extent[0] = ((bounds[0] < currentVOI[0]) ? currentVOI[0]/spacing[0] : bounds[0]/spacing[0]);
    extent[1] = ((bounds[1] > currentVOI[1]) ? currentVOI[1]/spacing[0] : bounds[1]/spacing[0]);
    extent[2] = ((bounds[2] < currentVOI[2]) ? currentVOI[2]/spacing[1] : bounds[2]/spacing[1]);
    extent[3] = ((bounds[3] > currentVOI[3]) ? currentVOI[3]/spacing[1] : bounds[3]/spacing[1]);
    extent[4] = ((bounds[4] < currentVOI[4]) ? currentVOI[4]/spacing[2] : bounds[4]/spacing[2]);
    extent[5] = ((bounds[5] > currentVOI[5]) ? currentVOI[5]/spacing[2] : bounds[5]/spacing[2]);
  }

  index[0] = extent[0];
  index[1] = extent[2];
  index[2] = extent[4];
  size[0]  = extent[1]-extent[0] + 1;
  size[1]  = extent[3]-extent[2] + 1;
  size[2]  = extent[5]-extent[4] + 1;

  itkVolumeType::RegionType region(index, size);

  typedef itk::ExtractImageFilter<itkVolumeType, itkVolumeType> ExtractType;
  ExtractType::Pointer extract = ExtractType::New();
  extract->SetInPlace(false);
  extract->ReleaseDataFlagOff();
  extract->SetInput(pickedChannel->toITK());
  extract->SetExtractionRegion(region);
  extract->Update();

  double seedValue = static_cast<int>(extract->GetOutput()->GetPixel(seed));

  if (m_actor == NULL)
  {
    connectFilter = ConnectedThresholdFilterType::New();
    connectFilter->SetConnectivity(itk::ConnectedThresholdImageFilter<itkVolumeType, itkVolumeType>::FullConnectivity);
    connectFilter->ReleaseDataFlagOff();
    connectFilter->SetReplaceValue(1);
  }
  connectFilter->SetInput(extract->GetOutput());
  connectFilter->SetLower(std::max(seedValue - m_threshold->lowerThreshold(), 0.0));
  connectFilter->SetUpper(std::min(seedValue + m_threshold->upperThreshold(), 255.0));
  connectFilter->SetSeed(seed);
  connectFilter->Update();

  if (m_actor == NULL)
  {
    i2v = itk2vtkFilterType::New();
    i2v->ReleaseDataFlagOff();
    i2v->SetInput(connectFilter->GetOutput());
  }
  i2v->Update();

  if (m_actor != NULL)
  {
    view->updateView();
    return;
  }

  QColor color = m_viewManager->activeTaxonomy()->color();

  vtkSmartPointer<vtkLookupTable> lut = vtkSmartPointer<vtkLookupTable>::New();
  lut->Allocate();
  lut->SetNumberOfTableValues(2);
  lut->Build();
  lut->SetTableValue(0, 0.0, 0.0, 0.0, 0.0);
  lut->SetTableValue(1, color.redF(), color.greenF(), color.blueF(), color.alphaF());
  lut->Modified();

  int plane = 0;
  vtkSmartPointer<vtkMatrix4x4> matrix = vtkSmartPointer<vtkMatrix4x4>::New();
  if (extent[4]==extent[5])
  {
    double elements[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, extent[4]*spacing[2], 0, 0, 0, 1 };
    matrix->DeepCopy(elements);
    i2v->GetOutput()->SetOrigin(0,0, point[2] - (extent[4]*spacing[2])); // resolves "Fit to slices" discrepancy
    plane = 2;
  }
  else if (extent[2] == extent[3])
  {
    double elements[16] = { 1, 0, 0, 0, 0, 0, 1, extent[2]*spacing[1], 0, -1, 0, 0, 0, 0, 0, 1 };
    matrix->DeepCopy(elements);
    i2v->GetOutput()->SetOrigin(0, point[1] - (extent[2]*spacing[1]), 0); // resolves "Fit to slices" discrepancy
    plane = 1;
  }
  else if (extent[0] == extent[1])
  {
    double elements[16] = { 0, 0, -1, extent[0]*spacing[0], 1, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 1 };
    matrix->DeepCopy(elements);
    i2v->GetOutput()->SetOrigin(point[0] - (extent[0]*spacing[0]), 0, 0); // resolves "Fit to slices" discrepancy
    plane = 0;
  }
  i2v->GetOutput()->Update();
  delete[] point;

  vtkSmartPointer<vtkImageResliceToColors> reslice = vtkSmartPointer<vtkImageResliceToColors>::New();
  reslice->OptimizationOn();
  reslice->BorderOn();
  reslice->SetOutputFormatToRGBA();
  reslice->AutoCropOutputOff();
  reslice->InterpolateOff();
  reslice->SetResliceAxes(matrix);
  reslice->SetInputConnection(i2v->GetOutput()->GetProducerPort());
  reslice->SetOutputDimensionality(2);
  reslice->SetLookupTable(lut);
  reslice->Update();

  m_actor = vtkSmartPointer<vtkImageActor>::New();
  m_actor->GetMapper()->SetInputConnection(reslice->GetOutputPort());
  m_actor->GetMapper()->BorderOn();
  m_actor->SetInterpolate(false);
  m_actor->Update();

  double pos[3];
  memset(pos, 0, 3*sizeof(double));
  int sign = ((plane == 2) ? -1 : 1);
  pos[plane] += (sign * 0.09); // the crosshair is at 0.1, and the preview must be under it
  m_actor->SetPosition(pos);

  view->addActor(m_actor);
  m_viewOfPreview = view;
  view->updateView();
}

void SeedGrowSegmentationTool::lostEvent(EspinaRenderView *view)
{
  removePreview(view);
}
