#include <QWidget>

//Forward declaration
class pqRenderView;
class QScrollBar;
class QSpinBox;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class pqOutputPort;

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
public:
	VolumeWidget();
	~VolumeWidget();

	pqRenderView *getView(){return m_view;}
	void showSource(pqOutputPort *opPort, Rep3D rep);

public slots:
	void connectToServer();
	void disconnectFromServer();
	void updateRepresentation();

private:
	pqRenderView *m_view;
	QScrollBar *m_scroll;
	QSpinBox *m_slice;
	QWidget *m_viewWidget;
	QVBoxLayout *m_mainLayout;
	QHBoxLayout *m_controlLayout;
	bool m_init;

protected:
		Q_DISABLE_COPY(VolumeWidget)
};
