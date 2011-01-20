#include <QWidget>
#include <QList>

//Forward declaration
class SliceBlender;
class pqRenderView;
class QWidget;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class pqOutputPort;
class Segmentation;
class Renderer;

#include "slicer.h"

enum Rep3D 
{
	POINTS = 0
	, SURFACE = 2
	, MESH = 2
	, OUTLINE = 3
	, VOLUME = 4
	, SLICE = 6
	, HIDEN = 100
};

typedef Segmentation Actor;

class VolumeWidget : public QWidget
{
  Q_OBJECT

protected:
  Q_DISABLE_COPY(VolumeWidget)

public:
	VolumeWidget();
	~VolumeWidget();

	pqRenderView *getView(){return m_view;}
	// Set axial plane's input
	void setPlane(SliceBlender *slice, const SlicePlane plane);

	//void showSource(pqOutputPort *opPort, Rep3D rep);
	void setValidActors(const QList<Actor *> *segmentations) {m_valid = segmentations;}
	void setRejectedActors(const QList<Actor *> *segmentations) {m_rejected = segmentations;}
	void setUserSelection(const QList<Actor *> *segmentations) {m_userSelection = segmentations;}

public slots:
	void connectToServer();
	void disconnectFromServer();
	void updateRepresentation();
	// Show/Hide axial planes
	void showPlanes(bool value);
	// Show/Hide scene actors
	void showActors(bool value);
	// Renderer Selection
	void setMeshRenderer(); 
	void setVolumeRenderer(); 

private:
	void renderValidActors();
	void renderRejected();
	void renderUserSelection();

private:
	pqRenderView *m_view;
	QWidget *m_viewWidget;
	QToolButton *m_togglePlanes;
	QToolButton *m_toggleActors;
	QVBoxLayout *m_mainLayout;
	QHBoxLayout *m_controlLayout;

	bool m_init;
	bool m_showPlanes;
	bool m_showActors;
	Renderer *m_renderer;
	SliceBlender *m_planes[SLICE_AXIS_LAST+1];

	const QList<Actor *> *m_valid;
	const QList<Actor *> *m_rejected;
	const QList<Actor *> *m_userSelection;
};
