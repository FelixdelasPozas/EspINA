/*
 * 
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#include "Coordinator.h"
#include "EntrySelectorDialog.h"
#include "LevelSelectorDialog.h"
#include "EntryEditorDialog.h"

using namespace ESPINA;

//------------------------------------------------------------------------
Metadona::Id Coordinator::selectEntry(const Metadona::Level& level, std::vector< Metadona::Id > entries) const
{
  Metadona::Id id;

  EntrySelectorDialog dialog(level.c_str(), entries);

  if (dialog.exec() == QDialog::Accepted)
  {
    id = dialog.selectedId();
  }
  else
  {
    throw Aborted_Exception();
  }

  return id;
}

//------------------------------------------------------------------------
void Coordinator::createEntry(Metadona::Entry& entry)
{
  EntryEditorDialog dialog(entry);

  if (dialog.exec() == QDialog::Accepted)
  {
  }
  else
  {
    throw Aborted_Exception();
  }
}

//------------------------------------------------------------------------
Metadona::Level Coordinator::selectNextLevel(const std::vector<Metadona::Level>& levels)
{
  Metadona::Id level;

  LevelSelectorDialog dialog(levels);

  if (dialog.exec() == QDialog::Accepted)
  {
    level = dialog.selectedLevel();
  }
  else
  {
    throw Aborted_Exception();
  }

  return level;
}
