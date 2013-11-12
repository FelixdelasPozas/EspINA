/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "SeedGrowSegmentationTool.h"
#include <GUI/Selectors/PixelSelector.h>
#include <QAction>

using namespace EspINA;

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::SeedGrowSegmentationTool()
: m_selector(new ActionSelector())
, m_seedThreshold(new SeedThreshold())
, m_applyROI(new ApplyROI())
{
  addVoxelSelector(
    new QAction(QIcon(":/espina/pixelSelector.svg"),
                tr("Create segmentation based on selected pixel (Ctrl +)"),
                m_selector),
    SelectorSPtr{new PixelSelector()}
  );
  addVoxelSelector(
    new QAction(QIcon(":/espina/bestPixelSelector.svg"),
                tr("Create segmentation based on best pixel (Ctrl +)"),
                m_selector),
    SelectorSPtr{new PixelSelector()}
  );
}

//-----------------------------------------------------------------------------
SeedGrowSegmentationTool::~SeedGrowSegmentationTool()
{
  delete m_selector;
  delete m_seedThreshold;
  delete m_applyROI;
}

//-----------------------------------------------------------------------------
QList<QAction *> SeedGrowSegmentationTool::actions() const
{
  QList<QAction *> actions;

  actions << m_selector;
  actions << m_seedThreshold;
  actions << m_applyROI;

  return actions;
}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationTool::enabled() const
{

}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setEnabled(bool value)
{

}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::setInUse(bool value)
{

}

//-----------------------------------------------------------------------------
bool SeedGrowSegmentationTool::filterEvent(QEvent* e, EspINA::RenderView* view)
{

}

//-----------------------------------------------------------------------------
QCursor SeedGrowSegmentationTool::cursor() const
{

}

//-----------------------------------------------------------------------------
void SeedGrowSegmentationTool::addVoxelSelector(QAction* action, SelectorSPtr selector)
{
  m_selector->addAction(action);
  m_voxelSelectors[action] = selector;

  selector->setMultiSelection(false);
  selector->setSelectionTag(Selector::CHANNEL);
}
