/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>
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


#ifndef STEREOLOGICALINCLUSION_H
#define STEREOLOGICALINCLUSION_H

#include <CountingFrames/CountingFrame.h>

#include <Core/Extensions/SegmentationExtension.h>
#include <Core/EspinaTypes.h>
#include <Core/EspinaRegion.h>

// Forward declaration
class vtkPoints;
class vtkPolyData;

namespace EspINA
{

  class StereologicalInclusion
  : public Segmentation::Information
  {
    Q_OBJECT

    static const QString EXTENSION_FILE;

    typedef QSet<int> CacheEntry;
    static QMap<SegmentationPtr, CacheEntry> s_cache;

  public:
    static const ModelItem::ExtId ID;

    static const Segmentation::InfoTag EXCLUDED;

  public:
    explicit StereologicalInclusion();
    virtual ~StereologicalInclusion();

    virtual ModelItem::ExtId id();

    virtual ModelItem::ExtIdList dependencies() const;

    virtual Segmentation::InfoTagList availableInformations() const;

    virtual QVariant information(const Segmentation::InfoTag &tag);

    virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());

    virtual bool isCacheFile(const QString &file) const
    { return EXTENSION_FILE == file; }

    virtual bool loadCache(QuaZipFile &file, const QDir &tmpDir, EspinaModel *model);

    virtual bool saveCache(Snapshot &cacheList);

    virtual Segmentation::InformationExtension clone();

    void setCountingFrames(CountingFrameList regions);

    bool isExcluded() const;

    void evaluateCountingFrame(CountingFrame *cf);

  public slots:
    void evaluateCountingFrames();

  protected:
    bool isExcludedFromCountingFrame(CountingFrame *cf);
    bool isRealCollision(EspinaRegion interscetion);
    bool isOnEdge();

  private:
    bool m_isOnEdge;
    QMap<CountingFrame *, bool> m_isExcludedFrom;
  };

  typedef StereologicalInclusion * StereologicalInclusionPtr;

  StereologicalInclusionPtr stereologicalInclusionPtr(Segmentation::InformationExtension extension);

} // namespace EspINA

#endif // STEREOLOGICALINCLUSION_H
