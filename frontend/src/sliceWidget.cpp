#include "sliceWidget.h"

//Espina includes
#include "slicer.h"

//ParaQ includes
#include "pqTwoDRenderView.h"
#include "pqApplicationCore.h"
#include "pqActiveObjects.h"
#include "pqDisplayPolicy.h"
#include "pqObjectBuilder.h"
#include "pqPipelineRepresentation.h"
#include "vtkSMImageSliceRepresentationProxy.h"
#include "vtkSMIntVectorProperty.h"
#include "vtkSMViewProxy.h"
#include "vtkSMRenderViewProxy.h"
#include "vtkSMTwoDRenderViewProxy.h"
#include "vtkPVGenericRenderWindowInteractor.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkAbstractPicker.h"
#include "vtkPropCollection.h"
#include "vtkPVDataInformation.h"
#include "pqOutputPort.h"

//Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QSpinBox>
#include <QMouseEvent>

#include <QDebug>

#define HINTWIDTH 40

//-----------------------------------------------------------------------------
SliceWidget::SliceWidget(SliceBlender *input)
	: m_input(input)
	, m_view(NULL)
	, m_viewWidget(NULL)
	, m_rep(NULL)
	, m_slice(NULL)
	, m_init(false)
{
	m_controlLayout = new QHBoxLayout();
	m_scroll = new QScrollBar(Qt::Horizontal);
	m_scroll->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
	m_spin = new QSpinBox();
	m_spin->setMinimumWidth(HINTWIDTH);
	m_spin->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Preferred);
	QObject::connect(m_scroll,SIGNAL(valueChanged(int)),m_spin,SLOT(setValue(int)));
	QObject::connect(m_spin,SIGNAL(valueChanged(int)),m_scroll,SLOT(setValue(int)));
	QObject::connect(m_spin,SIGNAL(valueChanged(int)),m_input,SLOT(setSlice(int)));
	QObject::connect(m_input,SIGNAL(updated()),this,SLOT(updateRepresentation()));
	QObject::connect(m_input,SIGNAL(outputChanged(pqOutputPort *)),
			this,SLOT(setInput(pqOutputPort *)));
	m_controlLayout->addWidget(m_scroll);
	m_controlLayout->addWidget(m_spin);

	m_mainLayout = new QVBoxLayout();
	m_mainLayout->addLayout(m_controlLayout);
	setLayout(m_mainLayout);
}


//-----------------------------------------------------------------------------
SliceWidget::~SliceWidget()
{
	//Objects creted by pqObjectBuilder have to be destroyed by it
	if (m_view)	pqApplicationCore::instance()->getObjectBuilder()->destroy(m_view);

	if (m_rep) m_rep->Delete();
	if (m_slice) m_slice->Delete();
}

//-----------------------------------------------------------------------------
void SliceWidget::vtkWidgetMouseEvent(QMouseEvent *event)
{
	if (event->type() == QMouseEvent::MouseButtonPress &&
			event->buttons() == Qt::LeftButton)
	{
		//Use Render Window Interactor's Picker to find the world coordinates
		//of the stack
		vtkSMTwoDRenderViewProxy* view = vtkSMTwoDRenderViewProxy::SafeDownCast( 
				m_view->getProxy());
		vtkSMRenderViewProxy* renModule = view->GetRenderView();
		vtkRenderWindowInteractor *rwi = vtkRenderWindowInteractor::SafeDownCast(
				renModule->GetInteractor());
		if (!rwi) 
			return;
		
		int x_dis,y_dis, z_dis = 0.0; //Display coordinates
		rwi->GetEventPosition(x_dis,y_dis);
		vtkAbstractPicker *picker = rwi->GetPicker();
		if (!picker) 
			return;

		//Change coordinates acording the plane
		picker->Pick(x_dis,y_dis,z_dis,renModule->GetRenderer());
		double pos[3];//World coordinates
		picker->GetPickPosition(pos);
		std::cout << pos[0] << " " << pos[1] << " " << m_spin->value() << "\n";
		m_input->getOutput()->getDataInformation()->PrintSelf(std::cout,vtkIndent(0));
		//Get Spacing
		double sx, sy, sz;//Image Spacing

		int i, j, k;//Image coordinates
	}
}

//-----------------------------------------------------------------------------
void SliceWidget::connectToServer()
{
	//qDebug() << "Creating View";
	pqObjectBuilder *builder = pqApplicationCore::instance()->getObjectBuilder();
	pqServer * server= pqActiveObjects::instance().activeServer();
	m_view = qobject_cast<pqTwoDRenderView*>(builder->createView(
			  pqTwoDRenderView::twoDRenderViewType(),server));
	m_viewWidget = m_view->getWidget();
	QObject::connect(m_viewWidget,SIGNAL(mouseEvent(QMouseEvent *)),
			this,SLOT(vtkWidgetMouseEvent(QMouseEvent *)));
	m_mainLayout->insertWidget(0,m_viewWidget);//To preserve view order
}

//-----------------------------------------------------------------------------
void SliceWidget::disconnectFromServer()
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

//-----------------------------------------------------------------------------
void SliceWidget::updateRepresentation()
{
	m_view->render();
}


//-----------------------------------------------------------------------------
bool SliceWidget::initialize()
{
	m_init = false;
	if (m_view->getRepresentations().size() == 0)
		return m_init;

	pqPipelineRepresentation* pipelineRep = qobject_cast<pqPipelineRepresentation*>(m_view->getRepresentations()[0]);
	if (!pipelineRep) 
		return m_init;

	m_rep = vtkSMImageSliceRepresentationProxy::SafeDownCast(pipelineRep->getRepresentationProxy());
	if (!m_rep)
		return m_init;
	//m_rep->PrintSelf(std::cout,vtkIndent(0));

	vtkSMIntVectorProperty *sliceMode = vtkSMIntVectorProperty::SafeDownCast(m_rep->GetProperty("SliceMode"));
	if (!sliceMode)
	{
		return m_init;
	} else {
		sliceMode->SetElements1(m_plane);
	}

	m_slice = vtkSMIntVectorProperty::SafeDownCast(m_rep->GetProperty("Slice"));
	m_init = m_slice != NULL;

	m_rep->UpdateVTKObjects();
	return m_init;
}

void SliceWidget::setInput(pqOutputPort *opPort)
{
	pqDisplayPolicy *displayManager = pqApplicationCore::instance()->getDisplayPolicy();
	displayManager->setRepresentationVisibility(opPort,m_view,true);
}
