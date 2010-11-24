#ifndef _SLICER_H_
#define _SLICER_H_

#include <QObject>

enum SlicePlane {SLICE_PLANE_XY, SLICE_PLANE_YZ, SLICE_PLANE_XZ, SLICE_PLANES};

//class Slicer
//{
//public:
//	virtual void addInput() = 0;
//	virtual int getOutput() = 0;
//public slots:
//	virtual void setPlane() = 0;
//};

//Forward declarations
class pqOutputPort;
class pqPipelineSource;
#include <QList> //TODO: Forward declaration

class SliceBlender : public QObject
{
	Q_OBJECT
public:
	SliceBlender(SlicePlane plane);

	void addInput(pqPipelineSource *source);
	pqOutputPort *getOutput();
public slots:
	void setPlane(){}
	void setSlice(int slice);

signals:
	void outputChanged(pqOutputPort *);
	void updated();

private:
	QList<pqPipelineSource *> *m_inputs;
	QList<pqPipelineSource *> *m_slicers;
	pqPipelineSource *m_blender;
	SlicePlane m_plane;
};


#endif//_SLICER_H_
