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

#ifndef ESPINA_READ_ONLY_SEGMENTATION_EXTENSION_H
#define ESPINA_READ_ONLY_SEGMENTATION_EXTENSION_H

#include "Core/Analysis/Extensions/SegmentationExtension.h"

namespace EspINA {

  class ReadOnlySegmentationExtension
  : public SegmentationExtension
  {
  public:
    explicit ReadOnlySegmentationExtension(SegmentationExtension::Type type)
    : m_type(type)
    {}

    virtual SegmentationExtension::Type type() const
    { return m_type; }

    virtual State state() const
    { return State(); }//TODO

    virtual Snapshot snapshot() const
    { return Snapshot(); } // TODO

    virtual bool invalidateOnChange() const
    { return false; } // TODO

    virtual InfoTagList availableInformations() const
    { return InfoTagList(); }

    virtual TypeList dependencies() const
    { return TypeList(); }

    virtual QVariant information(const InfoTag& tag) const
    { return QVariant(); }

    virtual bool validCategory(const QString& classificationName) const
    { return true; }

  protected:
    virtual void onSegmentationSet(SegmentationPtr channel)
    {}


  private:
    SegmentationExtension::Type m_type;
  };

} // namespace EspINA

#endif // ESPINA_READ_ONLY_SEGMENTATION_EXTENSION_H