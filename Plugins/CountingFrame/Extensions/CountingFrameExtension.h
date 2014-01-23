/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  <copyright holder> <email>
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

#ifndef ESPINA_COUNTING_FRAME_EXTENSION_H
#define ESPINA_COUNTING_FRAME_EXTENSION_H

#include "CountingFramePlugin_Export.h"

#include <Core/Analysis/Extensions/ChannelExtension.h>
#include <Plugins/CountingFrame/CountingFrames/CountingFrame.h>
#include <Plugins/CountingFrame/Extensions/StereologicalInclusion.h>

namespace EspINA
{
  namespace CF {

  class StereologicalInclusion;
//   class CountingFramePanel;
//   class ViewManager;

  class CountingFramePlugin_EXPORT CountingFrameExtension
  : public QObject
  , public ChannelExtension
  {
    Q_OBJECT
    static const QString FILE;

  public:
    static Type TYPE;

  public:
    explicit CountingFrameExtension();

    virtual ~CountingFrameExtension();

    virtual Type type() const
    { return TYPE; }

    virtual bool invalidateOnChange() const
    { return true; }

    virtual State state() const;

    virtual Snapshot snapshot() const;

    virtual bool isCacheFile(const QString &file) const
    { return FILE == file; }

    void addCountingFrame   (CountingFrame *countingFrame);

    void removeCountingFrame(CountingFrame *countingFrame);

    CountingFrameList countingFrames() const
    { return m_countingFrames; }

    virtual void onChannelSet(ChannelPtr channel);

  protected slots:
    void onCountingFrameUpdated(CountingFrame *countingFrame);

  private:
    /// Retrieves StereologicalInclusion extension from a segmentation
    /// If no extension is found, a new one is added to the segmentation
    StereologicalInclusionSPtr stereologicalInclusionExtension(SegmentationSPtr segmentation);

  private:
//     CountingFramePanel *m_plugin;
//     ViewManager        *m_viewManager;
    Nm     m_inclusion[3];
    Nm     m_exclusion[3];

    CountingFrameList m_countingFrames;
  };

  using CountingFrameExtensionPtr  = CountingFrameExtension *;
  using CountingFrameExtensionSPtr = std::shared_ptr<CountingFrameExtension>;

  //CountingFrameExtensionSPtr CountingFramePlugin_EXPORT countingFrameExtensionPtr(ChannelExtensionSPtr extension);

  } // namespace CF
} // namespace EspINA

#endif // ESPINA_COUNTING_FRAME_EXTENSION_H
