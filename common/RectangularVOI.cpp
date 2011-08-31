/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#include "RectangularVOI.h"

#include "espina_debug.h"

#include "sample.h"
#include "filter.h"

#include <pqApplicationCore.h>

#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pq3DWidget.h>
#include <vtkSMProxy.h>
#include <vtkSMPropertyHelper.h>
#include <espina.h>
#include <products.h>
#include <vtkSMNewWidgetRepresentationProxy.h>

#include <QDebug>
#include <assert.h>
#include <cache/cachedObjectBuilder.h>
#include <vtkBoxRepresentation.h>
#include <vtkProperty.h>

#include <vtkObjectFactory.h>
#include <vtkCamera.h>
#include <vtkRenderer.h>
#include <vtkCellPicker.h>
#include <vtkAssemblyPath.h>
#include <vtkPolyData.h>
#include <vtkIdList.h>
#include "vtkNonRotatingBoxWidget.h"

/*
class VTK_WIDGETS_EXPORT VOIRepresentation : public vtkBoxRepresentation
{
public:
  static VOIRepresentation *New();  
  
  vtkTypeMacro(VOIRepresentation, vtkBoxRepresentation); 
  
//   virtual void WidgetInteraction(double e[2]);
  virtual int ComputeInteractionState(int X, int Y, int modify = 0);
};

int VOIRepresentation::ComputeInteractionState(int X, int Y, int modify)
{
    // Okay, we can process this. Try to pick handles first;
  // if no handles picked, then pick the bounding box.
  if (!this->Renderer || !this->Renderer->IsInViewport(X, Y))
    {
    this->InteractionState = vtkBoxRepresentation::Outside;
    return this->InteractionState;
    }

  vtkAssemblyPath *path;
  // Try and pick a handle first
  this->LastPicker = NULL;
  this->CurrentHandle = NULL;
  this->HandlePicker->Pick(X,Y,0.0,this->Renderer);
  path = this->HandlePicker->GetPath();
  if ( path != NULL )
    {
    this->ValidPick = 1;
    this->LastPicker = this->HandlePicker;
    this->CurrentHandle =
           reinterpret_cast<vtkActor *>(path->GetFirstNode()->GetViewProp());
    if ( this->CurrentHandle == this->Handle[0] )
      {
      this->InteractionState = vtkBoxRepresentation::MoveF0;
      }
    else if ( this->CurrentHandle == this->Handle[1] )
      {
      this->InteractionState = vtkBoxRepresentation::MoveF1;
      }
    else if ( this->CurrentHandle == this->Handle[2] )
      {
      this->InteractionState = vtkBoxRepresentation::MoveF2;
      }
    else if ( this->CurrentHandle == this->Handle[3] )
      {
      this->InteractionState = vtkBoxRepresentation::MoveF3;
      }
    else if ( this->CurrentHandle == this->Handle[4] )
      {
      this->InteractionState = vtkBoxRepresentation::MoveF4;
      }
    else if ( this->CurrentHandle == this->Handle[5] )
      {
      this->InteractionState = vtkBoxRepresentation::MoveF5;
      }
    else if ( this->CurrentHandle == this->Handle[6] )
      {
      this->InteractionState = vtkBoxRepresentation::Translating;
      }
    }
  else //see if the hex is picked
    {
    this->HexPicker->Pick(X,Y,0.0,this->Renderer);
    path = this->HexPicker->GetPath();
    if ( path != NULL )
      {
      this->LastPicker = this->HexPicker;
      this->ValidPick = 1;
      if ( !modify )
        {
	  double pp[3];
	  HexPicker->GetPickPosition(pp);
	  std::cout << "Pick at " << pp[0] << " "<< pp[1] << " "<< pp[2] << std::endl;
	  std::cout << "Assembly Path: " << HexPicker->GetCellId() << std::endl;
	  vtkIdType cellId = HexPicker->GetCellId();
	  vtkIdList *facePointsIds = vtkIdList::New();
	  HexPolyData->GetCellPoints(cellId, facePointsIds);
	  
	  double p[4][3];
	  for(int i=0; i<facePointsIds->GetNumberOfIds(); i++)
	  {
	    HexPolyData->GetPoint(facePointsIds->GetId(i),p[i]);
	    std::cout << "\tPoint " << i << " (" << p[i][0] << "," << p[i][1] << "," << p[i][2] << ")" << std::endl;
	  }
	  
	  double absdiffs[4];
	  absdiffs[0] = abs(p[0][0]-pp[0]);
	  absdiffs[1] = abs(pp[0]-p[1][0]);
	  switch (cellId)
	  {
	    case 4:
	      if (absdiffs[0] < absdiffs[1])
	      {
		this->CurrentHandle == this->Handle[4];
		this->InteractionState = vtkBoxRepresentation::MoveF4;
	      }
	      else
	      {
		this->CurrentHandle == this->Handle[1];
		this->InteractionState = vtkBoxRepresentation::MoveF1;
	      }
	      break;
	    default:
	      this->CurrentHandle == this->Handle[6];
	      this->InteractionState = vtkBoxRepresentation::Translating;
	  };
// 	  double *propBounds;
// 	  propBounds = path->GetFirstNode()->GetProp()->GetBounds();
// 	  if (propBounds[0] == propBounds[1])
// 	  {
// 	    this->CurrentHandle == this->Handle[0];
// 	    this->InteractionState = vtkBoxRepresentation::MoveF0;
// 	  }
// 	  else
// 	  {
// 	    this->CurrentHandle == this->Handle[1];
// 	    this->InteractionState = vtkBoxRepresentation::MoveF1;
	    
// 	  }
//         this->InteractionState = vtkBoxRepresentation::Outside;
        }
      else
        {
	  this->CurrentHandle = this->Handle[6];
	  this->InteractionState = vtkBoxRepresentation::Translating;
        }
      }
    else
      {
      this->InteractionState = vtkBoxRepresentation::Outside;
      }
    }

  return this->InteractionState;
}


vtkStandardNewMacro(VOIRepresentation);   
void VOIRepresentation::WidgetInteraction(double e[2])
{
    // Convert events to appropriate coordinate systems
  vtkCamera *camera = this->Renderer->GetActiveCamera();
  if ( !camera )
    {
    return;
    }
  double focalPoint[4], pickPoint[4], prevPickPoint[4];
  double z, vpn[3];
  camera->GetViewPlaneNormal(vpn);

  // Compute the two points defining the motion vector
  double pos[3];
  if ( this->LastPicker == this->HexPicker )
    {    
    this->HexPicker->GetPickPosition(pos);
    }    
  else 
    {    
    this->HandlePicker->GetPickPosition(pos);
    }    
  vtkInteractorObserver::ComputeWorldToDisplay(this->Renderer,
                                               pos[0], pos[1], pos[2],
                                               focalPoint);
  z = focalPoint[2];
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer,this->LastEventPosition[0],
                                               this->LastEventPosition[1], z, prevPickPoint);
  vtkInteractorObserver::ComputeDisplayToWorld(this->Renderer, e[0], e[1], z, pickPoint);

  // Process the motion
  if ( this->InteractionState == vtkBoxRepresentation::MoveF0 )
    {    
    this->MoveMinusXFace(prevPickPoint,pickPoint);
    }    

  else if ( this->InteractionState == vtkBoxRepresentation::MoveF1 )
    {
    this->MovePlusXFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoxRepresentation::MoveF2 )
    {
    this->MoveMinusYFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoxRepresentation::MoveF3 )
    {
    this->MovePlusYFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoxRepresentation::MoveF4 )
    {
    this->MoveMinusZFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoxRepresentation::MoveF5 )
    {
    this->MovePlusZFace(prevPickPoint,pickPoint);
    }

  else if ( this->InteractionState == vtkBoxRepresentation::Translating )
    {
    this->Translate(prevPickPoint, pickPoint);
    }

  else if ( this->InteractionState == vtkBoxRepresentation::Scaling )
    {
    this->Scale(prevPickPoint, pickPoint,
                static_cast<int>(e[0]), static_cast<int>(e[1]));
    }

  // Store the start position
  this->LastEventPosition[0] = e[0];
  this->LastEventPosition[1] = e[1];
  this->LastEventPosition[2] = 0.0;

}*/


