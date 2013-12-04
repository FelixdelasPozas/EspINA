/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef ESPINA_EDGE_DISTANCE_H
#define ESPINA_EDGE_DISTANCE_H

#include "Extensions/EspinaExtensions_Export.h"

#include <Core/Analysis/Extensions/SegmentationExtension.h>
#include <Core/Utils/Spatial.h>

namespace EspINA
{

  class EspinaExtensions_EXPORT EdgeDistance
  : public SegmentationExtension
  {
    static const QString EXTENSION_FILE;
  public:
    static const Type TYPE;

    static const InfoTag LEFT_DISTANCE;
    static const InfoTag TOP_DISTANCE;
    static const InfoTag UPPER_DISTANCE;
    static const InfoTag RIGHT_DISTANCE;
    static const InfoTag BOTTOM_DISTANCE;
    static const InfoTag LOWER_DISTANCE;

    explicit EdgeDistance();
    virtual ~EdgeDistance();

    virtual Type type() const
    { return TYPE; }

    virtual State state() const;

    virtual Snapshot snapshot() const;

    virtual TypeList dependencies() const
    { return TypeList(); }

    virtual bool invalidateOnChange() const
    { return true; }

    virtual InfoTagList availableInformations() const;

    virtual bool validCategory(const QString& classificationName) const
    { return true; }

    virtual QVariant information(const InfoTag& tag) const;

    void edgeDistance(Nm distances[6]) const;

    virtual void invalidate();

  protected:
    virtual void onSegmentationSet(SegmentationPtr segmentation);

  private:
    void updateDistances() const;

    void setDistances(Nm distances[6]);

  private:
    mutable bool m_init;
    mutable Nm   m_distances[6];

    friend class AdaptiveEdges;
  };

  using EdgeDistancePtr  = EdgeDistance *;
  using EdgeDistanceSPtr = std::shared_ptr<EdgeDistance>;

  EdgeDistancePtr EspinaExtensions_EXPORT edgeDistance(SegmentationExtensionPtr extension);

}// namespace EspINA

#endif // ESPINA_EDGE_DISTANCE_H
