#include <QWidget>
#include <QList>

//Forward declaration
class pqRenderView;
class QWidget;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;
class pqOutputPort;
class Segmentation;

#include "slicer.h"

enum Rep3D 
{
	POINTS = 0
	, SURFACE = 2
	, OUTLINE = 3
	, VOLUME = 4
	, SLICE = 6
	, HIDEN = 100
};

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
	void setPlane(pqOutputPort *opPort, const SlicePlane plane);
	//
	void showSource(pqOutputPort *opPort, Rep3D rep);

public slots:
	void connectToServer();
	void disconnectFromServer();
	void updateRepresentation();
	// Show/Hide axial planes
	void showPlanes(bool value);

private:
	pqRenderView *m_view;
	QWidget *m_viewWidget;
	QToolButton *m_togglePlanes;
	QToolButton *m_toggle3D;
	QVBoxLayout *m_mainLayout;
	QHBoxLayout *m_controlLayout;

	bool m_init;
	pqOutputPort *m_planes[SLICE_AXIS_LAST+1];
	bool m_showPlanes;
	QList<Segmentation *> m_valid;
	QList<Segmentation *> m_rejeted;
	QList<Segmentation *> m_userSelection;
};
