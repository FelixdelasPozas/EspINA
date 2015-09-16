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
#include "FreehandROITool.h"
#include "RestrictToolGroup.h"
#include <GUI/View/RenderView.h>

// Qt
#include <QDebug>
#include <QAction>

using namespace ESPINA;

//-----------------------------------------------------------------------------
FreehandROITool::FreehandROITool(Support::Context  &context,
                             RestrictToolGroup *toolGroup)
: ProgressTool("FreehandROI", ":espina/roi_freehand_roi.svg", tr("Freehand ROI"), context)
, m_undoStack    {context.undoStack()}
, m_toolGroup    {toolGroup}
, m_drawingWidget{context.viewState(), context.model()}
{
  setCheckable(true);
  setExclusive(true);

  addSettingsWidget(&m_drawingWidget);

  configureDrawingTools();

  connect(getSelection().get(), SIGNAL(activeChannelChanged(ChannelAdapterPtr)),
          this,                 SLOT(updateReferenceItem(ChannelAdapterPtr)));

  connect(&m_drawingWidget, SIGNAL(painterChanged(MaskPainterSPtr)),
          this,             SLOT(onPainterChanged(MaskPainterSPtr)));

  connect(&m_drawingWidget, SIGNAL(maskPainted(BinaryMaskSPtr<unsigned char>)),
          this,             SIGNAL(roiDefined(BinaryMaskSPtr<unsigned char>)));

  setEventHandler(m_drawingWidget.painter());
}

//-----------------------------------------------------------------------------
FreehandROITool::~FreehandROITool()
{
}

//-----------------------------------------------------------------------------
void FreehandROITool::abortOperation()
{
}

//-----------------------------------------------------------------------------
void FreehandROITool::setColor(const QColor& color)
{
  m_drawingWidget.setDrawingColor(color);
}

//-----------------------------------------------------------------------------
void FreehandROITool::ROIChanged()
{
}

//-----------------------------------------------------------------------------
void FreehandROITool::cancelROI()
{
//   m_currentSelector->abortOperation();
}

//-----------------------------------------------------------------------------
void FreehandROITool::onPainterChanged(MaskPainterSPtr painter)
{
  setEventHandler(painter);
}

//-----------------------------------------------------------------------------
void FreehandROITool::updateReferenceItem(ChannelAdapterPtr channel)
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
void FreehandROITool::restoreSettings(std::shared_ptr<QSettings> settings)
{
  m_drawingWidget.restoreSettings(settings);
}

//-----------------------------------------------------------------------------
void FreehandROITool::saveSettings(std::shared_ptr<QSettings> settings)
{
  m_drawingWidget.saveSettings(settings);
}

//-----------------------------------------------------------------------------
void FreehandROITool::configureDrawingTools()
{
  m_drawingWidget.setDrawingColor(Qt::yellow);
  m_drawingWidget.showCategoryControls(false);
}
