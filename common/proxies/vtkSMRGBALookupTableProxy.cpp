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


#include "vtkSMRGBALookupTableProxy.h"

// ParaView
#include "vtkClientServerStream.h"
#include "vtkObjectFactory.h" 
#include "vtkProcessModule.h"                                                                                                          
#include "vtkSMIntVectorProperty.h"                                                                                                    
#include "vtkSMDoubleVectorProperty.h"                                                                                                 
//#include "vtkMath.h"         


vtkStandardNewMacro(vtkSMRGBALookupTableProxy); 

//---------------------------------------------------------------------------
vtkSMRGBALookupTableProxy::vtkSMRGBALookupTableProxy()
{
  this->SetVTKClassName("vtkLookupTable");
}

//---------------------------------------------------------------------------
vtkSMRGBALookupTableProxy::~vtkSMRGBALookupTableProxy()
{
  this->SetVTKClassName(0);
}

//---------------------------------------------------------------------------
void vtkSMRGBALookupTableProxy::CreateVTKObjects()
{
  if (this->ObjectsCreated)
  {                                                                                                                                  
    return;
  }
  this->SetServers(vtkProcessModule::CLIENT | 
    vtkProcessModule::RENDER_SERVER);
  this->Superclass::CreateVTKObjects();
  
}

//---------------------------------------------------------------------------
void vtkSMRGBALookupTableProxy::UpdateVTKObjects(vtkClientServerStream& stream)
{
    this->Superclass::UpdateVTKObjects(stream);
    this->Build();
}

void vtkSMRGBALookupTableProxy::SetTableValue(int id, double r, double g, double b, double a)
{
  vtkClientServerStream stream;

  stream << vtkClientServerStream::Invoke                                                                                            
         << this->GetID() << "SetNumberOfTableValues"                                                                                
         << 256
         << vtkClientServerStream::End;  
  
  stream << vtkClientServerStream::Invoke
  << this->GetID() << "SetTableValue" << id 
  << r << g << b << a
  << vtkClientServerStream::End;
  
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  pm->SendStream(this->ConnectionID, this->Servers, stream);
}

void vtkSMRGBALookupTableProxy::SetTableValue(int id, double rgba[4])
{
  const int OPAQUE = 1;
  SetTableValue(id, rgba[0], rgba[1], rgba[2], rgba[3]);
}


//---------------------------------------------------------------------------
void vtkSMRGBALookupTableProxy::Build()
{
  vtkClientServerStream stream;

  vtkSMProperty* p;
  vtkSMIntVectorProperty* intVectProp;
  vtkSMDoubleVectorProperty* doubleVectProp;
  
  stream << vtkClientServerStream::Invoke << this->GetID() 
  << "SetRampToLinear" 
  << vtkClientServerStream::End;
  
  // Background color
  //stream << vtkClientServerStream::Invoke
  //<< this->GetID() << "SetTableValue" << 0 << 0 << 0 << 0 << 1
  //<< vtkClientServerStream::End;
  
  /*
  stream << vtkClientServerStream::Invoke
  << this->GetID() << "SetTableValue" << 255 << 0 << 1 << 0 << 1
  << vtkClientServerStream::End;
  */
  
  vtkProcessModule* pm = vtkProcessModule::GetProcessModule();
  pm->SendStream(this->ConnectionID, this->Servers, stream);
}

void vtkSMRGBALookupTableProxy::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}





