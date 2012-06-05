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


#include "vtkAppositionPlaneFeatures.h"

#include <vtkObjectFactory.h>
#include <vtkPolyData.h>
#include <vtkMeshQuality.h>
#include <vtkInformationVector.h>
#include <vtkInformation.h>
#include <vtkIdTypeArray.h>
#include <vtkIdList.h>
#include <vtkEdgeListIterator.h>
#include <vtkMutableUndirectedGraph.h>
#include <vtkBoostConnectedComponents.h>
#include <vtkDataSetAttributes.h>
#include <vtkMath.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkAppositionPlaneFeatures);

//-----------------------------------------------------------------------------
vtkAppositionPlaneFeatures::vtkAppositionPlaneFeatures()
: Area(0.0)
, Perimeter(0.0)
{
}

//-----------------------------------------------------------------------------
vtkAppositionPlaneFeatures::~vtkAppositionPlaneFeatures()
{
}


//-----------------------------------------------------------------------------
double vtkAppositionPlaneFeatures::area(vtkPolyData* mesh)
{
  mesh->Update();
  int nc = mesh->GetNumberOfCells();
  double totalArea = 0.0;
//   std::cout << "Number of Cells: " << nc << std::endl;
  for (int i = 0; i < nc; i++)
    totalArea += vtkMeshQuality::TriangleArea(mesh->GetCell(i));
  
  return totalArea;
}

//-----------------------------------------------------------------------------
bool isPerimeter(vtkIdType cellId, vtkIdType p1, vtkIdType p2, vtkPolyData *mesh) 
{
    vtkSmartPointer<vtkIdList> neighborCellIds = vtkSmartPointer<vtkIdList>::New();
    mesh->GetCellEdgeNeighbors(cellId, p1, p2, neighborCellIds);
    //std::cout << "Neigh: " <<  neighborCellIds->GetNumberOfIds() << std::endl;
    return (neighborCellIds->GetNumberOfIds() == 0);
}

//-----------------------------------------------------------------------------
double vtkAppositionPlaneFeatures::perimeter(vtkPolyData* mesh)
{
  double totalPerimeter = 0.0;
  try
  {
    mesh->BuildLinks();
    
    //  std::cout << "Points: " << mesh->GetNumberOfPoints() << std::endl;
    int num_of_cells = mesh->GetNumberOfCells();
    
    vtkSmartPointer<vtkMutableUndirectedGraph> graph = 
    vtkSmartPointer<vtkMutableUndirectedGraph>::New();
    
    vtkSmartPointer<vtkIdTypeArray> pedigree = vtkSmartPointer<vtkIdTypeArray>::New();
    graph->GetVertexData()->SetPedigreeIds(pedigree);
    
    for (vtkIdType cellId = 0; cellId < num_of_cells; cellId++) {
      vtkSmartPointer<vtkIdList> cellPointIds = vtkSmartPointer<vtkIdList>::New();
      mesh->GetCellPoints(cellId, cellPointIds);
      for(vtkIdType i = 0; i < cellPointIds->GetNumberOfIds(); i++) {
	vtkIdType p1; 
	vtkIdType p2; 
	
	p1 = cellPointIds->GetId(i);
	if(i+1 == cellPointIds->GetNumberOfIds())
	  p2 = cellPointIds->GetId(0);
	else
	  p2 = cellPointIds->GetId(i+1);
	
	if (isPerimeter(cellId,p1,p2, mesh)){
	  vtkIdType i1 = graph->AddVertex(p1);
	  vtkIdType i2 = graph->AddVertex(p2);
	  
	  graph->AddEdge(i1, i2);
	  
	  //              if (graph->GetDegree(i1) > 2) std::cout << "cell: "<< cellId << " "<< i << " OJETE i1: " << i1 << std::endl;
	  //              if (graph->GetDegree(i2) > 2) std::cout << "cell: "<< cellId << " "<< i << " OJETE i2: " << i2 << std::endl;
	}   
      }   
    }   
    
    //  std::cout << "Vertices: " << graph->GetNumberOfVertices() << std::endl;
    //  std::cout << "Edges: " << graph->GetNumberOfEdges() << std::endl;
    //  std::cout << "Pedigree: " << pedigree->GetNumberOfTuples() << std::endl;
    
    vtkSmartPointer<vtkBoostConnectedComponents> connectedComponents =
    vtkSmartPointer<vtkBoostConnectedComponents>::New();
    connectedComponents->SetInput(graph);
    connectedComponents->Update();
    vtkGraph* outputGraph = connectedComponents->GetOutput();
    vtkIntArray* components = vtkIntArray::SafeDownCast( outputGraph->GetVertexData()->GetArray("component"));
    
    double * range = components->GetRange(0);
    //  std::cout << "components: " << range[0] << " to "<< range[1] << std::endl;
    
    std::vector<double> perimeters;
    perimeters.resize(range[1]+1);
    
    vtkSmartPointer<vtkEdgeListIterator> edgeListIterator = vtkSmartPointer<vtkEdgeListIterator>::New();
    graph->GetEdges(edgeListIterator);
    
    while(edgeListIterator->HasNext()) {
      vtkEdgeType edge = edgeListIterator->Next();
      
      if (components->GetValue(edge.Source) != components->GetValue(edge.Target) ) { 
	throw "ERROR: Edge between 2 disconnected component";
      }   
      double x[3];
      double y[3];
      mesh->GetPoint(pedigree->GetValue(edge.Source), x);
      mesh->GetPoint(pedigree->GetValue(edge.Target), y);
      
      //      std::cout << "X: " << x[0] << " " << x[1] << " " << x[2] << " Point: " << pedigree->GetValue(edge.Source) << std::endl;
      //      std::cout << "Y: " << y[0] << " " << y[1] << " " << y[2] << " Point: " << pedigree->GetValue(edge.Target) << std::endl;
      
      double edge_length = sqrt(vtkMath::Distance2BetweenPoints(x, y));
      //      std::cout << edge_length << std::endl;
      perimeters[components->GetValue(edge.Source)] += edge_length;
      //perimeters[components->GetValue(edge.Source)] += 1;
    }
    
    double sum_per = 0.0;
    double max_per = 0.0;
    //  std::cout << "myvector contains:";
    for (unsigned int i=0;i<perimeters.size();i++) {
      //      std::cout << " " << perimeters[i];
      sum_per += perimeters[i];
      max_per = perimeters[i] > max_per ? perimeters[i] : max_per;
    }
    totalPerimeter = max_per;
  }catch (...)
  {
    std::cerr << "Couldn't compute perimter" << std::endl;
  }
  
  //  std::cout << " SUM: " << sum_per << endl;
  
  return totalPerimeter;
}

//-----------------------------------------------------------------------------
int vtkAppositionPlaneFeatures::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation *appPlaneInfo = inputVector[0]->GetInformationObject(0);
  
  vtkPolyData *appPlane = vtkPolyData::SafeDownCast(
    appPlaneInfo->Get(vtkDataObject::DATA_OBJECT()) );
  
  appPlane->Update();
  
  Area = area(appPlane);
  Perimeter = perimeter(appPlane);
  
  return 1;
}
