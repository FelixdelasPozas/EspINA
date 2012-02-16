#include "SeedGrowRegionSegmentationFilter.h"

#include "cache/cachedObjectBuilder.h"

#include <espINAFactory.h>
#include <espina.h>
#include <QDebug>

QString stripName(QString args){return args.split(";")[0];}//FAKE
QString stripArgs(QString args){return args.split(";")[1];}//FAKE

//-----------------------------------------------------------------------------
SeedGrowRegionSegmentationFilter::SeedGrowRegionSegmentationFilter ( EspinaProduct* input, IVOI* voi, ITraceNode::Arguments& args )
: m_applyFilter(NULL)
, m_grow(NULL)
, m_restoreFilter(NULL)
, m_finalFilter(NULL)
{
  type = FILTER;
  ProcessingTrace* trace = ProcessingTrace::instance();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  //m_args = QString("%1=%2;").arg("Sample").arg(input->label());
  m_args = ESPINA_ARG("Sample", input->getArgument("Id"));
  foreach(QString argName, args.keys())
  {
    m_args.append(ESPINA_ARG(argName, args[argName]));
  }
  
  vtkProduct voiOutput(input->creator(),input->portNumber());
  //! Executes VOI
  if (voi)
  {
    m_applyFilter = voi->applyVOI(input);
    if (m_applyFilter)
    {
      voiOutput = m_applyFilter->product(0);
      m_args.append(ESPINA_ARG("ApplyVOI", "["+m_applyFilter->getFilterArguments() + "]"));
    }
  }

  //! Execute Grow Filter
  vtkFilter::Arguments growArgs;
  growArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, voiOutput.id()));
  growArgs.push_back(vtkFilter::Argument(QString("Seed"),vtkFilter::INTVECT,args["Seed"]));
  growArgs.push_back(vtkFilter::Argument(QString("Threshold"),vtkFilter::DOUBLEVECT,args["Threshold"]));
  //growArgs.push_back(vtkFilter::Argument(QString("ProductPorts"),vtkFilter::INTVECT, "0"));
  m_grow = cob->createFilter("filters","SeedGrowRegionSegmentationFilter",growArgs);
  
  //! Create segmenations. SeedGrowRegionSegmentationFilter has only 1 output
  assert(m_grow->numProducts() == 1);
  
  m_finalFilter = m_grow;
  
  vtkProduct growOutput = m_grow->product(0);
  //! Restore possible VOI transformation
  if (voi)
  {
    m_restoreFilter = voi->restoreVOITransormation(&growOutput);
    if (m_restoreFilter)
    {
      growOutput = m_restoreFilter->product(0);
      m_finalFilter = m_restoreFilter;
      //TODO Anadir args
      
    }
  }

  assert(m_finalFilter->numProducts() == 1);
  m_numSeg = m_finalFilter->numProducts();
  
  Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &m_finalFilter->product(0));

  // Trace EspinaFilter
  trace->addNode(this);
  // Connect input
  trace->connect(input,this,"Sample");
  // Trace Segmentation
  trace->addNode(seg);
  // Trace connection
  trace->connect(this, seg,"Segmentation");
  
  EspINA::instance()->addSegmentation(seg);
}

//-----------------------------------------------------------------------------
SeedGrowRegionSegmentationFilter::SeedGrowRegionSegmentationFilter ( ITraceNode::Arguments& args )
: m_applyFilter(NULL)
, m_grow(NULL)
, m_restoreFilter(NULL)
, m_finalFilter(NULL)
{
  foreach(QString key, args.keys())
  {
    if( key == "ApplyVOI" )
      m_args.append(ESPINA_ARG(key, "["+ args[key] + "]"));
    else
      m_args.append(ESPINA_ARG(key, args[key]));
  }
  type = FILTER;
  ProcessingTrace* trace = ProcessingTrace::instance();
  CachedObjectBuilder *cob = CachedObjectBuilder::instance();

  vtkProduct input(args["Sample"]);

  vtkProduct voiOutput(input.creator(),input.portNumber());
  //! Executes VOI
  if (args.contains("ApplyVOI") )
  {
    ITraceNode::Arguments voiArgs = ITraceNode::parseArgs(args["ApplyVOI"]);
    m_applyFilter = trace->getRegistredPlugin(voiArgs["Type"])->createFilter(voiArgs["Type"],voiArgs);
    if (m_applyFilter)
    {
      voiOutput = m_applyFilter->product(0);
      //m_args.append("ApplyVOI=" + applyFilter->getFileArguments());
      //m_args.append(ESPINA_ARG("ApplyVOI", "["+m_applyFilter->getFilterArguments() + "]"));
    }
  }

  //! Execute Grow Filter
  vtkFilter::Arguments growArgs;
  growArgs.push_back(vtkFilter::Argument(QString("Input"),vtkFilter::INPUT, voiOutput.id()));
  growArgs.push_back(vtkFilter::Argument(QString("Seed"),vtkFilter::INTVECT,args["Seed"]));
  growArgs.push_back(vtkFilter::Argument(QString("Threshold"),vtkFilter::DOUBLEVECT,args["Threshold"]));
  //growArgs.push_back(vtkFilter::Argument(QString("ProductPorts"),vtkFilter::INTVECT, "0"));
  // Disk cache. If the .seg contains .mhd files now it try to load them
//   Cache::Index id = cob->generateId("filter", "SeedGrowSegmentationFilter", growArgs);
//   m_grow = cob->getFilter(id);
//   if( !m_grow )
  m_grow = cob->createFilter("filters","SeedGrowRegionSegmentationFilter",growArgs);
  
  //! Create segmenations. SeedGrowSegmentationFilter has only 1 output
  assert(m_grow->numProducts() == 1);

  m_finalFilter = m_grow;

  vtkProduct growOutput = m_grow->product(0);
  //! Restore possible VOI transformation
  if (args.contains("RestoreVOI"))
  {
    
    //TODO: Restore
//     m_restoreFilter = voi->restoreVOITransormation(&growOutput);
//     if (m_restoreFilter)
//     {
//       growOutput = m_restoreFilter->product(0);
//       m_finalFilter = m_restoreFilter;
//       //Anadir args
//     }
  }

  assert(m_finalFilter->numProducts() == 1);
  m_numSeg = m_finalFilter->numProducts();

  Segmentation *seg = EspINAFactory::instance()->CreateSegmentation(this, &m_finalFilter->product(0));

  // Trace EspinaFilter
  trace->addNode(this);
  // Connect input
  trace->connect(args["Sample"],this,"Sample");
  // Trace Segmentation
  trace->addNode(seg);
  // Trace connection
  trace->connect(this, seg,"Segmentation");

  EspINA::instance()->addSegmentation(seg);

}

//-----------------------------------------------------------------------------
SeedGrowRegionSegmentationFilter::~SeedGrowRegionSegmentationFilter()
{
  qDebug() << "Desrtoying filter";
  CachedObjectBuilder* cob = CachedObjectBuilder::instance();
  if( m_restoreFilter )
    delete m_restoreFilter;
  if (m_grow)
    cob->removeFilter(m_grow);
  if (m_applyFilter)
    delete m_applyFilter;
}

//-----------------------------------------------------------------------------
void SeedGrowRegionSegmentationFilter::removeProduct ( EspinaProduct* product )
{
  m_numSeg = 0;
}
