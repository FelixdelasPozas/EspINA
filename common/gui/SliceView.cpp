/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.es>

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

#include "common/gui/SliceView.h"

// // EspINA
#include "EspinaTypes.h"
#include "common/settings/ISettingsPanel.h"
#include "common/model/Channel.h"
#include "common/model/Representation.h"
#include "common/model/Segmentation.h"
#include "common/selection/SelectionManager.h"
#include "ColorEngine.h"
#include "common/gui/SliceViewState.h"
#include "vtkInteractorStyleEspinaSlice.h"
#include <vtkRectangularWidget.h>
#include <vtkRectangularRepresentation.h>
#include <vtkLookupTable.h>
#include "common/gui/VolumeView.h"

// Debug
#include <QDebug>
// Qt includes
#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollBar>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QVector3D>
#include <QWheelEvent>

#include <vtkInteractorStyleImage.h>
#include <vtkObjectFactory.h>
#include <vtkPropCollection.h>
#include <vtkCommand.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkWorldPointPicker.h>
#include <vtkCamera.h>
#include <vtkPolyDataMapper.h>
#include <vtkImageActor.h>
#include <vtkImageMapper3D.h>
#include <vtkImageProperty.h>
#include <vtkProperty.h>
#include <vtkAbstractWidget.h>
#include <vtkWidgetRepresentation.h>
#include <QVTKWidget.h>
#include <vtkSphereSource.h>
#include <vtkImageResliceToColors.h>
#include <vtkMatrix4x4.h>
#include <vtkAlgorithmOutput.h>
#include <itkImageToVTKImageFilter.h>
#include <vtkCoordinate.h>
#include <vtkCellArray.h>
#include <vtkCellPicker.h>
#include <vtkProp3DCollection.h>
#include <vtkPropPicker.h>

// class MouseMoveCallback : public vtkCommand
// {
//   static MouseMoveCallback *New()
//   {
//     return new MouseMoveCallback();
//   }
// 
// virtual void execute(vtkobject* caller, long unsigned int eventid, void* calldata)
// {
//   //vtkinteractorstyle *style = static_cast<vtkinteractorstyle *>(caller);
// 
//   switch(eventid)
//   {
//     case vtkcommand::mousemoveevent:
//       qdebug() << "moviendose";
//   }
// }
// };

// ColorEngine *SliceView::m_colorEngine = NULL;
// Nm SliceView::m_crosshairPoint[];
// Nm SliceView::m_slicingRanges[];
// Nm SliceView::m_slicingStep[];

//-----------------------------------------------------------------------------
// SLICE VIEW
//-----------------------------------------------------------------------------
SliceView::SliceView(PlaneType plane, QWidget* parent) :
QWidget(parent)
, m_titleLayout(new QHBoxLayout())
, m_title(new QLabel("Sagital"))
, m_mainLayout(new QVBoxLayout())
, m_controlLayout(new QHBoxLayout())
, m_view(new QVTKWidget())
, m_scrollBar(new QScrollBar(Qt::Horizontal))
, m_fromSlice(new QPushButton("From"))
, m_spinBox(new QSpinBox())
, m_toSlice(new QPushButton("To"))
, m_plane(plane)
, m_settings(new Settings(m_plane))
, m_inThumbnail(false)
{
  memset(m_crosshairPoint, 0, 3*sizeof(Nm));
  memset(m_slicingRanges, 0, 6 * sizeof(Nm));
  m_slicingStep[0] = m_slicingStep[1] = m_slicingStep[2] = 1;

  setupUI();

  this->setAutoFillBackground(true);
  setLayout(m_mainLayout);

  // Color background
  QPalette pal = this->palette();
  pal.setColor(QPalette::Base, pal.color(QPalette::Window));
  this->setPalette(pal);
  this->setStyleSheet("QSpinBox { background-color: white;}");

  switch (m_plane)
  {
    case AXIAL:
      m_state = new AxialState();
      break;
    case SAGITTAL:
      m_state = new SagittalState();
      break;
    case CORONAL:
      m_state = new CoronalState();
      break;
  };

  m_renderer = vtkSmartPointer<vtkRenderer>::New();
  m_renderer->GetActiveCamera()->ParallelProjectionOn();
  m_slicingMatrix = vtkMatrix4x4::New();
  m_channelPicker = vtkSmartPointer<vtkCellPicker>::New();
  m_channelPicker->PickFromListOn();
  m_segmentationPicker = vtkSmartPointer<vtkCellPicker>::New();
  m_segmentationPicker->PickFromListOn();

  m_state->updateSlicingMatrix(m_slicingMatrix);
  m_view->GetRenderWindow()->AddRenderer(m_renderer);
  m_view->GetInteractor()->SetInteractorStyle(vtkInteractorStyleEspinaSlice::New());
  m_view->GetRenderWindow()->Render();

  buildCrosshairs();
}

