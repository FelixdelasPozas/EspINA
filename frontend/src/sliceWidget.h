#include <QWidget>

//Forward declaration
class pqTwoDRenderView;
class QScrollBar;
class QSpinBox;
class vtkSMImageSliceRepresentationProxy;
class vtkSMIntVectorProperty;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;

class SliceWidget : public QWidget
{
	Q_OBJECT
public:
	SliceWidget();
	~SliceWidget();

	bool initialize();
	pqTwoDRenderView *getView(){return m_view;}

public slots:
	void setPlane(int plane);
	void connectToServer();
	void disconnectFromServer();

private:

private:
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

protected:
		Q_DISABLE_COPY(SliceWidget)

protected slots:
	void setSlice(int slice);

signals:
	void sliceChanged(int);
};


