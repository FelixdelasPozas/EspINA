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
// 	  std::cout << "Pick at " << pp[0] << " "<< pp[1] << " "<< pp[2] << std::endl;
// 	  std::cout << "Assembly Path: " << HexPicker->GetCellId() << std::endl;
	  vtkIdType cellId = HexPicker->GetCellId();
	  vtkIdList *facePointsIds = vtkIdList::New();
	  HexPolyData->GetCellPoints(cellId, facePointsIds);
	  
	  double p[4][3];
	  for(int i=0; i<facePointsIds->GetNumberOfIds(); i++)
	  {
	    HexPolyData->GetPoint(facePointsIds->GetId(i),p[i]);
// 	    std::cout << "\tPoint " << i << " (" << p[i][0] << "," << p[i][1] << "," << p[i][2] << ")" << std::endl;
	  }
	  
	  enum EDGE {TOP_EDGE, RIGHT_EDGE, BOTTOM_EDGE, LEFT_EDGE};
	  enum FACE {XY_FACE=4, XY_FACE_BACK=5, XZ_FACE_BACK=2, XZ_FACE=3, YZ_FACE_BACK=0, YZ_FACE=1};
	  	  
	  double edgeDist[4];
	  double pickTh = 10;
	  bool insideFace;
	  switch (cellId)
	  {
	    case XY_FACE:
	    case XY_FACE_BACK:
	      edgeDist[TOP_EDGE]   = p[0][1] - pp[1];//Edge from P0P3
	      edgeDist[RIGHT_EDGE] = pp[0] - p[3][0];//Edge from P3P2
	      edgeDist[BOTTOM_EDGE]= pp[1] - p[2][1];//Edge from P2P1
	      edgeDist[LEFT_EDGE]  = p[1][0] - pp[0];//Edge from P1P0
	      
	      insideFace = cellId==XY_FACE?
		edgeDist[TOP_EDGE] <= 0 && edgeDist[RIGHT_EDGE] <= 0 &&
		edgeDist[BOTTOM_EDGE] <= 0 && edgeDist[LEFT_EDGE] <= 0:
		edgeDist[TOP_EDGE] <= 0 && edgeDist[RIGHT_EDGE] >= 0 &&
		edgeDist[BOTTOM_EDGE] <= 0 && edgeDist[LEFT_EDGE] >= 0;
	      
	      if (abs(edgeDist[TOP_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[4];
		this->InteractionState = vtkBoxRepresentation::MoveF2;
	      } else if (abs(edgeDist[RIGHT_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[1];
		this->InteractionState = vtkBoxRepresentation::MoveF1;
	      } else if (abs(edgeDist[BOTTOM_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[2];
		this->InteractionState = vtkBoxRepresentation::MoveF3;
	      } else if (abs(edgeDist[LEFT_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[3];
		this->InteractionState = vtkBoxRepresentation::MoveF0;
	      }else if (insideFace)
	      {
		this->CurrentHandle == this->Handle[6];
		this->InteractionState = vtkBoxRepresentation::Translating;
	      }
	      break;
	    case XZ_FACE:
	    case XZ_FACE_BACK:
	      edgeDist[TOP_EDGE]   = pp[2] - p[2][2];//Edge from P2P3
	      edgeDist[RIGHT_EDGE] = p[1][0] - pp[0];//Edge from P1P2
	      edgeDist[BOTTOM_EDGE]= p[1][2] - pp[2];//Edge from P1P0
	      edgeDist[LEFT_EDGE]  = pp[0] - p[0][0];//Edge from P0P3

	      insideFace = cellId==XZ_FACE?
		edgeDist[TOP_EDGE] <= 0 && edgeDist[RIGHT_EDGE] <= 0 &&
		edgeDist[BOTTOM_EDGE] <= 0 && edgeDist[LEFT_EDGE] <= 0:
		edgeDist[TOP_EDGE] <= 0 && edgeDist[RIGHT_EDGE] >= 0 &&
		edgeDist[BOTTOM_EDGE] <= 0 && edgeDist[LEFT_EDGE] >= 0;
	      
	      if (abs(edgeDist[TOP_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[5];
		this->InteractionState = vtkBoxRepresentation::MoveF5;
	      } else if (abs(edgeDist[RIGHT_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[0];
		this->InteractionState = vtkBoxRepresentation::MoveF0;
	      } else if (abs(edgeDist[BOTTOM_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[4];
		this->InteractionState = vtkBoxRepresentation::MoveF4;
	      } else if (abs(edgeDist[LEFT_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[1];
		this->InteractionState = vtkBoxRepresentation::MoveF1;
	      }else if (insideFace)
	      {
		this->CurrentHandle == this->Handle[6];
		this->InteractionState = vtkBoxRepresentation::Translating;
	      }
	      break;
	    case YZ_FACE:
	    case YZ_FACE_BACK:
	      edgeDist[TOP_EDGE]   = pp[1] - p[1][1];//Edge from P1P2
	      edgeDist[RIGHT_EDGE] = p[1][2] - pp[2];//Edge from P1P2
	      edgeDist[BOTTOM_EDGE]= p[3][1] - pp[1];//Edge from P3P0
	      edgeDist[LEFT_EDGE]  = pp[2] - p[3][2];//Edge from P2P3
	      
	      insideFace = cellId==YZ_FACE?
		edgeDist[TOP_EDGE] <= 0 && edgeDist[RIGHT_EDGE] <= 0 &&
		edgeDist[BOTTOM_EDGE] <= 0 && edgeDist[LEFT_EDGE] <= 0:
		edgeDist[TOP_EDGE] >= 0 && edgeDist[RIGHT_EDGE] <= 0 &&
		edgeDist[BOTTOM_EDGE] >= 0 && edgeDist[LEFT_EDGE] <= 0;
	      
	      if (abs(edgeDist[TOP_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[3];
		this->InteractionState = vtkBoxRepresentation::MoveF3;
	      } else if (abs(edgeDist[RIGHT_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[4];
		this->InteractionState = vtkBoxRepresentation::MoveF4;
	      } else if (abs(edgeDist[BOTTOM_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[2];
		this->InteractionState = vtkBoxRepresentation::MoveF2;
	      } else if (abs(edgeDist[LEFT_EDGE]) < pickTh)
	      {
		this->CurrentHandle == this->Handle[5];
		this->InteractionState = vtkBoxRepresentation::MoveF5;
	      }else if (insideFace)
	      {
		this->CurrentHandle == this->Handle[6];
		this->InteractionState = vtkBoxRepresentation::Translating;
	      }
	      break;
	    default:
	      this->InteractionState = vtkBoxRepresentation::Outside;
	  };
// 	  std::cout << cellId << " " << edgeDist[0] << " " << edgeDist[1] << " " << edgeDist[2] << " " << edgeDist[3] << std::endl;
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