//-----------------------------------------------------------------------------
void SliceView::buildCrosshairs()
{
 vtkSmartPointer<vtkPoints> HPoints = vtkSmartPointer<vtkPoints>::New();
  HPoints->InsertNextPoint(-0.5, 0, 0);
  HPoints->InsertNextPoint(0.5, 0, 0);
  vtkSmartPointer<vtkCellArray> HLine = vtkSmartPointer<vtkCellArray>::New();
  HLine->EstimateSize(1, 2);
  HLine->InsertNextCell (2);
  HLine->InsertCellPoint(0);
  HLine->InsertCellPoint(1);

  m_HCrossLineData = vtkSmartPointer<vtkPolyData>::New();
  m_HCrossLineData->SetPoints(HPoints);
  m_HCrossLineData->SetLines (HLine);
  m_HCrossLineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> HMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  HMapper->SetInput(m_HCrossLineData);

  m_HCrossLine = vtkSmartPointer<vtkActor>::New();
  m_HCrossLine->SetMapper(HMapper);
  m_HCrossLine->GetProperty()->SetLineWidth(2);
  m_HCrossLine->SetPickable(false);

  vtkSmartPointer<vtkPoints> VPoints = vtkSmartPointer<vtkPoints>::New();
  VPoints->InsertNextPoint(0, -0.5, 0);
  VPoints->InsertNextPoint(0, 0.5, 0);
  vtkSmartPointer<vtkCellArray> VLine = vtkSmartPointer<vtkCellArray>::New();
  VLine->EstimateSize(1, 2);
  VLine->InsertNextCell (2);
  VLine->InsertCellPoint(0);
  VLine->InsertCellPoint(1);

  m_VCrossLineData = vtkSmartPointer<vtkPolyData>::New();
  m_VCrossLineData->SetPoints(VPoints);
  m_VCrossLineData->SetLines(VLine);
  m_VCrossLineData->Update();

  vtkSmartPointer<vtkPolyDataMapper> VMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  VMapper->SetInput(m_VCrossLineData);

  m_VCrossLine = vtkSmartPointer<vtkActor>::New();
  m_VCrossLine->SetMapper(VMapper);
  m_VCrossLine->GetProperty()->SetLineWidth(2);
  m_VCrossLine->SetPickable(false);

}

//-----------------------------------------------------------------------------
void SliceView::buildTitle()
{
  QPushButton *close = new QPushButton("x");
  close->setMaximumHeight(20);
  close->setMaximumWidth(20);

  QPushButton *max = new QPushButton("-");
  max->setMaximumHeight(20);
  max->setMaximumWidth(20);

  QPushButton *undock = new QPushButton("^");
  undock->setMaximumHeight(20);
  undock->setMaximumWidth(20);

  connect(close, SIGNAL(clicked(bool)),
          this, SLOT(close()));
  connect(max, SIGNAL(clicked(bool)),
          this, SLOT(maximize()));
  connect(undock, SIGNAL(clicked(bool)),
          this, SLOT(undock()));

  m_titleLayout->addWidget(m_title);
  m_titleLayout->addWidget(max);
  m_titleLayout->addWidget(undock);
  m_titleLayout->addWidget(close);

  m_mainLayout->addLayout(m_titleLayout);
}

//-----------------------------------------------------------------------------
void SliceView::setupUI()
{
  m_view->installEventFilter(this);
  m_scrollBar->setMaximum(0);
  m_scrollBar->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  m_fromSlice->setFlat(true);
  m_fromSlice->setVisible(false);
  m_fromSlice->setEnabled(false);
  m_fromSlice->setMaximumHeight(20);
  m_fromSlice->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  connect(m_fromSlice, SIGNAL(clicked(bool)), this, SLOT(selectFromSlice()));

  m_spinBox->setMaximum(0);
  m_spinBox->setMinimumWidth(40);
  m_spinBox->setMaximumHeight(20);
  m_spinBox->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  m_spinBox->setAlignment(Qt::AlignRight);

  m_toSlice->setFlat(true);
  m_toSlice->setVisible(false);
  m_toSlice->setEnabled(false);
  m_toSlice->setMaximumHeight(20);
  m_toSlice->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
  connect(m_toSlice,   SIGNAL(clicked(bool)),     this,        SLOT(selectToSlice()));
  connect(m_scrollBar, SIGNAL(valueChanged(int)), m_spinBox,   SLOT(setValue(int)));
  connect(m_scrollBar, SIGNAL(valueChanged(int)), this,        SLOT(scrollValueChanged(int)));
  connect(m_spinBox,   SIGNAL(valueChanged(int)), m_scrollBar, SLOT(setValue(int)));

  //   connect(SelectionManager::instance(),SIGNAL(VOIChanged(IVOI*)),this,SLOT(setVOI(IVOI*)));
  m_mainLayout->addWidget(m_view);
  m_controlLayout->addWidget(m_scrollBar);
  m_controlLayout->addWidget(m_fromSlice);
  m_controlLayout->addWidget(m_spinBox);
  m_controlLayout->addWidget(m_toSlice);

  m_mainLayout->addLayout(m_controlLayout);
}

//-----------------------------------------------------------------------------
SliceView::~SliceView()
{
  delete m_state;
}

//-----------------------------------------------------------------------------
QString SliceView::title() const
{
  return m_title->text();
}

//-----------------------------------------------------------------------------
void SliceView::setTitle(const QString& title)
{
  m_title->setText(title);
}

//-----------------------------------------------------------------------------
void SliceView::setCrosshairColors(double hcolor[3], double vcolor[3])
{
  m_HCrossLine->GetProperty()->SetColor(hcolor);
  m_VCrossLine->GetProperty()->SetColor(vcolor);
}

