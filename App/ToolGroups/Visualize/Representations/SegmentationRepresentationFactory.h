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

#ifndef ESPINA_SEGMENTATION_REPRESENTATION_FACTORY_H
#define ESPINA_SEGMENTATION_REPRESENTATION_FACTORY_H

#include <Support/Representations/BasicRepresentationSwitch.h>
#include <Support/Representations/RepresentationFactory.h>
#include <GUI/ColorEngines/ColorEngine.h>
#include <GUI/Representations/RepresentationPool.h>
#include <GUI/Representations/Settings/SegmentationSlicePoolSettings.h>
#include <GUI/Widgets/NumericalInput.h>

namespace ESPINA
{
  class SegmentationRepresentationFactory
  : public RepresentationFactory
  {
  public:
    explicit SegmentationRepresentationFactory();

  private:
    virtual Representation doCreateRepresentation ( Support::Context& context, ViewTypeFlags supportedViews ) const;

    void createSliceRepresentation     (Representation &representation, Support::Context &context, ViewTypeFlags supportedViews) const;
    void createContourRepresentation   (Representation &representation, Support::Context &context) const;
    void createSkeletonRepresentation  (Representation &representation, Support::Context &context) const;
    void createMeshRepresentation      (Representation &representation, Support::Context &context) const;

    void groupSwitch(const QString &order, Support::Widgets::ToolSPtr tool) const;

  private:
    static const unsigned int WINDOW_SIZE;
  };
}

#endif // ESPINA_SEGMENTATION_REPRESENTATION_FACTORY_H
