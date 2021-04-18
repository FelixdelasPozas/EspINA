/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2015  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "BasicRepresentationSwitch.h"

#include <Support/Widgets/ProgressTool.h>
#include <GUI/Widgets/Styles.h>
#include <QPushButton>

using namespace ESPINA;
using namespace ESPINA::GUI::Representations;
using namespace ESPINA::GUI::Widgets;

//----------------------------------------------------------------------------
BasicRepresentationSwitch::BasicRepresentationSwitch(const QString            &id,
                                                     RepresentationManagerSPtr manager,
                                                     ViewTypeFlags             supportedViews,
                                                     Support::Context         &context)
: RepresentationSwitch{id, manager->icon(), manager->description(), context}
, m_manager           {manager}
, m_flags             {supportedViews}
{
  setChecked(m_manager->representationsVisibility());
}

//----------------------------------------------------------------------------
ESPINA::ViewTypeFlags BasicRepresentationSwitch::supportedViews() const
{
  return m_flags;
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::showRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  m_manager->show(frame);
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::hideRepresentations(const GUI::Representations::FrameCSPtr frame)
{
  m_manager->hide(frame);
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::invalidateRepresentationsImplementation(ViewItemAdapterList items,
                                                                        const GUI::Representations::FrameCSPtr frame)
{
  for(auto pool: m_manager->pools())
  {
    pool->invalidateRepresentations(items, frame);
  }
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::restoreSettings(std::shared_ptr<QSettings> settings)
{
  restoreCheckedState(settings);
}

//----------------------------------------------------------------------------
void BasicRepresentationSwitch::saveSettings(std::shared_ptr<QSettings> settings)
{
  saveCheckedState(settings);
}
