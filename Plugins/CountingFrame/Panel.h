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

#ifndef ESPINA_COUNTING_FRAME_PANEL_H
#define ESPINA_COUNTING_FRAME_PANEL_H

#include "CountingFramePlugin_Export.h"

#include <Support/Widgets/Panel.h>
#include <Support/Context.h>
#include <Tasks/ComputeOptimalMargins.hxx>

#include "CountingFrames/CountingFrame.h"
#include "CountingFrameManager.h"
#include <QStandardItemModel>

namespace ESPINA
{
  namespace CF
  {
  // Forward declaration
  class Channel;
  class CountingFrameExtension;

  /// Counting Frame Plugin
  class CountingFramePlugin_EXPORT Panel
  : public ESPINA::Panel
  {
    Q_OBJECT

    class GUI;
    class CFModel;

  public:
    static const QString ID;

  public:
    explicit Panel(CountingFrameManager *manager,
                   Support::Context &context);
    virtual ~Panel();

    void deleteCountingFrame(CountingFrame *cf);

    void deleteCountingFrames();

  public slots:
    virtual void reset(); // slot

  private slots:
    /** \brief Saves the properties and description of all the CFs on the panel
     * to text, CSV and XLS.
     *
     */
    void exportCountingFramesData();

    void applyCategoryConstraint();

    void enableCategoryConstraints(bool enable);

    /// Update UI depending on selected row's counting frame
    void updateUI(QModelIndex index);

    void showInfo(CountingFrame *cf);

    void onMarginsComputed();

    void onCountingFrameCreated(CountingFrame *cf);

    void onCountingFrameApplied(CountingFrame *cf);

    void onSegmentationsAdded(ViewItemAdapterSList items);

    void createCountingFrame();

    void resetActiveCountingFrame();

    void updateActiveCountingFrameMargins();

    void deleteActiveCountingFrame();

    void onChannelChanged(ChannelAdapterPtr channel);

    void changeUnitMode(bool useSlices);

    void reportProgess(int progress);

  private:
    /** \brief Exports the properties and descriptions of the CFs in the panel
     * to a text file.
     * \param[in] fileName file name.
     *
     */
    void exportAsText(const QString &fileName) const;

    /** \brief Exports the properties and descriptions of the CFs in the panel
     * to a comma separated values text file.
     * \param[in] fileName file name.
     *
     */
    void exportAsCSV(const QString &fileName) const;

    /** \brief Exports the properties and descriptions of the CFs in the panel
     * to a Microsoft Excel 97 file.
     * \param[in] fileName file name.
     *
     */
    void exportAsXLS(const QString &fileName) const;

    QModelIndex findCategoryIndex(const QString &classificationName);

    void updateSegmentationRepresentations();

    void updateSegmentationExtensions();

    /** \brief Returns inclusion margins defined by the UI
     * \param[out] values inclusion margins values.
     *
     */
    void inclusionMargins(double values[3]);

    /** Returns exclusion margins defined by the UI.
     * \param[out] values exclusion margins values.
     *
     */
    void exclusionMargins(double values[3]);

    void updateTable();

    void applyCountingFrames(SegmentationAdapterSList segmentations);

  private:
    CountingFrameManager   *m_manager;

    GUI     *m_gui;
    CFModel *m_cfModel;

    bool m_useSlices;

    CountingFrameList m_countingFrames;
    CountingFrame    *m_activeCF;

    using ComputeOptimalMarginsTask = ComputeOptimalMargins<ChannelPtr, SegmentationSList>;
    using ComputeOptimalMarginsSPtr = std::shared_ptr<ComputeOptimalMarginsTask>;

    struct PendingCF
    {
      CountingFrame *CF;
      ComputeOptimalMarginsSPtr Task;

      PendingCF() : CF(nullptr) {}

      PendingCF(CountingFrame *cf, ComputeOptimalMarginsSPtr task)
      : CF(cf)
      , Task(task){}

      bool operator==(const PendingCF &rhs) const
      {
        return CF == rhs.CF && Task == rhs.Task;
      }
    };

    QList<PendingCF> m_pendingCFs;

    friend class CountingFrameExtension;
  };

  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_COUNTING_FRAME_PANEL_H
