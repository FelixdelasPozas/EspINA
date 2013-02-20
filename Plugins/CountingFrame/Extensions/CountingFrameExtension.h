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


#ifndef COUNTINGFRAMEEXTENSION_H
#define COUNTINGFRAMEEXTENSION_H

#include <Core/Extensions/ChannelExtension.h>
#include <Plugins/CountingFrame/CountingFrames/CountingFrame.h>

namespace EspINA
{

class StereologicalInclusion;
  class CountingFramePanel;
  class ViewManager;

  const ModelItem::ExtId CountingFrameExtensionID = "CountingFrameExtension";

  class CountingFrameExtension
  : public Channel::Extension
  {
    Q_OBJECT

    enum CFType {
      ADAPTIVE    = 0,
      RECTANGULAR = 1
    };

    struct CF
    {
      CFType Type;
      Nm     Inclusion[3];
      Nm     Exclusion[3];
    };

    typedef QMap<int, CF> CacheEntry;

    static QMap<ChannelPtr, CacheEntry> s_cache;

    static const QString EXTENSION_FILE;

  public:
    static const ModelItem::ArgumentId COUNTING_FRAMES;

    explicit CountingFrameExtension(CountingFramePanel *plugin, ViewManager *vm);
    virtual ~CountingFrameExtension();

    virtual ModelItem::ExtId id()
    { return CountingFrameExtensionID; }

    virtual ModelItem::ExtIdList dependencies() const;

    virtual void initialize(ModelItem::Arguments args = ModelItem::Arguments());
    virtual QString serialize() const;

    virtual bool isCacheFile(const QString &file) const
    { return EXTENSION_FILE == file; }

    virtual bool loadCache(QuaZipFile &file, const QDir &tmpDir, EspinaModel *model);

    virtual bool saveCache(Snapshot &cacheList);

    virtual Channel::ExtensionPtr clone();

    void addCountingFrame   (CountingFrame* countingFrame);
    void deleteCountingFrame(CountingFrame *countingFrame);

    CountingFrameList countingFrames() const {return m_countingFrames;}

  protected slots:
    void countinfFrameUpdated(CountingFrame* countingFrame);

  private:
    /// Retrieves StereologicalInclusion extension from a segmentation model item
    /// If no extension is found, a new one is add to the item
    StereologicalInclusion *stereologicalInclusion(SegmentationPtr segmentation);

  private:
    CountingFramePanel *m_plugin;
    ViewManager        *m_viewManager;
    CountingFrameList   m_countingFrames;

    mutable ModelItem::Arguments    m_args;
  };

  typedef CountingFrameExtension * CountingFrameExtensionPtr;
  CountingFrameExtensionPtr countingFrameExtensionPtr(Channel::ExtensionPtr extension);

}

#endif // COUNTINGFRAMEEXTENSION_H