const QString RectangularVOI::ApplyFilter::FilterType = "RectangularVOI::ApplyFilter";

RectangularVOI::ApplyFilter::ApplyFilter(vtkProduct* input, double* bounds)
{
   CachedObjectBuilder *cob = CachedObjectBuilder::instance();

   vtkFilter::Arguments args;
   args.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, input->id()));
   QString VolumeArg = QString("%1,%2,%3,%4,%5,%6").arg(bounds[0]).arg(bounds[1]).arg(bounds[2]).arg(bounds[3]).arg(bounds[4]).arg(bounds[5]);
   args.push_back(vtkFilter::Argument(QString("VOI"),vtkFilter::INTVECT, VolumeArg));
   m_rvoi = cob->createFilter("filters","RectangularVOI",args);
   
   m_args.append(ESPINA_ARG("Type",FilterType));
   m_args.append(ESPINA_ARG("Input",input->id()));
   m_args.append(ESPINA_ARG("Bound", VolumeArg));
}

//-----------------------------------------------------------------------------
RectangularVOI::ApplyFilter::ApplyFilter(ITraceNode::Arguments &args)
{
   CachedObjectBuilder *cob = CachedObjectBuilder::instance();

   vtkFilter::Arguments vtkArgs;
   vtkArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, args["Input"]));
   vtkArgs.push_back(vtkFilter::Argument(QString("VOI"),vtkFilter::INTVECT, args["Bound"]));
   m_rvoi = cob->createFilter("filters","RectangularVOI", vtkArgs);

   m_args.append(ESPINA_ARG("Type",FilterType));
   m_args.append(ESPINA_ARG("Input",args["Input"]));
   m_args.append(ESPINA_ARG("Bound", args["Bound"]));
}

