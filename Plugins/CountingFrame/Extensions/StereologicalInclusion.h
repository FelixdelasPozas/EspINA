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
  const ModelItem::ExtId StereologicalInclusionID = "CountingFrameExtension";

  class StereologicalInclusion
  : public Segmentation::Information
  {
    Q_OBJECT

    static const QString EXTENSION_FILE;

    struct ExtensionData
    {
      explicit ExtensionData() : IsExcluded(false) {}

      bool IsExcluded;
      QMap<CountingFrame::Id, bool> ExclusionCFs;
    };

    typedef Cache<SegmentationPtr, ExtensionData> ExtensionCache;

    static ExtensionCache s_cache;

  public:
    static const Segmentation::InfoTag EXCLUDED;

  public:
    explicit StereologicalInclusion();
    virtual ~StereologicalInclusion();

    virtual ModelItem::ExtId id();

    virtual ModelItem::ExtIdList dependencies() const;

    virtual Segmentation::InfoTagList availableInformations() const;

    virtual bool validTaxonomy(const QString &qualifiedName) const
    { return true; }

    virtual void setSegmentation(SegmentationPtr seg);

    virtual QVariant information(const Segmentation::InfoTag &tag);

    virtual QString toolTipText() const;

    virtual bool isCacheFile(const QString &file) const
    { return EXTENSION_FILE == file; }

    virtual void loadCache(QuaZipFile &file, const QDir &tmpDir, IEspinaModel *model);

    virtual bool saveCache(Snapshot &cacheList);

    virtual Segmentation::InformationExtension clone();

    virtual void initialize();

    virtual void invalidate(SegmentationPtr segmentation = NULL);

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
    QMap<CountingFrame *, bool> m_exclusionCFs;
  };

  typedef StereologicalInclusion * StereologicalInclusionPtr;

  StereologicalInclusionPtr stereologicalInclusionPtr(Segmentation::InformationExtension extension);

} // namespace EspINA

#endif // STEREOLOGICALINCLUSION_H
