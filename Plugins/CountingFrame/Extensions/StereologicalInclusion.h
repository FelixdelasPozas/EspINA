/*
 *    
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_STEREOLOGICAL_INCLUSION_H
#define ESPINA_STEREOLOGICAL_INCLUSION_H

// Plugin
#include "CountingFramePlugin_Export.h"
#include "CountingFrames/CountingFrame.h"

// ESPINA
#include <Core/Analysis/Extension.h>
#include <Core/Utils/Bounds.h>

class vtkPoints;
class vtkPolyData;

namespace ESPINA
{
  namespace CF {
  class CountingFramePlugin_EXPORT StereologicalInclusion
  : public SegmentationExtension
  {
    Q_OBJECT

    static const QString FILE;

  public:
    static const Type           TYPE;
    static const InformationKey TOUCH_EDGES;

    InformationKey cfKey(CountingFrame *cf) const;

  public:
    explicit StereologicalInclusion(const InfoCache &infoCache = InfoCache());

    virtual ~StereologicalInclusion();

    virtual Type type() const
    { return TYPE; }

    virtual bool invalidateOnChange() const
    { return true; }

    virtual State state() const;

    virtual Snapshot snapshot() const;

    virtual TypeList dependencies() const;

    virtual bool validCategory(const QString& classificationName) const
    { return true; }

    virtual InformationKeyList availableInformation() const;

//     virtual QVariant information(const Key& tag) const;

    virtual QString toolTipText() const;

    void addCountingFrame(CountingFrame *cf);
    void removeCountingFrame(CountingFrame *cf);
    bool hasCountingFrames() const;
    // The Segmentation is excluded at least by a CF
    bool isExcluded();

    bool isOnEdge() const;

  protected:
    virtual QVariant cacheFail(const InformationKey& tag) const;

    virtual void onExtendedItemSet(Segmentation *segmentation);

  public slots:
    void evaluateCountingFrame(CountingFrame *cf);
    void evaluateCountingFrames();

  private:
    bool isExcludedByCountingFrame(CountingFrame *cf);
    bool isRealCollision(const Bounds& collisionBounds);
    void checkSampleCountingFrames();

  private:
    bool m_isInitialized;
    bool m_isUpdated;

    QMutex m_mutex;
    bool   m_isExcluded;
    QMap<CountingFrame *, bool>   m_exclusionCFs;
    QMap<CountingFrame::Id, bool> m_excludedByCF;
  };

  using StereologicalInclusionPtr  = StereologicalInclusion *;
  using StereologicalInclusionSPtr = std::shared_ptr<StereologicalInclusion>;

  StereologicalInclusionSPtr stereologicalInclusion(SegmentationExtensionSPtr extension);

  } // namespace CF
} // namespace ESPINA

#endif // STEREOLOGICALINCLUSION_H
