#ifndef _SEGMENTATION_H_
#define _SEGMENTATION_H_

//Forward declarations
class pqPipelineSource;

typedef int Taxonomy;

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
};


// This class represents a segmenation object
class SegmentedObject : public Segmentation
{
public:
	SegmentedObject();
	~SegmentedObject();

	pqPipelineSource *workingStack();
	SegmentedObject *objects() {return this;}

private:
	Taxonomy type;
};

#endif// _SEGMENTATION_H_
