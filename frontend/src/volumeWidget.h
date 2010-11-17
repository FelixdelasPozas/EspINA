#include <QWidget>

//Forward declaration
class pqRenderView;
class QScrollBar;
class QSpinBox;
class QWidget;
class QVBoxLayout;
class QHBoxLayout;

class VolumeWidget : public QWidget
{
	Q_OBJECT
public:
	VolumeWidget();
	~VolumeWidget();

	pqRenderView *getView(){return m_view;}

public slots:
	void connectToServer();
	void disconnectFromServer();

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
