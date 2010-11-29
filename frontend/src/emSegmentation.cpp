#include "emSegmentation.h"

//ParaQ Includes
#include "pqPipelineSource.h"

EMSegmentation::EMSegmentation()
	: m_stack(NULL)
	, m_workingStack(NULL)
{
}

void EMSegmentation::setStack(pqPipelineSource *stack)
{
	if (m_stack)
	{
		//TODO: Free previous stacks and state
		//pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
	}
	m_stack = stack;
	m_workingStack = stack;
}

pqPipelineSource *EMSegmentation::visualizationStack()
{
	return m_stack;
}

pqPipelineSource *EMSegmentation::workingStack()
{
	return m_workingStack;
}


void EMSegmentation::addSegmentation(pqPipelineSource *segmentation)
{
	m_segmentations->push_back(segmentation);
	emit segmentationAdded(segmentation);
}
