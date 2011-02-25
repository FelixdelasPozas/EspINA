#include "interfaces.h"

#include "pqPipelineSource.h"


pqOutputPort *IRenderable::outputPort()
{
  return m_source->getOutputPort(m_portNumber);
}

pqPipelineSource *IRenderable::sourceData()
{
  return m_source;
}

int IRenderable::portNumber()
{
  return m_portNumber;
}