/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2011  Jorge Peña Pastor <jpena@cesvima.upm.es>
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

#ifndef ESPINA_COUNTING_FRAME_PANEL_H
#define ESPINA_COUNTING_FRAME_PANEL_H

#include "CountingFramePlugin_Export.h"

#include <Support/DockWidget.h>

#include "CountingFrames/CountingFrame.h"
#include "CountingFrameManager.h"

#include <QStandardItemModel>

namespace EspINA
{
  namespace CF
  {
  // Forward declaration
  class Channel;
  class CountingFrameExtension;

  /// Counting Frame Plugin
  class CountingFramePlugin_EXPORT Panel
  : public DockWidget
  {
    Q_OBJECT

    class GUI;
  public:
    static const QString ID;

  public:
    explicit Panel(CountingFrameManager *manager,
                   ModelAdapterSPtr      model,
                   ViewManagerSPtr       viewManager,
                   QWidget              *parent = nullptr);
    virtual ~Panel();

    virtual void reset(); // slot

    void createAdaptiveCF(ChannelAdapterPtr channel,
                          Nm inclusion[3],
                          Nm exclusion[3]);

    void createRectangularCF(ChannelAdapterPtr channel,
                             Nm inclusion[3],
                             Nm exclusion[3]);

    void deleteCountingFrame(CountingFrame *cf);

    CountingFrameList countingFrames() const;

  protected slots:
    void applyCategoryConstraint();

    void clearCountingFrames();

    void enableTaxonomicalConstraints(bool enable);

    /// Update UI depending on selected row's counting frame
    void updateUI(int row);

    void showInfo(CountingFrame *cf);

    void updateSegmentations();

    void createCountingFrame();

    void resetActiveCountingFrame();

    void updateActiveCountingFrameMargins();

    void deleteActiveCountingFrame();

    void saveActiveCountingFrameDescription();

    void channelChanged(ChannelAdapterPtr channel);

    void changeUnitMode(bool useSlices);

  private:
    /// Find margin values which discard all segmentations that
    /// touch the channel margins
    void computeOptimalMargins(ChannelAdapterPtr channel,
                               Nm inclusion[3],
                               Nm exclusion[3]);

    /// Return inclusion margins definded by the UI
    void inclusionMargins(double values[3]);

    /// Return exclusion margins definded by the UI
    void exclusionMargins(double values[3]);

    void registerCF(CountingFrameExtension *ext,
                    CountingFrame *cf);

  signals:
    void countingFrameCreated(CountingFrame *);

    void countingFrameDeleted(CountingFrame *);

  private:
    CountingFrameManager *m_manager;
    ModelAdapterSPtr      m_model;
    ViewManagerSPtr       m_viewManager;

    GUI *m_gui;

    bool m_useSlices;

    QList<CountingFrameExtension *> m_countingFramesExtensions;
    CountingFrameList m_countingFrames;
    CountingFrame    *m_activeCF;
    CountingFrame::Id m_nextId;

    static const int NUM_FIXED_ROWS = 2;

    friend class CountingFrameExtension;
  };

  } // namespace CF
} // namespace EspINA

#endif // ESPINA_COUNTING_FRAME_PANEL_H
