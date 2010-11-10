#include <QWidget>

//Forward declaration
class pqTwoDRenderView;
class QScrollBar;
class QSpinBox;

class VolumeWidget : public QWidget
{
public:
	VolumeWidget();
	~VolumeWidget();

	pqTwoDRenderView *getView(){return m_view;}
private:
	pqTwoDRenderView *m_view;
	QScrollBar *m_scroll;
	QSpinBox *m_slice;

protected:
		Q_DISABLE_COPY(VolumeWidget)
};


