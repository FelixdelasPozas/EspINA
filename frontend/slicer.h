#ifndef _SLICER_H_
#define _SLICER_H_

#include <QObject>
#include <interfaces.h>

class Stack;
class Segmentation;
class vtkImageMapToColors;

enum SlicePlane 
{
	SLICE_PLANE_FIRST = 0
	, SLICE_PLANE_XY = 0
	, SLICE_PLANE_YZ = 1
	, SLICE_PLANE_XZ = 2
	, SLICE_PLANE_LAST = 2
};

enum SliceAxis 
{
	SLICE_AXIS_FIRST = 0
	, SLICE_AXIS_X = 0
	, SLICE_AXIS_Y = 1
	, SLICE_AXIS_Z = 2
	, SLICE_AXIS_LAST = 2
};


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

/// This class takes several volume images and slice them
/// according to a given plane, blending their output in a
/// single slice
class SliceBlender : public QObject
{
	Q_OBJECT

//public:
//	enum Blender {BLENDER_OFF=0, BLENDER_ON=1};
public:
	/// Creates a SliceBlender filter in *plane* 
	SliceBlender(SlicePlane plane);

public:
	/// Set stack as background
	void setBackground(IRenderable *background); //-> IRendered
	/// Add seg to the set of inputs to blend
	void addSegmentation(IRenderable *seg); //-> IRenderable

	/// Gets the filter output. Depending on the behaviour
	/// of the filter it can show only the first one (BLENDING_OFF)
	/// or a slice containing a blend of all the inputs (BLENDING_ON)
	pqOutputPort *getOutput();
	pqOutputPort *getBgOutput();

	/// Return the number of slices this filter handles
	int getNumSlices();

	int getAxisX() {return m_xAxis;}
	int getAxisY() {return m_yAxis;}
	int getAxisZ() {return m_zAxis;}

	double getBound(int index){return m_bounds[index];}
	int getExtent(int index){return m_extent[index];}

public slots:
	void setPlane(SlicePlane plane)
	{
		//TODO: Update all slice modes for all inputs
		m_plane = plane;
	}
	void setSlice(int slice);
	void setBlending(bool blending)
	{
	  m_blending = blending;
	  emit outputChanged(getOutput());
	  emit updated();
	}

signals:
	/// This signal is triggered when the internal state of the filter
	/// has been changed
	void updated();

	/// This signal is triggered when the blending is activated/deactivated
	/// Note that this doesn't require to update the pipeline
	void outputChanged(pqOutputPort *);

private:
	void updateAxis();

private:
	IRenderable *m_background;
	pqPipelineSource *m_bgSlicer;
	pqPipelineSource *m_bgMapper;
	QList</*Segmentation*/ IRenderable *> *m_segmentations;
	QList<pqPipelineSource *> *m_slicers;
	QList<pqPipelineSource *> *m_sliceMappers;
	pqPipelineSource *m_blender;
	SlicePlane m_plane;
	bool m_blending;
	double m_bounds[6];
	int m_extent[6];
	int m_xAxis, m_yAxis, m_zAxis;

protected:
		Q_DISABLE_COPY(SliceBlender)
};


#endif//_SLICER_H_