//-----------------------------------------------------------------------------
void SliceView::setCrosshairVisibility(bool visible)
{
  if (visible)
  {
    m_renderer->AddActor(m_HCrossLine);
    m_renderer->AddActor(m_VCrossLine);
//     vtkRectangularWidget *boxWidget = vtkRectangularWidget::New();
//     boxWidget->SetInteractor(m_view->GetRenderWindow()->GetInteractor());
//     boxWidget->CreateDefaultRepresentation();
//     boxWidget->SetPlane(m_plane);
//     boxWidget->SetEnabled(true);
//     boxWidget->GetRepresentation()->PlaceWidget(m_range);
//     boxWidget->On();
//     vtkRectangularRepresentation *rep = vtkRectangularRepresentation::SafeDownCast(boxWidget->GetRepresentation());
  }else
  {
    m_renderer->RemoveActor(m_HCrossLine);
    m_renderer->RemoveActor(m_VCrossLine);
  }
  forceRender();
}

//-----------------------------------------------------------------------------
void SliceView::setThumbnailVisibility(bool visible)
{
  //TODO:vtkSMPropertyHelper(m_view->getViewProxy(),"ShowThumbnail").Set(visible);
  forceRender();
}

//-----------------------------------------------------------------------------
void SliceView::resetCamera()
{
  double origin[3] = { 0, 0, 0 };
  m_state->updateCamera(m_renderer->GetActiveCamera(), origin);
  m_renderer->ResetCamera();
}

//-----------------------------------------------------------------------------
void SliceView::eventPosition(int& x, int& y)
{
  vtkRenderWindowInteractor *rwi = m_view->GetRenderWindow()->GetInteractor();
  Q_ASSERT(rwi);
  rwi->GetEventPosition(x, y);
}

//-----------------------------------------------------------------------------
SelectionHandler::MultiSelection SliceView::select(SelectionHandler::SelectionFilters filters, SelectionHandler::ViewRegions regions)
{
  bool multiSelection = false;
  SelectionHandler::MultiSelection msel;

  //if (m_inThumbnail)
  //  return msel;

  vtkRenderer *renderer = m_renderer;
  Q_ASSERT(renderer);

  ////   qDebug() << "EspINA::SliceView" << m_plane << ": Making selection";
  // Select all products that belongs to all the regions
  foreach(const QPolygonF &region, regions)
  {
    SelectionHandler::VtkRegion vtkRegion;
    // Translate view pixels into Vtk pixels
    vtkRegion = display2vtk(region);

    if (vtkRegion.isEmpty())
      return msel;

    //     qDebug() << "Analyze Region:";
      foreach(QPointF p, region)
      {
        foreach(QString filter, filters)
        {
          //        qDebug() << "\t\tLooking for" << filter;
          if (filter == SelectionHandler::EspINA_Channel)
          {
            foreach(Channel *channel, pickChannels(p.x(), p.y(), renderer, multiSelection))
            {
              SelectionHandler::Selelection selection(vtkRegion,channel);
              msel.append(selection);
            }
          }
          else if (filter == SelectionHandler::EspINA_Segmentation)
          {
            foreach(Segmentation *seg, pickSegmentations(p.x(), p.y(), renderer, multiSelection))
            {
              SelectionHandler::Selelection selection(vtkRegion, seg);
              msel.append(selection);
            }
            // 	} else if (filter == SelectionHandler::EspINA_Representation)
            // 	{
              // 	  foreach(Representation *rep, pickRepresentation(propPicker, multiSelection))
              // 	  {
                // 	    SelectionHandler::Selelection selection(vtkRegion, rep);
                // 	    msel.append(selection);
                // 	  }
          }
          else
          {
            Q_ASSERT(false);
          }
        }
      }
  }

  return msel;
}

//-----------------------------------------------------------------------------
QVTKWidget *SliceView::view()
{
  return m_view;
}

//-----------------------------------------------------------------------------
vtkRenderWindow* SliceView::renderWindow()
{
  return m_view->GetRenderWindow();
}

//-----------------------------------------------------------------------------
void SliceView::sliceViewCenterChanged(Nm x, Nm y, Nm z)
{
  qDebug() << "Slice View: " << m_plane << " has new center";
  emit centerChanged(x, y, z);
}

//-----------------------------------------------------------------------------
void SliceView::scrollValueChanged(int value/*nm*/)
{
  m_state->setSlicingPosition(m_slicingMatrix, slicingPosition());
  forceRender();
  emit sliceChanged(this->m_settings->plane(), slicingPosition());
}

//-----------------------------------------------------------------------------
void SliceView::selectFromSlice()
{
  emit selectedFromSlice(slicingPosition(), m_plane);
}

//-----------------------------------------------------------------------------
void SliceView::selectToSlice()
{
  emit selectedToSlice(slicingPosition(), m_plane);
}

//-----------------------------------------------------------------------------
void SliceView::close()
{
  emit closeRequest();
}

//-----------------------------------------------------------------------------
void SliceView::maximize()
{
  emit maximizeRequest();
}

//-----------------------------------------------------------------------------
void SliceView::minimize()
{
  emit minimizeRequest();
}

//-----------------------------------------------------------------------------
void SliceView::undock()
{
  emit undockRequest();
}

