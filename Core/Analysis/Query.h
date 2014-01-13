/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  <copyright holder> <email>
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

#ifndef ESPINA_RELATIONSHIPS_H
#define ESPINA_RELATIONSHIPS_H

#include <Core/EspinaTypes.h>

namespace EspINA {

  namespace Query
  {

  SampleSPtr sample(SegmentationPtr segmentation);

  SampleSPtr sample(SegmentationSPtr segmentation);

  ChannelSPtr channel(SegmentationSPtr segmentation);

  ChannelSList channels(SampleSPtr sample);

  SegmentationSList segmentations(SampleSPtr sample);

  } // namespace Query
} // namespace EspINA

#endif // ESPINA_RELATIONSHIPS_H
