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

        static const QString VOXELS_FILE;
        static const QString LABELS_FILE;
        static const QString DATA_FILE;

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
        typedef enum SLICVariant
        {
          SLIC,
          SLICO,
          ASLIC
        } SLICVariant;

        static const Type TYPE;
        virtual QString type() const
        {
          return TYPE;
        }
        virtual State state() const;
        virtual Snapshot snapshot() const;
        virtual bool invalidateOnChange() const
        { return false; }
        virtual void invalidate()
        {};
        virtual const QString toolTipText() const override
        { return QObject::tr("SLIC Algorithm"); }
        virtual const TypeList dependencies() const
        {
          Extension::TypeList deps;
          deps << Extensions::ChannelEdges::TYPE;
          return deps;
        }
        virtual const InformationKeyList availableInformation() const;

        /** \brief Returns the label of the supervoxel that
         * the selected voxel belongs to.
         * \param[in] coordinates of the chosen voxel.
         *
         */
        unsigned long int getSupervoxel(itkVolumeType::IndexType position);
        /** \brief Returns the mean color of the specified supervoxel.
         * \param[in] supervoxel label.
         *
         */
        unsigned char getSupervoxelColor(unsigned int supervoxel);
        /** \brief Returns the mean position of all voxels contained in
         * the specified supervoxel.
         * \param[in] supervoxel label.
         *
         */
        itkVolumeType::IndexType getSupervoxelCenter(unsigned int supervoxel);
        /** \brief Draws a grayscale slice based on the calculated mean color
         * values of supervoxels.
         * \param[in] slice to draw.
         * \param[out] ImageData to draw the slice on.
         *
         */
        bool drawSliceInImageData(unsigned int slice, vtkSmartPointer<vtkImageData> data);
        /** \brief Returns a smart pointer to an ITK Image that spans the chosen Bounds
         * and contains the labels assigned to each voxel.
         * \param[in] Bounds of the region to obtain
         *
         */
        itk::SmartPointer<itk::Image<unsigned int, 3>> getLabeledImageFromBounds(Bounds bounds);
        /** \brief Draws the calculated supervoxel centers onto an ITK Image.
         * \param[in] slice to draw.
         * \param[out] ImageData to draw the centers on.
         *
         */
        bool drawVoxelCenters(unsigned int slice, vtkSmartPointer<vtkPoints> data);
        /** \brief Returns true if SLIC has been computed for the current stack.
         *
         */
        bool isComputed();
        /** \brief Returns true if a SLIC computation is currently running.
         *
         */
        bool isRunning();
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
        /** \brief Returns the uncompressed grayscale representation of a slice as
         * a byte array encapsulated in a unique pointer.
         *
         */
        std::unique_ptr<char[]> getUncompressedSlice(int slice);
        /** \brief Returns the uncompressed labeled representation of a slice as
         * a long long int array encapsulated in a unique pointer.
         *
         */
        std::unique_ptr<long long[]> getUncompressedLabeledSlice(int slice);

        typedef struct Label
        {
          double norm_quotient;
          float m_s;
          unsigned int index;
          itkVolumeType::IndexType center;
          unsigned char color;
          unsigned char m_c;
        } Label;

      protected:
        virtual void onExtendedItemSet(Channel* item) {}

        virtual QVariant cacheFail(const InformationKey& tag) const
        { return QVariant(); }

      public:
        typedef struct SuperVoxel
        {
          itkVolumeType::IndexType center;
          unsigned char color;
        } SuperVoxel;

      protected slots:
        void onComputeSLIC(unsigned char parameter_m_s, unsigned char parameter_m_c, Extensions::StackSLIC::SLICVariant variant, unsigned int max_iterations, double tolerance);
        void onSLICComputed();
        void onAbortSLIC();

      signals:
        void computeFinished();
        void computeAborted();
        void progress(int);

      private:
        class SLICComputeTask;

        typedef struct SLICResult
        {
          private:
            const int capacity = 10; /** Uncompressed slices cache capacity */
            /** \brief struct holding the slice data and cache-related information.
             *
             */
            typedef struct ResultCacheValue {
                int z;
                int accessTime;
                QByteArray array;
            } ResultCacheValue;
            struct CacheCompare {
                bool operator() (const ResultCacheValue& lhs, const ResultCacheValue& rhs) const {
                    return lhs.accessTime < rhs.accessTime;
                }
            };
            std::set<ResultCacheValue, CacheCompare> voxel_cache;
          public:
            QList<SuperVoxel> supervoxels; /** List of calculated supervoxels. */
            //TODO: Replace usages of supervoxel_count with supervoxels.size() and test
            unsigned int supervoxel_count = 0; /** Number of calculated supervoxels. */
            //QList<QByteArray> voxels;
            unsigned int slice_count = 0; /** Number of slices in the stack. */
            double tolerance; /** Tolerance defined when SLIC was computed. */
            unsigned int iterations; /** Maximum iterations defined when SLIC was computed. */
            unsigned char m_s; /** Average supervoxel dimension defined when SLIC was computed. */
            unsigned char m_c; /** Color weight defined when SLIC was computed. */
            SLICVariant variant; /** SLICVariant used when SLIC was computed. */
            bool computed = false; /** If this result refers to a completed successful run. */
            bool modified = false; /** If this result contains results different than those loaded from the snapshot. */
            Bounds bounds; /** Bounds defining the region of the stack that has been computed. */
            TemporalStorage temp_storage; /** Temporal storage for new results yet to be saved to a snapshot. */
            /** \brief Returns a QByteArray that contains a compressed slice loaded from disk or cached.
             * \param[in] slice to return.
             *
             */
            QByteArray getSlice(int slice);
            /** \brief Returns a QByteArray that contains a compressed slice loaded from disk.
             * \param[in] slice to return.
             *
             */
            QByteArray getSliceUncached(int slice) const;
            mutable QReadWriteLock m_dataMutex;
            StackSLIC *stack_instance; /** Reference to the Extension instance that the results belong to. */
        } SLICResult;

        SchedulerSPtr m_scheduler; /** application scheduler. */
        CoreFactory  *m_factory;   /** core factory.          */
        std::shared_ptr<SLICComputeTask> m_task; /** Task instance that is currently running SLIC. */
        SLICResult result; /** struct holding the results of the last SLIC computation. */
        QString snapshotName(const QString& file) const;
        bool loadFromSnapshot();

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
         * \param[in] result Pointer to results.
         * \param[in] parameter_m_s Distance between supervoxels.
         * \param[in] parameter_m_c Color weight.
         * \param[in] variant SLIC variant.
         * \param[in] max_iterations Number of maximum iterations of the algorithm.
         * \param[in] tolerance Tolerance value for an early exit.
         *
         */
        explicit SLICComputeTask(ChannelPtr stack,
                                 SchedulerSPtr scheduler,
                                 CoreFactory *factory,
                                 SLICResult *result,
                                 unsigned int parameter_m_s,
                                 unsigned int parameter_m_c,
                                 SLICVariant variant,
                                 unsigned int max_iterations,
                                 double tolerance);

        /** \brief SLICComputeTask class virtual destructor.
         *
         */
        virtual ~SLICComputeTask()
        {};

      private:
        using IndexType      = itkVolumeType::IndexType;
        using RegionIterator = itk::ImageRegionConstIteratorWithIndex<itkVolumeType>;
        using ImageRegion    = itk::ImageRegion<3>;

        virtual void run();
        virtual void onAbort();

        /** \brief Modifies a custom-defined region to fit the image and ROI.
         * \param[in] int array with x,y,z position values.
         * \param[in] int array with x,y,z axis aligned sizes.
         *
         */
        void fitRegionToBounds(int region_position[], int region_size[]);

        /** \brief Modifies a custom-defined region to fit the image and ROI.
         * \param[in] long long int array with x,y,z position values.
         * \param[in] long long int array with x,y,z axis aligned sizes.
         *
         */
        void fitRegionToBounds(long long int region_position[], long long int region_size[]);

        /** \brief Saves the computed results to the result struct.
         * \param[in] List containing supervoxels information.
         * \param[in] array containing the supervoxel label assigned to each voxel.
         *
         */
        void saveResults(QList<Label> labels, unsigned int *voxels);

        /** \brief Compresses a slice and saves it to a QDataStream.
         * \param[out] QDataStream that will hold the compressed slice.
         * \param[in] slice to compress.
         *
         */
        void compressSlice(QDataStream &stream, unsigned int z);

        /** \brief Returns the custom-defined region of voxels that are candidates to belong to the
         * given supervoxel.
         * \param[in] supervoxel position.
         * \param[in] radius of influence assigned to the supervoxel.
         * \param[in] int array with x,y,z position values.
         * \param[in] int array with x,y,z axis aligned sizes.
         *
         */
        void findCandidateRegion(itkVolumeType::IndexType &center, double scan_size, int region_position[], int region_size[]);

        /** \brief Distributes evenly spaced empty supervoxels trying not to place them on edges.
         * \param[in] image to populate with supervoxels.
         * \param[out] list that will hold the created supervoxels.
         * \param[in] pointer to the edges extension.
         *
         */
        bool initSupervoxels(itkVolumeType *image, QList<Label> &labels, ChannelEdges *edgesExtension);

        /** \brief Returns true if the coordinates are inside the computable area.
         *
         */
        bool isInBounds(int x, int y, int z);

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
        float calculateDistance(IndexType &voxel_index, IndexType &center_index,
                                unsigned char voxel_color, unsigned char center_color,
                                float norm_quotient, float *color_distance, float *spatial_distance, bool only_spatial = false);

        /** \brief Finds the surrounding voxels that belong to the given supervoxel.
         * \param[in,out] label to update.
         * \param[in] pointer to the edges extension.
         * \param[in] pointer to the image.
         * \param[in] list of all supervoxels.
         *
         */
        void computeLabel(Label &label, std::shared_ptr<ChannelEdges> edgesExtension, itkVolumeType::Pointer image, QList<Label> *labels);

        /** \brief Creates the supervoxel with the given parameters.
         * \param[in] cur_index Supervoxel index number.
         * \param[in] edgesExtension Channel edges extension of the stack.
         * \param[in] image Stack image.
         * \param[in] labels List of labels.
         *
         */
        void createSupervoxel(IndexType cur_index, ChannelEdges *edgesExtension, itkVolumeType *image, QList<Label> *labels);

        /** \brief Connectivity algorithm that makes sure that all voxels are connected to their assigned supervoxel.
         *
         */
        void ensureConnectivity();

        /** \brief Ensures connectivity between a supervoxel and all its assigned voxels.
         * \param[in] label Label of the supervoxel.
         *
         */
        void labelConnectivity(Label &label);

        ChannelPtr    m_stack;    /** stack to process.                                                  */
        CoreFactory  *m_factory;  /** core object factory needed to create edges extension if neccesary. */
        SLICResult   *result;     /** Pointer to the result struct to write the computed results to.     */
        unsigned int *voxels;     /** Array holding the assigned values of all voxels.                   */
        QList<Label> *label_list; /** Pointer to the list of supervoxels.                                */
        NmVector3     spacing;    /** Current stack spacing.                                             */
        const Bounds  bounds;     /** Region to be computed.                                             */

        unsigned char parameter_m_s;  /** Distance between supervoxels. Initial parameter.      */
        unsigned char parameter_m_c;  /** Color weight parameter. Initial parameter.            */
        SLICVariant   variant;        /** SLICVariant to use. Initial parameter.                */
        unsigned int  max_iterations; /** Maximum number of iterations. Initial parameter.      */
        double        tolerance;      /** Tolerance value for an early exit. Initial parameter. */

        int max_x = 0, max_y = 0, max_z = 0, min_x = 0, min_y = 0, min_z = 0;
        unsigned long int n_voxels;

        /** Used to avoid dividing when switching from grayscale space (0-255)
         *  to CIELab intensity (0-100)
         *
         */
        const double color_normalization_constant = 100.0/255.0;
        double scan_size; /** Maximum distance to use when looking for candidate voxels. */

        mutable QMutex labelListMutex;
        QFutureWatcher<void> watcher;

        friend StackSLIC;
    };
  } // namespace Extensions
} // namespace ESPINA

#endif /* EXTENSIONS_SLIC_STACKSLIC_H_ */