//-----------------------------------------------------------------------------
bool SliceView::eventFilter(QObject* caller, QEvent* e)
{
  if (SelectionManager::instance()->filterEvent(e, this))
    return true;

  if (e->type() == QEvent::Wheel)
  {
    QWheelEvent *we = static_cast<QWheelEvent *>(e);
    int numSteps = we->delta() / 8 / 15 * (m_settings->invertWheel() ? -1 : 1);  //Refer to QWheelEvent doc.
    m_spinBox->setValue(m_spinBox->value() - numSteps);
    e->ignore();
  }
  else if (e->type() == QEvent::Enter)
  {
    QWidget::enterEvent(e);
    m_view->setCursor(SelectionManager::instance()->cursor());
    e->accept();
  }
  else if (e->type() == QEvent::MouseMove)
  {
//     QMouseEvent* me = static_cast<QMouseEvent*>(e);
//     vtkSMSliceViewProxy* view =
//     vtkSMSliceViewProxy::SafeDownCast(m_view->getProxy());
//     Q_ASSERT(view);
//     vtkRenderer * thumbnail = view->GetOverviewRenderer();
//     Q_ASSERT(thumbnail);
//     vtkPropPicker *propPicker = vtkPropPicker::New();
//     int x, y;
//     eventPosition(x, y);
//     if (thumbnail->GetDraw() && propPicker->Pick(x, y, 0.1, thumbnail))
//     {
//       if (!m_inThumbnail)
//         QApplication::setOverrideCursor(Qt::ArrowCursor);
//       m_inThumbnail = true;
//     }
//     else
//     {
//       if (m_inThumbnail)
//         QApplication::restoreOverrideCursor();
//       m_inThumbnail = false;
//     }
// 
//     if (m_inThumbnail && me->buttons() == Qt::LeftButton)
//     {
//       centerViewOnMousePosition();
//       if (me->modifiers() == Qt::CTRL)
//       {
//         centerCrosshairOnMousePosition();
//       }
//     }
  }
  else if (e->type() == QEvent::MouseButtonPress)
  {
    QMouseEvent* me = static_cast<QMouseEvent*>(e);
    if (me->button() == Qt::LeftButton)
    {
      if (me->modifiers() == Qt::CTRL)
      {
        centerCrosshairOnMousePosition();
        emit showCrosshairs(true);
      }
      else if (m_inThumbnail)
      {
        centerViewOnMousePosition();
      }
      else
      {
        selectPickedItems(me->modifiers() == Qt::SHIFT);
      }
    }
  }
  else if (e->type() == QEvent::KeyRelease)
  {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);
    if (ke->key() == Qt::Key_Control && ke->count() == 1)
      emit showCrosshairs(false);
  }

  return QWidget::eventFilter(caller, e);
}

//-----------------------------------------------------------------------------
void SliceView::centerCrosshairOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  Nm center[3];  //World coordinates
  pickChannel(xPos, yPos, center);

  centerViewOn(center);
  emit centerChanged(m_crosshairPoint[0], m_crosshairPoint[1], m_crosshairPoint[2]);
}

//-----------------------------------------------------------------------------
void SliceView::centerViewOnMousePosition()
{
  int xPos, yPos;
  eventPosition(xPos, yPos);

  double center[3];  //World coordinates
  pickChannel(xPos, yPos, center);

  Q_ASSERT(false);

  //vtkCamera * camera = m_view->getRenderViewProxy()->GetRenderer()->GetActiveCamera();
  //camera->SetFocalPoint(center);
  //m_view->render();
}

//-----------------------------------------------------------------------------
QList<Channel *> SliceView::pickChannels(double vx,
					 double vy,
					 vtkRenderer* renderer,
					 bool repeatable)
{
  QList<Channel *> channels;

  if (m_channelPicker->Pick(vx, vy, 0.1, renderer))
  {
    m_channelPicker->GetProp3Ds()->InitTraversal();
    vtkProp3D *pickedProp;
    while ((pickedProp = m_channelPicker->GetProp3Ds()->GetNextProp3D()))
    {
      Channel *pickedChannel = property3DChannel(pickedProp);
      Q_ASSERT(pickedChannel);
//       qDebug() << "Picked" << pickedChannel->data().toString();
      channels << pickedChannel;

      if (!repeatable)
	return channels;
    }
  }

  return channels;
}

//-----------------------------------------------------------------------------
QList<Segmentation *> SliceView::pickSegmentations(double vx,
                                                   double vy,
                                                   vtkRenderer* renderer,
                                                   bool repeatable)
{
  QList<Segmentation *> segmentations;

  if (m_segmentationPicker->Pick(vx, vy, 0.1, renderer))
  {
    QPolygonF selectedRegion;
    selectedRegion << QPointF(vx, vy);
    QVector3D pixel = display2vtk(selectedRegion).first();
 
    m_segmentationPicker->GetProp3Ds()->InitTraversal();
    vtkProp3D *pickedProp;
    while ((pickedProp = m_segmentationPicker->GetProp3Ds()->GetNextProp3D()))
    {
      Segmentation *pickedSeg = property3DSegmentation(pickedProp);
      Q_ASSERT(pickedSeg);
      Q_ASSERT(pickedSeg->volume());
//       qDebug() << "Picked" << pickedSeg->data().toString() << "bounds";
      EspinaVolume::IndexType pickedPixel = pickedSeg->index(pixel.x(), pixel.y(), pixel.z());
      if (!pickedSeg->volume()->GetLargestPossibleRegion().IsInside(pickedPixel) ||
          (pickedSeg->volume()->GetPixel(pickedPixel) == 0))
	continue;

//       qDebug() <<  pickedSeg->data().toString() << "picked";
      segmentations << pickedSeg;

      if (!repeatable)
	return segmentations;
    }
  }

  return segmentations;
}

