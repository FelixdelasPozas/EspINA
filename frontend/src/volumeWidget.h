#include <QWidget>

//Forward declaration
class pqRenderView;
class QScrollBar;
class QSpinBox;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;
class pqOutputPort;

class VolumeWidget : public QWidget
{
	Q_OBJECT
public:
	VolumeWidget();
	~VolumeWidget();

	pqRenderView *getView(){return m_view;}
	void showSource(pqOutputPort *opPort, bool visible);

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
