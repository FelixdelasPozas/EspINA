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

#include "CODERefiner.h"

#include "CODERefineWidget.h"

using namespace ESPINA;

//-----------------------------------------------------------------------------
CODERefiner::CODERefiner(const QString& title)
: m_title{title}
{

}

//-----------------------------------------------------------------------------
QWidget* CODERefiner::createWidget(SegmentationAdapterPtr segmentation, Support::Context& context)
{
  auto filter = std::dynamic_pointer_cast<MorphologicalEditionFilter>(segmentation->filter());
  auto widget = new CODERefineWidget(m_title, segmentation, filter, context);

//   connect(widget, SIGNAL(radiusChanged(int)),
//           this,   SIGNAL(radiusChanged(int)));
//   connect(this,   SIGNAL(radiusChanged(int)),
//           widget, SLOT(setRadius(int)));

  return widget;
}