//-----------------------------------------------------------------------------
void SliceView::selectPickedItems(bool append)
{
  int vx, vy;
  eventPosition(vx, vy);

  bool segPicked = false;
  // If no append, segmentations have priority over channels
  foreach(Segmentation *seg, pickSegmentations(vx, vy, m_renderer, append))
  {
    segPicked = true;
    emit segmentationSelected(seg, append);
    if (!append)
      return;
  }

  // Heterogeneus picking is not supported
  if (segPicked)
    return;

  foreach(Channel *channel, pickChannels(vx, vy, m_renderer, append))
  {
    emit channelSelected(channel);
    if (!append)
      return;
  }
}

//-----------------------------------------------------------------------------
double SliceView::suggestedChannelOpacity()
{
  double numVisibleRep = 0;

  foreach(Channel * channel, m_channels.keys())
    if (channel->isVisible())
      numVisibleRep++;

    if (numVisibleRep == 0)
      return 0.0;

    return 1.0 / numVisibleRep;
}

//-----------------------------------------------------------------------------
void SliceView::updateWidgetVisibility()
{
  foreach(SliceWidget * widget, m_widgets)
  {
    widget->setSlice(slicingPosition(), m_plane);
  }
}

//-----------------------------------------------------------------------------
Nm SliceView::slicingPosition() const
{
  return m_slicingStep[m_plane] * m_spinBox->value();
}

//-----------------------------------------------------------------------------
Channel* SliceView::property3DChannel(vtkProp3D* prop)
{
  foreach(Channel *channel, m_channels.keys())
  {
    if (m_channels[channel].slice == prop)
      return channel;
  }
  return NULL;
}

//-----------------------------------------------------------------------------
Segmentation* SliceView::property3DSegmentation(vtkProp3D* prop)
{
  foreach(Segmentation *seg, m_segmentations.keys())
  {
    if (m_segmentations[seg].slice == prop)
      return seg;
  }
  return NULL;
}


//-----------------------------------------------------------------------------
bool SliceView::pickChannel(int x, int y, Nm pickPos[3])
{
  //   vtkRenderer * thumbnail = view->GetOverviewRenderer();
  //   Q_ASSERT(thumbnail);

  vtkSmartPointer<vtkPropPicker> propPicker = vtkSmartPointer<vtkPropPicker> ::New();
  //   if (!thumbnail->GetDraw() || !propPicker->Pick(x, y, 0.1, thumbnail))
  //   {
    if (!propPicker->Pick(x, y, 0.1, m_renderer))
    {
      //qDebug() << "ePick Fail!";
      return false;
    }
    //   }

    propPicker->GetPickPosition(pickPos);

    pickPos[m_plane] = slicingPosition();
    //   qDebug() << "Pick Position" << pickPos[0] << pickPos[1] << pickPos[2];

    return true;
}

