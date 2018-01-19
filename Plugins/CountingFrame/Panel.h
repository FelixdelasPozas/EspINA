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

// Plugin
#include "CountingFramePlugin_Export.h"
#include "CountingFrames/CountingFrame.h"
#include "CountingFrameManager.h"

// ESPINA
#include <Support/Widgets/Panel.h>
#include <Support/Context.h>
#include <Tasks/ComputeOptimalMargins.hxx>

// Qt
#include <QStandardItemModel>

namespace ESPINA
{
  namespace CF
  {
    class CountingFrameExtension;

    /** \class Panel
     * \brief Counting frame plugin GUI panel.
     *
     */
    class CountingFramePlugin_EXPORT Panel
    : public ESPINA::Panel
    {
        Q_OBJECT
        class GUI;     /** gui implementation.     */
        class CFModel; /** qt model for the table. */

      public:
        static const QString ID;

      public:
        /** \brief Panel class constructor
         * \param[in] manager CF manager.
         * \param[in] context application context.
         * \param[in] parent QWidget parent of this one.
         *
         */
        explicit Panel(CountingFrameManager *manager,
                       Support::Context     &context,
                       QWidget              *parent = nullptr);

        /** \brief Panel class virtual destructor.
         *
         */
        virtual ~Panel();

        /** \brief Deletes the given counting frame.
         * \param[in] cf counting frame to delete.
         *
         */
        void deleteCountingFrame(CountingFrame *cf);

        /** \brief Deletes all counting frames.
         *
         */
        void deleteCountingFrames();

      public slots:
        virtual void reset();

      private slots:
        /** \brief Saves the properties and description of all the CFs on the panel
         * to text, CSV and XLS.
         *
         */
        void exportCountingFramesData();

        /** \brief Applies the category contraint to the current CF.
         *
         */
        void applyCategoryConstraint();

        /** \brief Enables the constraint GUI
         * \param[in] enable true to enable the GUI and false otherwise.
         *
         */
        void enableCategoryConstraints(bool enable);

        /** \brief Update UI depending on selected row's counting frame
         * \param[in] index selected CF index.
         *
         */
        void updateUI(QModelIndex index);

        /** \brief Updates the information GUI with the information for the given CF.
         * \param [in] cf counting frame.
         *
         */
        void showInfo(CountingFrame *cf);

        /** \brief Updates the margins GUI values after the computation thread has finished.

         *
         */
        void onMarginsComputed();

        /** \brief Inserts the new CF into the CF list and applies it.
         * \param[in] cf counting frame.
         *
         */
        void onCountingFrameCreated(CountingFrame *cf);

        /** \brief Updates the segmentation's representations after the CF has been applied.
         * \param[in] cf counting frame.
         *
         */
        void onCountingFrameApplied(CountingFrame *cf);

        /** \brief Computes the CF of the added segmentations.
         * \param[in] items list of segmentations.
         *
         */
        void onSegmentationsAdded(ViewItemAdapterSList items);

        /** \brief Executes the CF creation dialog and inserts the CF to the stack extensions.
         *
         */
        void createCountingFrame();

        /** \brief Lauches the optimal margins computatio task.
         *
         */
        void resetActiveCountingFrame();

        /** \brief Updates the margins values on the GUI.
         *
         */
        void updateActiveCountingFrameMargins();

        /** \brief Deletes the currently selected CF.
         *
         */
        void deleteActiveCountingFrame();

        /** \brief Updates the limits of the CF when the stack changes dimensions.
         *
         */
        void onChannelChanged(ChannelAdapterPtr channel);

        /** \brief Changes the unis of measurements.
         *
         */
        void changeUnitMode(bool useSlices);

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

        /** \brief
         *
         */
        QModelIndex findCategoryIndex(const QString &classificationName);

        /** \brief
         *
         */
        void updateSegmentationRepresentations();

        /** \brief
         *
         */
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

        /** \brief
         *
         */
        void updateTable();

        /** \brief
         *
         */
        void applyCountingFrames(SegmentationAdapterSList segmentations);

      private:
        GUI                  *m_gui;            /** user interface chessire-cat.                */
        CountingFrameManager *m_manager;        /** Counting frame manager.                     */
        CFModel              *m_cfModel;        /** table's qt model.                           */
        bool                  m_useSlices;      /** true to measure in slices and false for Nm. */
        CountingFrameList     m_countingFrames; /** list of counting frames.                    */
        CountingFrame        *m_activeCF;       /** pointer to the active counting frame.       */

        using ComputeOptimalMarginsTask = ComputeOptimalMargins<ChannelPtr, SegmentationSList>;
        using ComputeOptimalMarginsSPtr = std::shared_ptr<ComputeOptimalMarginsTask>;

        /** \struct PendingCF
         * \brief Data of currently computing CFs.
         *
         */
        struct PendingCF
        {
          CountingFrame            *CF;   /** pointer to counting frame. */
          ComputeOptimalMarginsSPtr Task; /** margins computation task.  */

          /** \brief PendingCF empty constructor.
           *
           */
          PendingCF() : CF{nullptr}
          {}

          /** \brief PendingCF constructor.
           * \param[in] cf counting frame pointer.
           * \param[in] task margins computation task for the given CF.
           *
           */
          PendingCF(CountingFrame *cf, ComputeOptimalMarginsSPtr task)
          : CF(cf)
          , Task(task)
          {}

          /** \brief PendingCF operator ==
           *
           */
          bool operator==(const PendingCF &rhs) const
          {
            return CF == rhs.CF && Task == rhs.Task;
          }
        };

        QList<PendingCF> m_pendingCFs; /** list of pending CF to be added and currently computing margins. */

        friend class CountingFrameExtension;
    };
  } // namespace CF
} // namespace ESPINA

#endif // ESPINA_COUNTING_FRAME_PANEL_H
