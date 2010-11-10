#include "volumeWidget.h"

#include "pqTwoDRenderView.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqObjectBuilder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSpinBox>

VolumeWidget::VolumeWidget()
{
	QHBoxLayout *controlLayout = new QHBoxLayout();
	m_scroll = new QScrollBar(Qt::Horizontal);
	m_slice = new QSpinBox();
	controlLayout->addWidget(m_scroll);
	controlLayout->addWidget(m_slice);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	m_view = qobject_cast<pqTwoDRenderView*>(
		  pqApplicationCore::instance()->getObjectBuilder()->createView(
			  pqTwoDRenderView::twoDRenderViewType(),
			  pqActiveObjects::instance().activeServer()));
	mainLayout->addWidget(m_view->getWidget());
	mainLayout->addLayout(controlLayout);
	setLayout(mainLayout);
}


VolumeWidget::~VolumeWidget()
{
	//Objects creted by pqObjectBuilder have to be destroyed by it
	pqApplicationCore::instance()->getObjectBuilder()->destroy(m_view);
}