//-----------------------------------------------------------------------------
RectangularVOI::ApplyFilter::~ApplyFilter()
{
   CachedObjectBuilder *cob = CachedObjectBuilder::instance();
   
   cob->removeFilter(m_rvoi);
}


//-----------------------------------------------------------------------------
void RectangularVOI::ApplyFilter::removeProduct(vtkProduct* product)
{
  assert(false);
}

//-----------------------------------------------------------------------------
RectangularVOI::RectangularVOI(bool registerPlugin)
: m_box(NULL)
{
  //bzero(m_widget,4*sizeof(pq3DWidget *));
  QString registerName = ApplyFilter::FilterType;
  if (registerPlugin)
    ProcessingTrace::instance()->registerPlugin(registerName, this);
}

//-----------------------------------------------------------------------------
EspinaFilter* RectangularVOI::createFilter(QString filter, ITraceNode::Arguments& args)
{
  if (filter == ApplyFilter::FilterType)
    return new ApplyFilter(args);
  else
    return NULL;
}

//-----------------------------------------------------------------------------
EspinaFilter *RectangularVOI::applyVOI(vtkProduct* product)
{
  // To apply widget bounds to vtkBox source
  if (m_widgets.size() > 0)
    m_widgets.first()->accept();
  
  double voiExtent[6];
  rvoiExtent(voiExtent);
  //WARNING: How to deal with bounding boxes out of resources...
  
  //m_rvoi[0] = std::max(productExtent[0], round(pos[0] + m_rvoi[0] * scale[0]/productSpacing[0]));
  //m_rvoi[1] = round(pos[0] + m_rvoi[1] * scale[0]);
  //m_rvoi[2] = round(pos[1] + m_rvoi[2] * scale[1]);
  //m_rvoi[3] = round(pos[1] + m_rvoi[3] * scale[1]);
  //m_rvoi[4] = round(pos[2] + m_rvoi[4] * scale[2]);
  //m_rvoi[5] = round((pos[2] + m_rvoi[5] * scale[2]/2));
  
  //qDebug() << "RectangularVOI Plugin::ApplyVOI on: "<< voiExtent[0]<< voiExtent[1]<< voiExtent[2]<< voiExtent[3]<< voiExtent[4]<< voiExtent[5];
  EspinaFilter *rvoi = new ApplyFilter(product,voiExtent);
  
  return rvoi;
}

