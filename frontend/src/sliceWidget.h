#ifndef _SLICE_WIDGET_H
#define _SLICE_WIDGET_H

#include <QWidget>

//Forward declaration
class SliceBlender;
class pqTwoDRenderView;
class QScrollBar;
class QSpinBox;
class vtkSMImageSliceRepresentationProxy;
class vtkSMIntVectorProperty;
class QVBoxLayout;
class QHBoxLayout;
class pqOutputPort;

/// Displays a unique slice of a image's stack. 
/// The slice which is shown can be modified using the widget controls
class SliceWidget : public QWidget
{
	Q_OBJECT

public:
	SliceWidget(SliceBlender *input);
	~SliceWidget();


public:
		bool initialize();
		pqTwoDRenderView *getView(){return m_view;}
		//void showSource(pqOutputPort *opPort, bool visible);

protected slots:
	void vtkWidgetMouseEvent(QMouseEvent *event);

public slots:
	void connectToServer();
	void disconnectFromServer();

private:
	SliceBlender *m_input;
	pqTwoDRenderView *m_view;
	QScrollBar *m_scroll;
	QSpinBox *m_spin;
	QWidget *m_viewWidget;
	vtkSMImageSliceRepresentationProxy *m_rep;
	vtkSMIntVectorProperty *m_slice;
	QVBoxLayout *m_mainLayout;
	QHBoxLayout *m_controlLayout;
	bool m_init;
	int m_plane;

protected slots:
	void updateRepresentation();
	void setInput(pqOutputPort *opPort);

protected:
		Q_DISABLE_COPY(SliceWidget)
};

#endif// _SLICE_WIDGET_H
