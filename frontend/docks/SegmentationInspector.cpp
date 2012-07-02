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

#include <model/ModelItem.h>
#include <model/Segmentation.h>
#include <model/Filter.h>
#include <gui/VolumeView.h>

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
, m_seg(seg)
, m_view(NULL)
{
  setupUi(this);

  m_view = new VolumeView();
  this->hlayout->insertWidget(0, m_view);

  m_view->addSegmentationRepresentation(seg);
  m_view->resetCamera();
  m_view->forceRender();

  connect(seg, SIGNAL(modified(ModelItem*)),
	  this, SLOT(updateScene()));

  ModelItem::Vector filters = seg->relatedItems(ModelItem::IN, "CreateSegmentation");
  Q_ASSERT(filters.size() > 0);
  Filter *filter = dynamic_cast<Filter *>(filters.first());
  Q_ASSERT(filter);
  this->pluginLayout->insertWidget(0, filter->createConfigurationWidget());

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
  delete m_view;

  m_inspectors.remove(m_seg);
}

//
void SegmentationInspector::updateScene()
{
  m_view->updateSegmentationRepresentation(m_seg);
  m_view->forceRender();
}

// //------------------------------------------------------------------------
// void SegmentationInspector::takeSnapshot()
// {
//   QString fileName = QFileDialog::getSaveFileName(this,
//      tr("Save Scene"), "", tr("Image Files (*.jpg *.png)"));
//   m_view->saveImage(1024,768,fileName);
// }
// 
// //------------------------------------------------------------------------
// void SegmentationInspector::exportScene()
// {
//   pqViewExporterManager *exporter = new pqViewExporterManager();
//   exporter->setView(m_view);
//   QString fileName = QFileDialog::getSaveFileName(this,
//      tr("Save Scene"), "", tr("3D Scene (*.x3d *.pov *.vrml)"));
//   exporter->write(fileName);
//   delete exporter;
// }
