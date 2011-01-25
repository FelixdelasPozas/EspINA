#ifndef E_FILTER_H
#define E_FILTER_H

#include "interfaces.h"
#include "processingTrace.h"

class vtkSMProxy;

class ESegementation : public ISelectableObject, public Source
{
};

class EFilter : public ISelectableObject, public Filter
{
public:
  EFilter();
  
private:
  vtkSMProxy *m_proxy;
};

#endif// E_FILTER_H