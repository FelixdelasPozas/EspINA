#include <QWidget>

//Forward declaration
class pqRenderView;
class QScrollBar;
class QSpinBox;

class VolumeWidget : public QWidget
{
public:
	VolumeWidget();
	~VolumeWidget();

	pqRenderView *getView(){return m_view;}
private:
	pqRenderView *m_view;
	QScrollBar *m_scroll;
	QSpinBox *m_slice;

protected:
		Q_DISABLE_COPY(VolumeWidget)
};


