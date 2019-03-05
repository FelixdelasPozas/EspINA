/*
 * Copyright (C) 2018, Álvaro Muñoz Fernández <golot@golot.es>
 *
 * This file is part of ESPINA.
 *
 * ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef EXTENSIONS_SLIC_STACKSLIC_H_
#define EXTENSIONS_SLIC_STACKSLIC_H_

#include <Extensions/EspinaExtensions_Export.h>

// ESPINA
#include <Core/Analysis/Extensions.h>
#include <Core/MultiTasking/Task.h>
#include <Extensions/EdgeDistances/ChannelEdges.h>

//Qt
#include <QFutureWatcher>

class vtkUnsignedCharArray;
class vtkPoints;

namespace ESPINA
{
  namespace Extensions
  {
    class EspinaExtensions_EXPORT StackSLIC
    : public ESPINA::Core::StackExtension
    {
        Q_OBJECT
      public:
        /** \brief StackSLIC class constructor.
         * \param[in] cache Extension cache data.
         *
         */
        explicit StackSLIC(SchedulerSPtr scheduler, CoreFactory* factory, const InfoCache &cache);

        /** \brief StackSLIC class virtual destructor.
         *
         */
        virtual ~StackSLIC();

        /** \brief Enum that defines the SLIC variant used.
         *
         */
        enum class SLICVariant
        {
          SLIC = 0,
          SLICO,
          ASLIC,
          UNDEFINED
        };

        static const Type TYPE;
        virtual QString type() const { return TYPE; }
        virtual State state() const { return State(); }
        virtual Snapshot snapshot() const;
        virtual bool invalidateOnChange() const { return false; }
        virtual void invalidate()  {};
        virtual const QString toolTipText() const override;
        virtual const InformationKeyList availableInformation() const { return InformationKeyList(); }
        virtual const TypeList dependencies() const
        {
          Extension::TypeList deps;
          deps << Extensions::ChannelEdges::TYPE;
          return deps;
        }

        /** \brief Returns the label of the supervoxel that
         * the selected voxel belongs to.
         * \param[in] coordinates of the chosen voxel.
         *
         */
        const unsigned int getSupervoxel(const itkVolumeType::IndexType position) const;

        /** \brief Returns the mean color of the specified supervoxel.
         * \param[in] supervoxel label.
         *
         */
        const unsigned char getSupervoxelColor(const unsigned int supervoxel) const;

        /** \brief Returns the mean position of all voxels contained in
         * the specified supervoxel.
         * \param[in] supervoxel label.
         *
         */
        const itkVolumeType::IndexType getSupervoxelCenter(const unsigned int supervoxel) const;

        /** \brief Draws a grayscale slice based on the calculated mean color
         * values of supervoxels.
         * \param[in] slice to draw.
         * \param[out] ImageData to draw the slice on.
         *
         */
        bool drawSliceInImageData(const unsigned int slice, vtkSmartPointer<vtkImageData> data);

        /** \brief Returns a smart pointer to an ITK Image that spans the chosen Bounds colored with label values.
         * \param[in] Bounds of the region to obtain
         *
         */
        itk::Image<unsigned int, 3>::Pointer getLabeledImageFromBounds(const Bounds bounds) const;

        /** \brief Returns a smart pointer to an ITK image that spans the chose Bounds colored with label colors.
         * \param[in] Bounds of the region to obtain
         *
         */
        itkVolumeType::Pointer getImageFromBounds(const Bounds bounds) const;

        /** \brief Draws the calculated supervoxel centers onto an ITK Image.
         * \param[in] slice to draw.
         * \param[out] ImageData to draw the centers on.
         *
         */
        bool drawVoxelCenters(const unsigned int slice, vtkSmartPointer<vtkPoints> data);

        /** \brief Returns true if SLIC has been computed for the current stack.
         *
         */
        const bool isComputed() const;

        /** \brief Returns true if a SLIC computation is currently running.
         *
         */
        const bool isRunning() const;

        /** \brief Returns the SLICVariant used on the last computation.
         *
         */
        StackSLIC::SLICVariant getVariant();

        /** \brief Returns the target supervoxel size used for the last computation
         * or the default value if SLIC hasn't been computed yet.
         *
         */
        unsigned char getSupervoxelSize();

        /** \brief Returns the color weight used for the last computation
         * or the default value if SLIC hasn't been computed yet.
         *
         */
        unsigned char getColorWeight();

        /** \brief Returns the maximum number of iterations used for the last computation
         * or the default value if SLIC hasn't been computed yet.
         *
         */
        unsigned int getIterations();

        /** \brief Returns the tolerance set for the last computation
         * or the default value if SLIC hasn't been computed yet.
         *
         */
        double getTolerance();

        /** \brief Returns the number of supervoxels calculated in the last computation
         * or 0 if SLIC hasn't been computed yet.
         *
         */
        unsigned int getSupervoxelCount();

        /** \brief Returns the spacing of the underlying stack.
         *
         */
        double getSliceSpacing();

        /** \brief Returns the uncompressed grayscale representation of a slice as an itk::Image
         *
         */
        itkVolumeType::Pointer getUncompressedSlice(const int slice) const;

        /** \brief Returns the uncompressed labeled representation of a slice as an itk::Image
         *
         */
        itk::Image<unsigned int, 3>::Pointer getUncompressedLabeledSlice(const int slice) const;

        /** \brief Returns the current progress of the computation task, if any.
         *
         */
        const int taskProgress() const;

        /** \brief Returns the task error messages.
         *
         */
        const QStringList errors() const;

        struct Label
        {
          double                   norm_quotient;
          float                    m_s;
          unsigned int             index;
          itkVolumeType::IndexType center;
          unsigned char            color;
          unsigned char            m_c;
          bool                     valid;

          /** \brief Label constructor.
           *
           */
          Label(double norm, float s, unsigned int i, itkVolumeType::IndexType cen, unsigned char col, unsigned char c, bool isValid)
          : norm_quotient{norm}, m_s{s}, index{i}, center(cen), color{col}, m_c{c}, valid{isValid}
          {};
        };

      protected:
        virtual void onExtendedItemSet(ChannelPtr stack);

        virtual QVariant cacheFail(const InformationKey& tag) const { return QVariant(); }

      public:
        /** \struct SuperVoxel
         * \brief Supervoxel computed data.
         *
         */
        struct SuperVoxel
        {
          itkVolumeType::IndexType center; /** position in the stack image.                 */
          unsigned char            color;  /** color.                                       */
          bool                     valid;  /** true if has voxels assigned, false if empty. */
        };

      protected slots:
        /** \brief Starts the SLIC computation with the given parameters.
         * \param[in] parameter_m_s Spacing between voxels, in voxel units.
         * \param[in] parameter_m_c Voxel color.
         * \param[in] variant SLIC algorithm variant.
         * \param[in] max_iterations SLIC algoritm max iterations.
         * \param[in] tolerance Tolerance value for supervoxel convergence.
         *
         */
        void onComputeSLIC(unsigned char parameter_m_s, unsigned char parameter_m_c,
                           Extensions::StackSLIC::SLICVariant variant, unsigned int max_iterations,
                           double tolerance);

        /** \brief Called when the computation task has finished or has been aborted.
         *
         */
        void onSLICComputed();

        /** \brief Aborts the SLIC computation.
         *
         */
        void onAbortSLIC();

      signals:
        void computeFinished();
        void computeAborted();
        void progress(int);

      private:
        using IndexType   = itkVolumeType::IndexType;
        using ImageRegion = itk::ImageRegion<3>;
        using ImageType   = itk::Image<unsigned int, 3>;

        static const QString VOXELS_FILE;
        static const QString LABELS_FILE;
        static const QString DATA_FILE;

        class SLICComputeTask;

        struct SLICResult
        {
            QList<SuperVoxel>         supervoxels; /** List of calculated supervoxels.                                                */
            double                    tolerance;   /** Tolerance defined when SLIC was computed.                                      */
            unsigned int              iterations;  /** Maximum iterations defined when SLIC was computed.                             */
            unsigned char             m_s;         /** Average supervoxel dimension defined when SLIC was computed.                   */
            unsigned char             m_c;         /** Color weight defined when SLIC was computed.                                   */
            SLICVariant               variant;     /** SLICVariant used when SLIC was computed.                                       */
            bool                      computed;    /** If this result refers to a completed successful run.                           */
            bool                      modified;    /** If this result contains results different than those loaded from the snapshot. */
            itkVolumeType::RegionType region;      /** region to compute.                                                             */
            bool                      converged;   /** true if converged, false otherwise.                                            */
            mutable QReadWriteLock    dataMutex;   /** data protection mutex.                                                         */

            SLICResult(): tolerance{0}, iterations{10}, m_s{10}, m_c{20}, variant{SLICVariant::SLIC}, computed{false}, modified{false}, converged{false} {};
        };

        SchedulerSPtr                    m_scheduler; /** application scheduler.                                   */
        CoreFactory                     *m_factory;   /** core factory.                                            */
        std::shared_ptr<SLICComputeTask> m_task;      /** Task instance that is currently running SLIC.            */
        SLICResult                       m_result;    /** struct holding the results of the last SLIC computation. */

        /** \brief Loads the results from disk.
         *
         */
        bool loadFromSnapshot();

        /** \brief Returns a QByteArray that contains a compressed slice loaded from disk or cached. The data returned needs to
         *  be decompressed first with qUncompress.
         * \param[in] slice Slice number.
         *
         */
        const QByteArray getSlice(const int slice) const;

        friend SLICComputeTask;
    };

    class StackSLIC::SLICComputeTask
    : public Task
    {
        Q_OBJECT
      public:
        /** \brief SCICComputeTask class constructor.
         * \param[in] stack Stack pointer.
         * \param[in] scheduler Application task scheduler.
         * \param[in] factory Application object factory.
         * \param[in] result Referencer to results struct.
         *
         */
        explicit SLICComputeTask(ChannelPtr    stack,
                                 SchedulerSPtr scheduler,
                                 CoreFactory  *factory,
                                 SLICResult   &result);

        /** \brief SLICComputeTask class virtual destructor.
         *
         */
        virtual ~SLICComputeTask()
        {};

      private:
        virtual void run();
        virtual void onAbort();

        /** \brief Saves the computed results to the result struct to disk.
         *
         */
        void saveResults();

        /** \brief Saves the computed image to the temporal storage.
         *
         */
        void saveRegionImage();

        /** \brief Compresses a slice and saves it to a QDataStream in RLE format prepending slice position and size.
         * \param[out] QDataStream that will hold the compressed slice.
         * \param[in] slice to compress.
         * \param[in] region Image region to compress.
         *
         */
        void compressSliceRLE(QDataStream &stream, const long int z, const ImageRegion &region);

        /** \brief Returns the custom-defined region of voxels that are candidates to belong to the
         * given supervoxel.
         * \param[in] supervoxel position.
         * \param[in] radius of influence assigned to the supervoxel.
         * \param[out] region Computed region.
         *
         */
        void findCandidateRegion(itkVolumeType::IndexType &center, double scan_size, ImageRegion &region) const;

        /** \brief Distributes evenly spaced empty supervoxels trying not to place them on edges.
         * \param[in] image to populate with supervoxels.
         * \param[out] list that will hold the created supervoxels.
         * \param[in] edgesExtension Pointer to the edges extension.
         *
         */
        bool initLabels(itkVolumeType *image, QList<Label> &labels, ChannelEdges *edgesExtension);

        /** \brief Returns the offset in memory or file of the given image index.
         * \param[in] index Voxel index struct.
         *
         */
        const unsigned long long int offsetOfIndex(const IndexType &index);

        //TODO: Check color_distance for nullptr and remove only_spatial?
        /** \brief Calculates the weighted distance between a voxel an a supervoxel.
         * \param[in] coordinates of the voxel.
         * \param[in] coordinates of the supervoxel.
         * \param[in] voxel color.
         * \param[in] mean color of the voxels contained in the supervoxel.
         * \param[in] normalization quotient used to calculate the weighted distance.
         * \param[out] weighted color distance.
         * \param[out] weighted spatial distance.
         * \param[in] wether color distance should be calculated.
         *
         */
        float calculateDistance(const IndexType &voxel_index, const IndexType &center_index,
                                const unsigned char voxel_color, const unsigned char center_color,
                                const float norm_quotient, float *color_distance, float *spatial_distance,
                                bool only_spatial = false);

        /** \brief Finds the surrounding voxels that belong to the given supervoxel.
         * \param[in,out] label Label to update.
         * \param[in] edgesExtension pointer to the edges extension.
         * \param[in] image pointer to the image.
         * \param[in] labels List of all supervoxels.
         *
         */
        void computeLabel(Label &label, ChannelEdgesSPtr edgesExtension, itkVolumeType *image, QList<Label> &labels);

        /** \brief Recomputes the label center looking at the current label voxel positions.
         * \param[in,out] label to update.
         * \param[in] image Pointer to stack image.
         * \param[in] tolerance Tolerance value for convergence test.
         *
         */
        void recalculateCenter(Label &label, itkVolumeType *image, const double tolerance);

        virtual bool hasErrors() const override
        { return !m_errorMessage.isEmpty(); };

        virtual const QStringList errors() const override;

        ChannelPtr                       m_stack;        /** stack to process.                                                  */
        CoreFactory                     *m_factory;      /** core object factory needed to create edges extension if neccesary. */
        SLICResult                      &result;         /** Pointer to the result struct to write the computed results to.     */
        std::unique_ptr<unsigned int[]>  voxels;         /** voxel volume storage.                                              */
        QString                          m_errorMessage; /** error message or empty on success.                                 */

        const double color_normalization_constant = 100.0/255.0; /** Used to avoid dividing when switching from grayscale space (0-255) to CIELab intensity (0-100) */

        mutable QMutex labelListMutex;

        QFutureWatcher<void> watcher;

        friend StackSLIC;
    };
  } // namespace Extensions
} // namespace ESPINA

#endif /* EXTENSIONS_SLIC_STACKSLIC_H_ */
