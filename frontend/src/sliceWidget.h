#include <QWidget>

//Forward declaration
class pqTwoDRenderView;
class QScrollBar;
class QSpinBox;

class SliceWidget : public QWidget
{
public:
	SliceWidget();
	~SliceWidget();

	pqTwoDRenderView *getView(){return m_view;}
private:
	pqTwoDRenderView *m_view;
	QScrollBar *m_scroll;
	QSpinBox *m_slice;

protected:
		Q_DISABLE_COPY(SliceWidget)
};


