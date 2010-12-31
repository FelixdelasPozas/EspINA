
#ifndef _SEGMENTATION_H_
#define _SEGMENTATION_H_

//Forward declarations
class pqPipelineSource;
class pqOutputPort;
#include <pqOutputPort.h> //TODO: Tmp
#include <pqPipelineSource.h> //TODO: Tmp


typedef double * Taxonomy;

class SegmentedObject;

// This class represents any kind of segmentation handled by ESPINA
class Segmentation
{

public:
	Segmentation(){}
	virtual ~Segmentation(){}

protected:
	Segmentation(const Segmentation &);
	Segmentation &operator=(const Segmentation &);

public:
	virtual pqPipelineSource *workingStack() = 0;
	virtual SegmentedObject *objects() = 0;
	virtual pqOutputPort *data() = 0;
};


// This class represents a segmenation object
class SegmentedObject : public Segmentation
{
public:
	SegmentedObject();
	//TODO: Tmp
    SegmentedObject (pqPipelineSource *source)
	{
	  m_pvData = source;
	  m_pvOutput = m_pvData->getOutputPort(0);
	}
	~SegmentedObject();

	pqPipelineSource *workingStack();
	SegmentedObject *objects() {return this;}
	pqOutputPort *data() {return m_pvOutput;}

private:
	Taxonomy semantic;
	pqPipelineSource *m_pvData;
	pqOutputPort *m_pvOutput;
	
};

#endif// _SEGMENTATION_H_
