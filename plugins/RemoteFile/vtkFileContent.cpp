#include "vtkFileContent.h"

#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkFileContent);
vtkCxxRevisionMacro(vtkFileContent,"$Revision: 1.46 $");
 
//---------------------------------------------------------------------------
vtkFileContent::vtkFileContent()
{
//  this->SetContent(0);
//  this->Content = NULL;
  this->Content = NULL;
}

//---------------------------------------------------------------------------
vtkFileContent::~vtkFileContent()
{
 //this->SetContent(0);
}
 
//---------------------------------------------------------------------------
void vtkFileContent::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );
}
 
//---------------------------------------------------------------------------
// void vtkFileContent::SetContent(const char* _arg)
// {
//   //vtkDebugMacro(<< this->GetClassName() << " (" << this << "): setting " << content " to " << (_arg?_arg:"(null)") );
//   if ( this->Content == NULL && _arg == NULL) { return;} 
//   if ( this->Content && _arg && (!strcmp(this->Content,_arg))) { return;} 
//   if (this->Content) { delete [] this->Content; } 
//   if (_arg) 
//     { 
//     std::size_t n = strlen(_arg) + 1; 
//     char *cp1 =  new char[n]; 
//     const char *cp2 = (_arg); 
//     this->Content = cp1; 
//     do { *cp1++ = *cp2++; } while ( --n ); 
//     } 
//    else 
//     { 
//     this->Content = NULL; 
//     } 
//   this->Modified(); 
// } 
