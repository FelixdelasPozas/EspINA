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


#ifndef COUNTINGFRAMESEGMENTATIONEXTENSION_H
#define COUNTINGFRAMESEGMENTATIONEXTENSION_H

#include <CountingFrames/CountingFrame.h>

#include <Core/Extensions/SegmentationExtension.h>
#include <Core/EspinaTypes.h>
#include <Core/EspinaRegion.h>

// Forward declaration
class vtkPoints;
class vtkPolyData;

namespace EspINA
{

  class CountingFrameSegmentationExtension
  : public SegmentationExtension
  {
    Q_OBJECT
  public:
    static const ExtId ID;
    static const InfoTag EXCLUDED;

  public:
    explicit CountingFrameSegmentationExtension();
    virtual ~CountingFrameSegmentationExtension();

    virtual ExtId id();

    virtual ExtIdList dependencies() const;

    virtual InfoList availableInformations() const
    { return SegmentationExtension::availableInformations(); }

    virtual RepList availableRepresentations() const
    { return SegmentationExtension::availableRepresentations(); }

    virtual QVariant information(InfoTag tag) const;

    virtual SegmentationRepresentationPtr representation(QString representation);

    void setCountingFrames(CountingFrameList regions);

    virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());

    virtual SegmentationExtensionPtr clone();

    bool isExcluded() const;

    void evaluateCountingFrame(CountingFrame *cf);

  public slots:
    void evaluateCountingFrames();

  protected:
    bool isExcludedFromCountingFrame(CountingFrame *cf);
    bool isOnEdge();
    bool realCollision(EspinaRegion interscetion);

  private:
    bool m_isOnEdge;
    QMap<CountingFrame *, bool> m_isExcludedFrom;
  };

} // namespace EspINA

#endif // COUNTINGFRAMESEGMENTATIONEXTENSION_H