//-----------------------------------------------------------------------------
EspinaFilter *RectangularVOI::restoreVOITransormation(vtkProduct* product)
{
  return NULL;
}


//-----------------------------------------------------------------------------
vtkSMProxy* RectangularVOI::getProxy()
{
  if (!m_box)
  {
    pqObjectBuilder *builder =  pqApplicationCore::instance()->getObjectBuilder();
    m_box =  builder->createProxy("implicit_functions","NonRotatingBox",pqApplicationCore::instance()->getActiveServer(),"widgets");
  }
  return m_box;
}

//-----------------------------------------------------------------------------
//WARNING: m_box representation's property values are invalid until accept
//         on widget after it has been added to a view and selected.
pq3DWidget* RectangularVOI::newWidget(ViewType viewType)
{
  QList<pq3DWidget *> widgets =  pq3DWidget::createWidgets(m_product->creator()->pipelineSource()->getProxy(), getProxy());
  
  assert(widgets.size() == 1);
  // By default ParaView doesn't "Apply" the changes to the widget. So we set 
  // up a slot to "Apply" when the interaction ends. 
  QObject::connect(widgets[0], SIGNAL(widgetEndInteraction()),
		   widgets[0], SLOT(accept()));
  QObject::connect(widgets[0], SIGNAL(widgetEndInteraction()),
		   this, SLOT(modifyVOI()));
//   QObject::connect(widgets[0], SIGNAL(widgetEndInteraction()),
// 		   this, SLOT(endInteraction()));
  //QObject::connect(this->PlaneWidget, SIGNAL(widgetEndInteraction()),
  //  this->View, SLOT(render())); 
  
  m_widgets.push_back(widgets[0]);
  
  vtkNonRotatingBoxWidget *boxwidget = dynamic_cast<vtkNonRotatingBoxWidget*>(widgets[0]->getWidgetProxy()->GetWidget());
  assert(boxwidget);
  if (viewType == VIEW_PLANE_YZ)
    boxwidget->SetInvertZCursor(true);
  if (viewType == VIEW_3D)
    boxwidget->SetProcessEvents(false);
  
  vtkBoxRepresentation *repbox =  dynamic_cast<vtkBoxRepresentation*>(boxwidget->GetRepresentation());
  repbox->HandlesOff();
  repbox->OutlineCursorWiresOff();
  vtkProperty *outline = repbox->GetOutlineProperty();
  outline->SetColor(1.0,1.0,0);
  
  return widgets[0];
}

//-----------------------------------------------------------------------------
void RectangularVOI::deleteWidget(pq3DWidget*& widget)
{
  m_widgets.removeOne(widget);
  qDebug() << "Active Widgets" << m_widgets.size();
  delete widget;
  widget = NULL;
}

//-----------------------------------------------------------------------------
bool RectangularVOI::contains(ISelectionHandler::VtkRegion region)
{
  foreach(Point p, region)
  {
    double voiExtent[6];
    rvoiExtent(voiExtent);
    if (p.x < voiExtent[0] || voiExtent[1] < p.x)
      return false;
    if (p.y < voiExtent[2] || voiExtent[3] < p.y)
      return false;
    if (p.z < voiExtent[4] || voiExtent[5] < p.z)
      return false;
  }
  return true;
}

//----------------------------------------------------------------------------
bool RectangularVOI::intersectPlane(ViewType plane, int slice)
{
  double voiExtent[6];
  rvoiExtent(voiExtent);
  switch(plane)
  {
    case VIEW_PLANE_XY:
      return (voiExtent[4] <= slice && slice <= voiExtent[5]);
    case VIEW_PLANE_XZ:
      return (voiExtent[2] <= slice && slice <= voiExtent[3]);
    case VIEW_PLANE_YZ:
      return (voiExtent[0] <= slice && slice <= voiExtent[1]);
    default:
      return false;
  };
}

//-----------------------------------------------------------------------------
void RectangularVOI::modifyVOI()
{
  emit voiModified();
}


