/*
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#include "DefaultHistory.h"

#include "ui_DefaultHistory.h"

using namespace ESPINA;

DefaultHistory::DefaultHistory(SegmentationAdapterPtr segmentation,
                               QWidget               *parent,
                               Qt::WindowFlags        f)
: QWidget(parent, f)
, m_gui(new Ui::DefaultHistory())
{
  m_gui->setupUi(this);

  m_gui->m_segmantionName->setText(segmentation->data(Qt::DisplayRole).toString());
}
