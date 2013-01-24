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


#ifndef EDGEDISTANCE_H
#define EDGEDISTANCE_H

#include "Core/Extensions/SegmentationExtension.h"
#include "Core/EspinaTypes.h"

namespace EspINA
{
  class EdgeDistance
  : public Segmentation::Information
  {
    struct CacheEntry
    {
      CacheEntry();

      Nm Distances[6];
    };

    static const QString EXTENSION_FILE;

    static QMap<SegmentationPtr, CacheEntry> s_cache;

  public:
    static const ModelItem::ExtId ID;

    static const Segmentation::InfoTag LEFT_DISTANCE;
    static const Segmentation::InfoTag TOP_DISTANCE;
    static const Segmentation::InfoTag UPPER_DISTANCE;
    static const Segmentation::InfoTag RIGHT_DISTANCE;
    static const Segmentation::InfoTag BOTTOM_DISTANCE;
    static const Segmentation::InfoTag LOWER_DISTANCE;

    explicit EdgeDistance();
    virtual ~EdgeDistance();

    virtual ModelItem::ExtId id();

    virtual ModelItem::ExtIdList dependencies() const
    { return Segmentation::Information::dependencies(); }

    virtual Segmentation::InfoTagList availableInformations() const;

    virtual QVariant information(const Segmentation::InfoTag &tag);

    virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());

    virtual bool isCacheFile(const QString &file) const
    { return EXTENSION_FILE == file; }

    virtual bool loadCache(QuaZipFile &file, const QDir &tmpDir, EspinaModel *model);

    virtual bool saveCache(CacheList &cacheList);

    virtual Segmentation::InformationExtension clone();

    void edgeDistance(Nm distances[6]) const
    {
      updateDistances();
      memcpy(distances, m_distances, 6*sizeof(Nm));
    }

  private:
    void updateDistances() const;
    void setDistances(Nm distances[6]);

  private:
    mutable Nm m_distances[6];

    friend class AdaptiveEdges;
  };

  typedef EdgeDistance  *EdgeDistancePtr;
  typedef QSharedPointer<EdgeDistancePtr> EdgeDistanceSPtr;

  EdgeDistancePtr edgeDistancePtr(ModelItem::Extension *extension);

}// namespace EspINA

#endif // EDGEDISTANCE_H
