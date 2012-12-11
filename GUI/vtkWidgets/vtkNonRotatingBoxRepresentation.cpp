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
	  vtkIdType cellId = HexPicker->GetCellId();

	  this->CurrentHandle = this->Handle[cellId];
	  this->InteractionState = cellId + 1;
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

