/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>

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


#include "vtkPVEspinaView.h"

#include <QDebug>


#include "vtkObjectFactory.h"
#include "vtkLegendScaleActor.h"
#include "vtkRenderer.h"
#include <vtkDataRepresentation.h>
#include <vtkCommand.h>

#include "vtkEspinaView.h"
#include <vtkRenderWindow.h>

vtkStandardNewMacro(vtkPVEspinaView);
//----------------------------------------------------------------------------

vtkPVEspinaView::vtkPVEspinaView()
{
  this->SetCenterAxesVisibility(false);
  this->SetOrientationAxesVisibility(false);
  this->SetOrientationAxesInteractivity(false);
  this->SetInteractionMode(INTERACTION_MODE_2D);

  this->OverviewRenderer = vtkSmartPointer<vtkRenderer>::New();
  this->OverviewRenderer->SetViewport(0.75,0,1,0.25);
  this->GetRenderWindow()->AddRenderer(this->OverviewRenderer);

  qDebug() << this << ": Created";
}

//----------------------------------------------------------------------------
vtkPVEspinaView::~vtkPVEspinaView()
{
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::SetInteractionMode(int mode)
{
  if (mode == INTERACTION_MODE_3D)
    {
    mode = INTERACTION_MODE_2D;
    }
  this->Superclass::SetInteractionMode(mode);
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::SetAxisVisibility(bool val)
{
}

void vtkPVEspinaView::AddActor(vtkProp* prop)
{
  qDebug() << this << ": Adding actor to both renderers";
  OverviewRenderer->AddActor(prop);
  OverviewRenderer->ResetCamera();
  GetRenderer()->AddActor(prop);
}


//----------------------------------------------------------------------------
void vtkPVEspinaView::AddSample(vtkDataRepresentation* rep)
{
  AddRepresentation(rep);
  std::cout << "Add Sample to Espina View" << std::endl;
}

//----------------------------------------------------------------------------
void vtkPVEspinaView::RemoveSample(vtkDataRepresentation* rep)
{
  std::cout << "Remove Sample to Espina View" << std::endl;
}



//----------------------------------------------------------------------------
void vtkPVEspinaView::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
