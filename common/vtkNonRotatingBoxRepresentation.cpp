#include "vtkNonRotatingBoxRepresentation.h"

#include <vtkRenderer.h>
#include <vtkIdList.h>
#include <vtkAssemblyPath.h>
#include <vtkCellPicker.h>
#include <vtkPolyData.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkNonRotatingBoxRepresentation); 

int vtkNonRotatingBoxRepresentation::ComputeInteractionState(int X, int Y, int modify)
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

