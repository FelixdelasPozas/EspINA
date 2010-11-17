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

	pqTwoDRenderView *getView(){return m_view;}
	void setPlane(int plane);

private:
	bool initialize();

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


protected:
		Q_DISABLE_COPY(SliceWidget)

protected slots:
	void setSlice(int slice);
public slots:
	void connectToServer();
	void disconnectFromServer();

signals:
	void sliceChanged(int);
};


