/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "SegmentationInspector.h"

#include <QFileDialog>
#include <QDebug>

#include <pqObjectBuilder.h>
#include <pqApplicationCore.h>
#include <pqActiveObjects.h>
#include <pqRenderView.h>
#include <pqDisplayPolicy.h>
#include <pqDataRepresentation.h>
#include <pqPipelineRepresentation.h>
#include <pqViewExporterManager.h>
#include <vtkSMRenderViewProxy.h>

#include <model/ModelItem.h>
#include <model/Segmentation.h>
#include <model/Filter.h>

QMap<Segmentation *, SegmentationInspector *> SegmentationInspector::m_inspectors;

//------------------------------------------------------------------------
SegmentationInspector* SegmentationInspector::CreateInspector(Segmentation* seg)
{
  SegmentationInspector *inspector;

  if (m_inspectors.contains(seg))
    inspector = m_inspectors[seg];
  else
    inspector = new SegmentationInspector(seg);

  return inspector;
}


//------------------------------------------------------------------------
SegmentationInspector::SegmentationInspector(Segmentation *seg, QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f)
, m_view(NULL)
, m_seg(seg)
{
  setupUi(this);
  m_renderPlane->setVisible(false);

  QIcon iconSave = qApp->style()->standardIcon(QStyle::SP_DialogSaveButton);
  m_export->setIcon(iconSave);
  if (!m_view)
  {
    pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
    pqServer * server= pqActiveObjects::instance().activeServer();
    m_view = qobject_cast<pqRenderView*>(ob->createView( pqRenderView::renderViewType(), server));

    this->viewLayout->insertWidget(0,m_view->getWidget());
    m_view->setCenterAxesVisibility(false);
    double black[3] = {0,0,0};
    //TODO: OLDVERSION m_view->getRenderViewProxy()->SetBackgroundColorCM(black);

//     m_informationView->setModel(EspINA::instance());
//     m_informationView->setRootIndex(EspINA::instance()->segmentationIndex(seg));

    ModelItem::Vector filters = seg->relatedItems(ModelItem::IN, "CreateSegmentation");
    Q_ASSERT(filters.size() > 0);
    Filter *filter = dynamic_cast<Filter *>(filters.first());
    Q_ASSERT(filter);
    this->pluginLayout->insertWidget(0, filter->createConfigurationWidget());

    connect(m_snapshot,SIGNAL(clicked(bool)),this,SLOT(takeSnapshot()));
    connect(m_export,SIGNAL(clicked(bool)),this,SLOT(exportScene()));
    connect(m_renderVolumetric,SIGNAL(clicked(bool)),this,SLOT(updateScene()));
    connect(m_renderMesh,SIGNAL(clicked(bool)),this,SLOT(updateScene()));
//     connect(m_renderPlane,SIGNAL(clicked(bool)),this,SLOT(updateScene()));

    m_renderVolumetric->setChecked(false);
    m_renderMesh->setChecked(true);
    m_renderPlane->setChecked(false);
  }

  updateScene();

  m_inspectors[m_seg] = this;
}

//------------------------------------------------------------------------
void SegmentationInspector::closeEvent(QCloseEvent *e)
{
  QWidget::closeEvent(e);
  deleteLater();
}

//------------------------------------------------------------------------
SegmentationInspector::~SegmentationInspector()
{
  pqObjectBuilder *ob = pqApplicationCore::instance()->getObjectBuilder();
  ob->destroy(m_view);

  m_inspectors.remove(m_seg);
}

//------------------------------------------------------------------------
void SegmentationInspector::takeSnapshot()
{
  QString fileName = QFileDialog::getSaveFileName(this,
     tr("Save Scene"), "", tr("Image Files (*.jpg *.png)"));
  m_view->saveImage(1024,768,fileName);
}

//------------------------------------------------------------------------
void SegmentationInspector::exportScene()
{
  pqViewExporterManager *exporter = new pqViewExporterManager();
  exporter->setView(m_view);
  QString fileName = QFileDialog::getSaveFileName(this,
     tr("Save Scene"), "", tr("3D Scene (*.x3d *.pov *.vrml)"));
  exporter->write(fileName);
  delete exporter;
}

//------------------------------------------------------------------------
void SegmentationInspector::updateScene()
{
  pqDisplayPolicy *dp = pqApplicationCore::instance()->getDisplayPolicy();
  pqRepresentation *rep;
//   foreach(rep,m_view->getRepresentations())
//   {
//     rep->setVisible(false);
//   }
//   if (m_renderVolumetric->isChecked())
//     m_seg->representation("Volumetric")->render(m_view);
//   if (m_renderMesh->isChecked())
//     m_seg->representation("Mesh")->render(m_view);
//   if (m_renderPlane->isChecked())
//     m_seg->representation("AppositionPlane")->render(m_view);
//   m_view->render();
}