// //----------------------------------------------------------------------------
// void RectangularVOI::endInteraction()
// {
//   //Update all widgets with box proxy bounds
//   assert(m_box);
//   
//   pq3DWidget *widget = qobject_cast<pq3DWidget *>(QObject::sender());
//   widget->accept();
//   
//   double scale[3];
//   vtkSMPropertyHelper(m_box,"Scale").Get(scale,3);
//   double pos[3];
//   vtkSMPropertyHelper(m_box,"Position").Get(pos,3);
//   double bounds[6];
//   vtkSMPropertyHelper(m_box,"Bounds").Get(bounds,6);
// //   qDebug() << "Moving RectangularVOI Plugin::Scale on: "<< scale[0]<< scale[1]<< scale[2];
// //   qDebug() << "Moving RectangularVOI Plugin::Pos on: "<< pos[0]<< pos[1]<< pos[2];
// //   qDebug() << "Bounds" << bounds[0] << bounds[1] << bounds[2] << bounds[3] << bounds[4] << bounds[5];
// }

//-----------------------------------------------------------------------------
void RectangularVOI::cancelVOI()
{
  emit voiCancelled();
}

//-----------------------------------------------------------------------------
void RectangularVOI::setFromSlice(int value)
{
  if (!m_box)
    return;
  
  double productSpacing[3];
  m_product->spacing(productSpacing);
  double extent[6];
  rvoiExtent(extent);
  for (int i=0; i<6; i++)
  {
    extent[i] = extent[i]*productSpacing[i/2];
  }
  extent[4] = value*productSpacing[2];
  vtkSMPropertyHelper(m_box,"Bounds").Set(extent,6);
  double pos[3] = {0, 0, 0};
  vtkSMPropertyHelper(m_box,"Position").Set(pos,3);
  double scale[3] = {1, 1, 1};
  vtkSMPropertyHelper(m_box,"Scale").Set(scale,3);
  m_box->UpdateVTKObjects();
}

//-----------------------------------------------------------------------------
void RectangularVOI::setToSlice(int value)
{
  if (!m_box)
    return;
  
  double productSpacing[3];
  m_product->spacing(productSpacing);
  double extent[6];
  rvoiExtent(extent);
  for (int i=0; i<6; i++)
  {
    extent[i] = extent[i]*productSpacing[i/2];
  }
  extent[5] = value*productSpacing[2];
  vtkSMPropertyHelper(m_box,"Bounds").Set(extent,6);
  double pos[3] = {0, 0, 0};
  vtkSMPropertyHelper(m_box,"Position").Set(pos,3);
  double scale[3] = {1, 1, 1};
  vtkSMPropertyHelper(m_box,"Scale").Set(scale,3);
  m_box->UpdateVTKObjects();
  foreach(pq3DWidget *widget, m_widgets)
  {
    widget->reset();
  }
  emit voiModified();
}


//-----------------------------------------------------------------------------
void RectangularVOI::rvoiExtent(double* rvoi)
{
  double productSpacing[3];
  m_product->spacing(productSpacing);
  
  double bounds[6];
  vtkSMPropertyHelper(m_box,"Bounds").Get(bounds,6);
  double scale[3];
  vtkSMPropertyHelper(m_box,"Scale").Get(scale,3);
  double pos[3];
  vtkSMPropertyHelper(m_box,"Position").Get(pos,3);
  //qDebug() << "RectangularVOI Plugin: Scale: "<< scale[0]<< scale[1]<< scale[2];
  //qDebug() << "RectangularVOI Plugin: Pos: "<< pos[0]<< pos[1]<< pos[2];
  //qDebug() << "RectangularVOI Plugin: Extent: "<< m_rvoi[0]<< m_rvoi[1]<< m_rvoi[2]<< m_rvoi[3]<< m_rvoi[4]<< m_rvoi[5];
  
  //double productExtent[6] = {bounds[0],bounds[1],bounds[2], bounds[3], bounds[4], bounds[5]/2};
  for (int i=0; i<6; i++)
    rvoi[i] = round((pos[i/2] + bounds[i]*scale[i/2])/productSpacing[i/2]);
}
