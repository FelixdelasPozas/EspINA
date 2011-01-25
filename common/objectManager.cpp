#include "objectManager.h"

//ParaQ Includes
#include "pqPipelineSource.h"

ObjectManager::ObjectManager()
	: m_stack(NULL)
	, m_workingStack(NULL)
{
}

void ObjectManager::setStack(pqPipelineSource *stack)
{
	if (m_stack)
	{
		//TODO: Free previous stacks and state
		//pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
	}
	m_stack = stack;
	m_workingStack = stack;
}

pqPipelineSource *ObjectManager::visualizationStack()
{
	return m_stack;
}

pqPipelineSource *ObjectManager::workingStack()
{
	return m_workingStack;
}


void ObjectManager::addSegmentation(pqPipelineSource *segmentation)
{
	m_segmentations->push_back(segmentation);
	emit segmentationAdded(segmentation);
}
