/*

    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

// ESPINA
#include "ManualROITool.h"
#include "RestrictToolGroup.h"
#include <GUI/View/RenderView.h>

// Qt
#include <QDebug>
#include <QAction>

using namespace ESPINA;

//-----------------------------------------------------------------------------
ManualROITool::ManualROITool(Support::Context  &context,
                             RestrictToolGroup *toolGroup)
: m_undoStack    {context.undoStack()}
, m_toolGroup    {toolGroup}
, m_drawingWidget{context}
{
  configureDrawingTools();

  connect(getSelection(context).get(), SIGNAL(activeChannelChanged(ChannelAdapterPtr)),
          this,                        SLOT(updateReferenceItem(ChannelAdapterPtr)));

  connect(&m_drawingWidget, SIGNAL(maskPainted(BinaryMaskSPtr<unsigned char>)),
          this,             SIGNAL(roiDefined(BinaryMaskSPtr<unsigned char>)));
}

//-----------------------------------------------------------------------------
ManualROITool::~ManualROITool()
{
}

//-----------------------------------------------------------------------------
QList<QAction *> ManualROITool::actions() const
{
  return m_drawingWidget.actions();
}

//-----------------------------------------------------------------------------
void ManualROITool::abortOperation()
{

}

//-----------------------------------------------------------------------------
void ManualROITool::setColor(const QColor& color)
{
  m_drawingWidget.setDrawingColor(color);
}

//-----------------------------------------------------------------------------
void ManualROITool::ROIChanged()
{
}

// //-----------------------------------------------------------------------------
// void ManualROITool::drawingModeChanged(bool isDrawing)
// {
//   QAction *actualAction = m_drawToolSelector->getCurrentAction();
//   QIcon icon;
//
//   if (m_discTool == actualAction)
//   {
//     if (isDrawing)
//       icon = QIcon(":/espina/voi_brush2D.svg");
//     else
//       icon = QIcon(":/espina/voi_brush2D-erase.svg");
//   }
//   else
//   {
//     if (m_sphereTool == actualAction)
//     {
//       if (isDrawing)
//         icon = QIcon(":/espina/voi_brush3D.svg");
//       else
//         icon = QIcon(":/espina/voi_brush3D-erase.svg");
//     }
//   }
//
//   m_drawToolSelector->setIcon(icon);
// }
//
// //------------------------------------------------------------------------
// void ManualROITool::drawStroke(Selector::Selection selection)
// {
//   emit roiDefined(selection);
//
//   updateReferenceItem();
// }
//
//-----------------------------------------------------------------------------
void ManualROITool::cancelROI()
{
//   m_currentSelector->abortOperation();
}

//-----------------------------------------------------------------------------
void ManualROITool::updateReferenceItem(ChannelAdapterPtr channel)
{
  Q_ASSERT(channel);

  auto output  = channel->output();
  auto origin  = readLockVolume(output)->origin();
  auto spacing = output->spacing();

  m_drawingWidget.setMaskProperties(spacing, origin);

  QImage brushImage;
  if (m_toolGroup->currentROI())
  {
    m_drawingWidget.setBrushImage(brushImage);
  }

  m_referenceItem = channel;
}

//-----------------------------------------------------------------------------
void ManualROITool::onToolEnabled(bool enabled)
{

}

//-----------------------------------------------------------------------------
void ManualROITool::configureDrawingTools()
{
  m_drawingWidget.setDrawingColor(Qt::yellow);
  m_drawingWidget.showCategoryControls(false);

//   m_drawingWidget.setPencil2DIcon(QIcon(":/espina/roi_brush2D.svg"));
//   m_drawingWidget.setPencil2DText(tr("Modify ROI drawing 2D discs"));
//
//   m_drawingWidget.setPencil3DIcon(QIcon(":/espina/roi_brush3D.svg"));
//   m_drawingWidget.setPencil3DText(tr("Modify ROI drawing 3D spheres"));

  //    QAction *contourTool = new QAction(QIcon(":espina/lasso.png"),
  //                                       tr("Modify segmentation drawing contour"),
  //                                       m_drawToolSelector);
}