//-----------------------------------------------------------------------------
void SliceView::addChannelRepresentation(Channel* channel)
{
  Q_ASSERT(!m_channels.contains(channel));

  SliceRep channelRep;

  channelRep.resliceToColors = vtkImageResliceToColors::New();
  channelRep.resliceToColors->ReleaseDataFlagOn();
  channelRep.resliceToColors->SetResliceAxes(m_slicingMatrix);
  channelRep.resliceToColors->SetInputConnection(channel->image());
  channelRep.resliceToColors->SetOutputDimensionality(2);

  channelRep.slice = vtkImageActor::New();
  channelRep.slice->SetInterpolate(false);
  channelRep.slice->GetMapper()->BorderOn();
  channelRep.slice->GetMapper()->SetInputConnection(channelRep.resliceToColors->GetOutputPort());
  m_state->updateActor(channelRep.slice);

    // if hue is -1 then use 0 saturation to make a grayscale image
  double hue = channel->color();
  double sat = hue >= 0?1.0:0.0;

  channelRep.selected = false;
  channelRep.visible = !channel->isVisible();  // Force initialization
  channelRep.color = channel->color();
  channelRep.lut = vtkLookupTable::New();
  channelRep.lut->Allocate();
  channelRep.lut->SetTableRange(0,255);
  channelRep.lut->SetHueRange(hue, hue);
  channelRep.lut->SetSaturationRange(0.0, sat);
  channelRep.lut->SetValueRange(0.0, 1.0);
  channelRep.lut->SetAlphaRange(1.0,1.0);
  channelRep.lut->SetNumberOfColors(256);
  channelRep.lut->SetRampToLinear();
  channelRep.lut->Build();

  channelRep.resliceToColors->SetLookupTable(channelRep.lut);
  channelRep.resliceToColors->Update();

  m_channels.insert(channel, channelRep);
  m_renderer->AddActor(channelRep.slice);
  m_channelPicker->AddPickList(channelRep.slice);
  forceRender();  // m_view->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void SliceView::removeChannelRepresentation(Channel* channel)
{
  Q_ASSERT(m_channels.contains(channel));

  SliceRep rep = m_channels[channel];
  m_renderer->RemoveActor(rep.slice);

  m_channels.remove(channel);
  rep.slice->Delete();
  rep.resliceToColors->Delete();
}

//-----------------------------------------------------------------------------
bool SliceView::updateChannelRepresentation(Channel* channel)
{
  Q_ASSERT(m_channels.contains(channel));
  SliceRep &rep = m_channels[channel];

  double pos[3];
  channel->position(pos);

  if (channel->isVisible() != rep.visible || channel->color() != rep.color.hueF() || memcmp(pos, rep.pos, 3 * sizeof(double)))
  {
    rep.visible = channel->isVisible();
    rep.color.setHsvF(channel->color(), 1.0, 1.0);
    memcpy(rep.pos, pos, 3 * sizeof(double));

    rep.slice->SetPosition(rep.pos);
    // don't want to update the color table everytime
    if (channel->color() != rep.color.hueF())
    {
      // if hue is -1 then use 0 saturation to make a grayscale image
      double hue = channel->color();
      double sat = hue >= 0?1.0:0.0;

      rep.lut->Allocate();
      rep.lut->SetTableRange(0,255);
      rep.lut->SetHueRange(hue, hue);
      rep.lut->SetSaturationRange(0.0, sat);
      rep.lut->SetSaturationRange(0.0, 1.0);
      rep.lut->SetValueRange(0.0, 1.0);
      rep.lut->SetAlphaRange(1.0,1.0);
      rep.lut->SetNumberOfColors(256);
      rep.lut->Build();
      rep.lut->Modified();
    }
    rep.color.setHsvF(channel->color(), 1.0, 1.0);
    rep.slice->SetVisibility(rep.visible);
    double opacity = suggestedChannelOpacity();
    rep.slice->GetProperty()->SetOpacity(opacity);
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
void SliceView::addSegmentationRepresentation(Segmentation* seg)
{
  Q_ASSERT(!m_segmentations.contains(seg));

  SliceRep segRep;

  segRep.resliceToColors = vtkImageResliceToColors::New();
  segRep.resliceToColors->ReleaseDataFlagOn();
  segRep.resliceToColors->SetResliceAxes(m_slicingMatrix);
  segRep.resliceToColors->SetInputConnection(seg->image());
  segRep.resliceToColors->SetOutputDimensionality(2);

  segRep.resliceToColors->SetLookupTable(m_colorEngine->lut(seg));
  segRep.resliceToColors->Update();

  segRep.slice = vtkImageActor::New();
  segRep.slice->SetInterpolate(false);
  segRep.slice->GetMapper()->BorderOn();
  segRep.slice->GetMapper()->SetInputConnection(segRep.resliceToColors->GetOutputPort());
  m_state->updateActor(segRep.slice);

  segRep.selected = !seg->isSelected();
  segRep.visible = seg->visible();
  segRep.color = m_colorEngine->color(seg);

  m_segmentations.insert(seg, segRep);
  m_renderer->AddActor(segRep.slice);
  m_segmentationPicker->AddPickList(segRep.slice);
  forceRender();  // m_view->GetRenderWindow()->Render();
}

//-----------------------------------------------------------------------------
void SliceView::removeSegmentationRepresentation(Segmentation* seg)
{
  Q_ASSERT(m_segmentations.contains(seg));

  SliceRep rep = m_segmentations[seg];

  m_renderer->RemoveActor(rep.slice);
  m_segmentationPicker->DeletePickList(rep.slice);

  // itkvtk filter is handled by a smartpointer, these two are not
  rep.slice->Delete();
  rep.resliceToColors->Delete();

  m_segmentations.remove(seg);

  forceRender();
}

//-----------------------------------------------------------------------------
bool SliceView::updateSegmentationRepresentation(Segmentation* seg)
{
  Q_ASSERT(m_segmentations.contains(seg));
  SliceRep &rep = m_segmentations[seg];

  if ((seg->isSelected() != rep.selected)
    || (seg->visible() != rep.visible)
    || (seg->data(Qt::DecorationRole).value<QColor>() != rep.color)
    || seg->updateForced())
  {
    rep.selected = seg->isSelected();
    rep.visible = seg->visible();
    rep.color = seg->data(Qt::DecorationRole).value<QColor>();

    rep.resliceToColors->SetLookupTable(m_colorEngine->lut(seg));
    rep.resliceToColors->Update();

    m_segmentations[seg].slice->SetVisibility(seg->visible());
    return true;
  }
  return false;
}

// //-----------------------------------º------------------------------------------
// void SliceView::addRepresentation(pqOutputPort* oport, QColor color)
// {
//   pqPipelineSource *source = oport->getSource();
//   vtkSMProxyManager   *pxm = vtkSMProxyManager::GetProxyManager();
// 
//   vtkSMRepresentationProxy* reprProxy = vtkSMRepresentationProxy::SafeDownCast(
//     pxm->NewProxy("representations", "SegmentationRepresentation"));
//   Q_ASSERT(reprProxy);
//   m_representations[oport].proxy  = reprProxy;
//   m_representations[oport].color  = color;
// 
//   // Set the reprProxy's input.
//   pqSMAdaptor::setInputProperty(reprProxy->GetProperty("Input"),
//                                 source->getProxy(), oport->getPortNumber());
// 
//   double colorD[3] = {color.redF(), color.greenF(), color.blueF()};
//   vtkSMPropertyHelper(reprProxy, "RGBColor").Set(colorD,3);
// 
//   reprProxy->UpdateVTKObjects();
// 
//   vtkSMProxy* viewModuleProxy = m_view->getProxy();
//   // Add the reprProxy to render module.
//   pqSMAdaptor::addProxyProperty(
//     viewModuleProxy->GetProperty("Representations"), reprProxy);
//   viewModuleProxy->UpdateVTKObjects();
// }
// 
// //-----------------------------------------------------------------------------
// void SliceView::removeRepresentation(pqOutputPort* oport)
// {
//   vtkSMProxy* viewModuleProxy = m_view->getProxy();
// 
//   if (!m_representations.contains(oport))
//     return;
//   //Q_ASSERT(m_representations.contains(rep));
// 
//   RepInfo sliceRep = m_representations[oport];
//   // Remove the reprProxy to render module.
//   pqSMAdaptor::removeProxyProperty(
//     viewModuleProxy->GetProperty("Representations"), sliceRep.proxy);
//   viewModuleProxy->UpdateVTKObjects();
//   m_view->getProxy()->UpdateVTKObjects();
// 
//   sliceRep.proxy->Delete();
//   m_representations.remove(oport);
// }

//-----------------------------------------------------------------------------
void SliceView::addPreview(Filter* filter)
{
	Q_ASSERT(false);
//   addPreview(filter->preview().outputPort());
//   m_preview = filter;
}

void SliceView::removePreview(Filter* filter)
{
	Q_ASSERT(false);
//   removePreview(filter->preview().outputPort());
}

//-----------------------------------------------------------------------------
void SliceView::addWidget(SliceWidget *sWidget)
{
  vtkAbstractWidget *widget = *sWidget;
  widget->SetInteractor(m_view->GetRenderWindow()->GetInteractor());
  widget->GetRepresentation()->SetVisibility(true);
  widget->On();
//   connect(widget, SIGNAL(modified()),
// 	  this, SLOT(updateWidgetVisibility()));
  m_renderer->ResetCameraClippingRange();
  m_widgets << sWidget;
}

//-----------------------------------------------------------------------------
void SliceView::removeWidget(SliceWidget* sWidget)
{
  Q_ASSERT(m_widgets.contains(sWidget));
  m_widgets.removeOne(sWidget);
}

//-----------------------------------------------------------------------------
void SliceView::previewExtent(int VOI[6])
{
  Q_ASSERT(false);
//   memcpy(VOI, m_gridSize, 6 * sizeof(int));
//   VOI[2 * m_plane] = m_scrollBar->value();
//   VOI[2 * m_plane + 1] = m_scrollBar->value();
}

//-----------------------------------------------------------------------------
void SliceView::setSegmentationVisibility(bool visible)
{
  foreach(SliceRep rep, m_segmentations)
  {
    rep.slice->SetVisibility(visible && rep.visible);
  }
  forceRender();
}

//-----------------------------------------------------------------------------
void SliceView::setShowPreprocessing(bool visible)
{
  if (m_channels.size() < 2)
    return;

  Channel *hiddenChannel = m_channels.keys()[visible];
  Channel *visibleChannel = m_channels.keys()[1 - visible];
  hiddenChannel->setData(false, Qt::CheckStateRole);
  hiddenChannel->notifyModification();
  visibleChannel->setData(true, Qt::CheckStateRole);
  visibleChannel->notifyModification();
  for (int i = 2; i < m_channels.keys().size(); i++)
  {
    Channel *otherChannel = m_channels.keys()[i];
    otherChannel->setData(false, Qt::CheckStateRole);
    otherChannel->notifyModification();
  }
}

//-----------------------------------------------------------------------------
void SliceView::setRulerVisibility(bool visible)
{
	//m_view->SetShowRuler(visible);
}

//-----------------------------------------------------------------------------
void SliceView::setSliceSelectors(SliceView::SliceSelectors selectors)
{
	m_fromSlice->setVisible(selectors.testFlag(From));
	m_toSlice->setVisible(selectors.testFlag(To));
}

//-----------------------------------------------------------------------------
void SliceView::forceRender()
{
  if (isVisible())
  {
    //     qDebug() << "Rendering View" << m_plane;
    updateWidgetVisibility();
    m_view->GetRenderWindow()->Render();
    m_view->update();
  }
}

//-----------------------------------------------------------------------------
void SliceView::setSlicingStep(Nm steps[3])
{
  if (steps[0] <= 0 || steps[1] <= 0 || steps[2] <= 0)
  {
    qFatal("SliceView: Invalid Step value. Slicing Step not changed");
    return;
  }

  Nm slicingPos = slicingPosition();

  memcpy(m_slicingStep, steps, 3 * sizeof(Nm));
  setSlicingRanges(m_slicingRanges);

  if (m_slicingStep[0] == 1 && m_slicingStep[1] == 1 && m_slicingStep[2] == 1)
    m_spinBox->setSuffix(" nm");
  else
    m_spinBox->setSuffix("");

  m_scrollBar->setValue(slicingPos/m_slicingStep[m_plane]);
}

//-----------------------------------------------------------------------------
void SliceView::setSlicingRanges(Nm ranges[6])
{
  if (ranges[1] < ranges[0] || ranges[3] < ranges[2] || ranges[5] < ranges[4])
  {
    qFatal("SliceView: Invalid Slicing Ranges. Ranges not changed");
    return;
  }

  double min = ranges[2 * m_plane] / m_slicingStep[m_plane];
  double max = ranges[2 * m_plane + 1] / m_slicingStep[m_plane];

  m_scrollBar->setMinimum(static_cast<int>(min));
  m_scrollBar->setMaximum(static_cast<int>(max));
  m_spinBox->setMinimum(static_cast<int>(min));
  m_spinBox->setMaximum(static_cast<int>(max));

  memcpy(m_slicingRanges, ranges, 6*sizeof(Nm));

  bool enabled = m_spinBox->minimum() < m_spinBox->maximum();
  m_fromSlice->setEnabled(enabled);
  m_toSlice->setEnabled(enabled);
}

//-----------------------------------------------------------------------------
void SliceView::centerViewOn(Nm center[3], bool force)
{
  if (!isVisible() ||
     (m_crosshairPoint[0] == center[0] &&
      m_crosshairPoint[1] == center[1] &&
      m_crosshairPoint[2] == center[2] &&
      !force))
    return;

  // Adjust crosshairs to fit slicing steps
  int sliceNumbers[3];
  for (int i = 0; i < 3; i++)
  {
    sliceNumbers[i] = center[i] / m_slicingStep[i];
    m_crosshairPoint[i] = floor((center[i]/m_slicingStep[i] + 0.5))*m_slicingStep[i];
  }

  // Disable scrollbox signals to avoid calling seting slice
  m_scrollBar->blockSignals(true);
  m_spinBox->setValue(sliceNumbers[m_plane]);
  m_scrollBar->setValue(sliceNumbers[m_plane]);
  m_scrollBar->blockSignals(false);

  m_state->setSlicingPosition(m_slicingMatrix, slicingPosition());
  m_state->setCrossHairs(m_HCrossLineData, m_VCrossLineData, m_crosshairPoint, m_slicingRanges);

  // Only center camera if center is out of the display view
  vtkSmartPointer<vtkCoordinate> coords = vtkSmartPointer<vtkCoordinate>::New();
  coords->SetViewport(m_renderer);
  coords->SetCoordinateSystemToNormalizedViewport();
  double ll[3], ur[3];
  coords->SetValue(0, 0); //LL
  memcpy(ll,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));
  coords->SetValue(1, 1); //UR
  memcpy(ur,coords->GetComputedWorldValue(m_renderer),3*sizeof(double));

  int H = (SAGITTAL == m_plane)?2:0;
  int V = (CORONAL  == m_plane)?2:1;
  bool centerOutOfCamera = m_crosshairPoint[H] < ll[H] || m_crosshairPoint[H] > ur[H] // Horizontally out
                        || m_crosshairPoint[V] > ll[V] || m_crosshairPoint[V] < ur[V];// Vertically out

  if (centerOutOfCamera)
  {
    m_state->updateCamera(m_renderer->GetActiveCamera(), m_crosshairPoint);
    m_renderer->ResetCameraClippingRange();
  }

  forceRender();
}

//-----------------------------------------------------------------------------
SelectionHandler::VtkRegion SliceView::display2vtk(const QPolygonF &region)
{
  //Use Render Window Interactor's Picker to find the world coordinates
  //of the stack
  //vtkSMRenderViewProxy* renModule = view->GetRenderWindow()->GetInteractor()->GetRenderView();
  SelectionHandler::VtkRegion vtkRegion;

  //   vtkWorldPointPicker *wpicker = vtkWorldPointPicker::New();
  double pickPos[3];  //World coordinates
  foreach(QPointF point, region)
  {
    if (!pickChannel(point.x(), point.y(), pickPos))
      continue;

    QVector3D vtkPoint(pickPos[0], pickPos[1], pickPos[2]);
//     vtkPoint.setX(floor((pickPos[0] / m_gridSize[0])+0.5));
//     vtkPoint.setY(floor((pickPos[1] / m_gridSize[1])+0.5));
//     vtkPoint.setZ(floor((pickPos[2] / m_gridSize[2])+0.5));
    vtkRegion << vtkPoint;
  }
  return vtkRegion;

}

//-----------------------------------------------------------------------------
SliceView::Settings::Settings(PlaneType plane, const QString prefix)
: INVERT_SLICE_ORDER(prefix + view(plane) + "::invertSliceOrder")
, INVERT_WHEEL(prefix + view(plane) + "::invertWheel")
, SHOW_AXIS(prefix + view(plane) + "::showAxis")
, m_InvertWheel(false)
, m_InvertSliceOrder(false)
, m_ShowAxis(false)
, m_plane(plane)
{
  QSettings settings("CeSViMa", "EspINA");

  if (!settings.contains(INVERT_SLICE_ORDER))
    settings.setValue(INVERT_SLICE_ORDER, m_InvertSliceOrder);
  if (!settings.contains(INVERT_WHEEL))
    settings.setValue(INVERT_WHEEL, m_InvertWheel);
  if (!settings.contains(SHOW_AXIS))
    settings.setValue(SHOW_AXIS, m_ShowAxis);

  m_InvertSliceOrder = settings.value(INVERT_SLICE_ORDER).toBool();
  m_InvertWheel      = settings.value(INVERT_WHEEL).toBool();
  m_ShowAxis         = settings.value(SHOW_AXIS).toBool();
}

//-----------------------------------------------------------------------------
const QString SliceView::Settings::view(PlaneType plane)
{
  // NOTE: break statements added to avoid warnings
  switch (plane)
  {
    case AXIAL:
      return QString("AxialSliceView");
      break;
    case SAGITTAL:
      return QString("SagittalSliceView");
      break;
    case CORONAL:
      return QString("CoronalSliceView");
      break;
    default:
      Q_ASSERT(false);
      break;
  };

  return QString("Unknown");
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setInvertSliceOrder(bool value)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(INVERT_SLICE_ORDER, value);
  m_InvertSliceOrder = value;
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setInvertWheel(bool value)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(INVERT_WHEEL, value);
  m_InvertWheel = value;
}

//-----------------------------------------------------------------------------
void SliceView::Settings::setShowAxis(bool value)
{
  QSettings settings("CeSViMa", "EspINA");

  settings.setValue(SHOW_AXIS, value);
  m_ShowAxis = value;
}
