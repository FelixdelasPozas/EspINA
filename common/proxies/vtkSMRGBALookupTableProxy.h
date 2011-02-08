/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
 * 
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef VTK_SM_RGBA_LOOKUP_TABLE_PROXY_H
#define VTK_SM_RGBA_LOOKUP_TABLE_PROXY_H

#include "vtkSMProxy.h"


class  VTK_EXPORT vtkSMRGBALookupTableProxy : public vtkSMProxy
{
public:
  static vtkSMRGBALookupTableProxy * New();  
  vtkTypeMacro(vtkSMRGBALookupTableProxy, vtkSMProxy);  
  void PrintSelf(ostream& os, vtkIndent indent);
  
  //! Create the Lookup Table values
  void Build();
  
  //! Push properties to VtkObjects and also build the LUT
  virtual void UpdateVTKObjects()
  {this->Superclass::UpdateVTKObjects(); }
  
  //! Add a new colour entry to the Lookup Table
  void SetTableValue(int id, double rgba[4]);
  void SetTableValue(int id, double r, double g, double b, double a);
  
  
protected:                                                                                                                             
  vtkSMRGBALookupTableProxy();                                                                                                             
  virtual ~vtkSMRGBALookupTableProxy();
  
  //! push properties to VTK object.                                                                                                    
  //! Also call Build(), hence rebuilds the lookup table.                                                                               
  virtual void UpdateVTKObjects(vtkClientServerStream& stream);                                                                        
  
  //! This method is overridden to change the servers.                                                                                  
  virtual void CreateVTKObjects();  
  
private:                                                                                                                               
  vtkSMRGBALookupTableProxy(const vtkSMProxy& ); // Not implemented
  void operator=(const vtkSMRGBALookupTableProxy&); // Not implemented      
};

#endif // VTK_SM_RGBA_LOOKUP_TABLE_PROXY_H
