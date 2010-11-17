#include "volumeWidget.h"

#include "pqRenderView.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqObjectBuilder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSpinBox>

#define HINTWIDTH 40

VolumeWidget::VolumeWidget()
{
	QHBoxLayout *controlLayout = new QHBoxLayout();
	m_scroll = new QScrollBar(Qt::Horizontal);
	m_scroll->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	m_slice = new QSpinBox();
	m_slice->setMinimumWidth(HINTWIDTH);
	m_slice->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	controlLayout->addWidget(m_scroll);
	controlLayout->addWidget(m_slice);

	QVBoxLayout *mainLayout = new QVBoxLayout();
	m_view = qobject_cast<pqRenderView*>(
		  pqApplicationCore::instance()->getObjectBuilder()->createView(
			  pqRenderView::renderViewType(),
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
