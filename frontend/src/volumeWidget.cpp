#include "volumeWidget.h"

#include "pqRenderView.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqObjectBuilder.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSpinBox>

#include <QDebug>

#define HINTWIDTH 40

//-----------------------------------------------------------------------------
VolumeWidget::VolumeWidget()
	: m_view(NULL)
	, m_viewWidget(NULL)
	, m_init(false)
{
	m_controlLayout = new QHBoxLayout();
	m_scroll = new QScrollBar(Qt::Horizontal);
	m_scroll->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	m_slice = new QSpinBox();
	m_slice->setMinimumWidth(HINTWIDTH);
	m_slice->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	m_controlLayout->addWidget(m_scroll);
	m_controlLayout->addWidget(m_slice);

	m_mainLayout = new QVBoxLayout();
	m_mainLayout->addLayout(m_controlLayout);
	setLayout(m_mainLayout);
}

//-----------------------------------------------------------------------------
VolumeWidget::~VolumeWidget()
{
	//Objects creted by pqObjectBuilder have to be destroyed by it
	if (m_view)	
		pqApplicationCore::instance()->getObjectBuilder()->destroy(m_view);
}

//-----------------------------------------------------------------------------
void VolumeWidget::connectToServer()
{
	qDebug() << "Creating View";
	pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
	pqServer * server= pqActiveObjects::instance().activeServer();
	m_view = qobject_cast<pqRenderView*>(builder->createView(
			  pqRenderView::renderViewType(), server));
	m_viewWidget = m_view->getWidget();
	m_mainLayout->insertWidget(0,m_viewWidget);//To preserver view order
}

//-----------------------------------------------------------------------------
void VolumeWidget::disconnectFromServer()
{
	if (m_view)
	{
		//qDebug() << "Deleting Widget";
		m_mainLayout->removeWidget(m_viewWidget);
		//qDebug() << "Deleting View";
		//TODO: BugFix -> destroy previous instance of m_view
		//pqApplicationCore::instance()->getObjectBuilder()->destroy(m_view);
	}
}


