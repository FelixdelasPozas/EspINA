#ifndef SEEDGROWREGIONSEGMENTATIONFILTER_H
#define SEEDGROWREGIONSEGMENTATIONFILTER_H

#include <filter.h>

class IVOI;

class SeedGrowRegionSegmentationFilter 
: public EspinaFilter
, public ITraceNode
{
public:
  //! Constructor interactivo  
  SeedGrowRegionSegmentationFilter(EspinaProduct *input, IVOI *voi, ITraceNode::Arguments &args);
  //! Constructor desde lista de argumentos
  SeedGrowRegionSegmentationFilter(ITraceNode::Arguments &args);
  
    virtual ~SeedGrowRegionSegmentationFilter();
  
  //! Implements IFilter Interface
  virtual int numProducts() {return m_numSeg;};
  virtual vtkProduct product(int i) {return vtkProduct(m_finalFilter->product(i).creator(),i);}
  virtual QList< vtkProduct* > products() {QList<vtkProduct*>a; return a;}
  virtual void removeProduct(EspinaProduct* product);

  virtual QString label() const {return getArgument("Type");}
  virtual QString getArgument ( QString name ) const  {return (name=="Type")?"SeedGrowRegionSegmentation::SeedGrowRegionSegmentationFilter":"";}
  virtual QString getArguments() {return m_args;}
  
private:
  EspinaFilter *m_applyFilter;
  vtkFilter *m_grow;
  EspinaFilter *m_restoreFilter;
  IFilter *m_finalFilter;
  int m_numSeg;
};

#endif // SEEDGROWREGIONSEGMENTATIONFILTER_H

