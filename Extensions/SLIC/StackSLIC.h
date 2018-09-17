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

        typedef enum SLICVariant {
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

        virtual QString toolTipText() const override
        { return tr("SLIC Algorithm"); }

        virtual TypeList dependencies() const
        {
          Extension::TypeList deps;
          deps << Extensions::ChannelEdges::TYPE;
          return deps;
        }

        virtual InformationKeyList availableInformation() const;

        unsigned long int getSupervoxel(unsigned int x, unsigned int y, unsigned int z);
        unsigned char getSupervoxelColor(unsigned int supervoxel);
        itk::Image<unsigned char, 3>::IndexType getSupervoxelCenter(unsigned int supervoxel);
        bool drawSliceInImageData(unsigned int slice, vtkSmartPointer<vtkImageData> data);
        bool drawVoxelCenters(unsigned int slice, vtkSmartPointer<vtkPoints> data);
        bool isComputed();
        bool isRunning();
        StackSLIC::SLICVariant getVariant();
        unsigned char getSupervoxelSize();
        unsigned char getColorWeight();
        unsigned int getIterations();
        double getTolerance();
        unsigned int getSupervoxelCount();
        double getSliceSpacing();

        typedef struct Label
        {
          double norm_quotient;
          float m_s;
          itk::Image<unsigned char, 3>::IndexType center;
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
          itk::Image<unsigned char, 3>::IndexType center;
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
            QList<SuperVoxel> supervoxels;
            unsigned int supervoxel_count = 0;
            QList<QByteArray> voxels;
            unsigned int slice_count = 0;
            double tolerance;
            unsigned int iterations;
            unsigned char m_s;
            unsigned char m_c;
            SLICVariant variant;
            bool computed = false;
            Bounds bounds;
            mutable QReadWriteLock m_dataMutex;
        } SLICResult;
        SchedulerSPtr m_scheduler; /** application scheduler. */
        CoreFactory  *m_factory;   /** core factory.          */
        std::shared_ptr<SLICComputeTask> task;
        SLICResult result;
        QString snapshotName(const QString& file) const;
        bool loadFromSnapshot();

        friend SLICComputeTask;
    };

    class StackSLIC::SLICComputeTask
    : public Task
    {
        Q_OBJECT
      public:
        explicit SLICComputeTask(ChannelPtr stack, SchedulerSPtr scheduler, CoreFactory *factory, SLICResult *result, unsigned int parameter_m_s, unsigned int parameter_m_c, SLICVariant variant, unsigned int max_iterations, double tolerance);
        virtual ~SLICComputeTask() {};

      private:
        virtual void run();
        virtual void onAbort();
        void fitRegionToBounds(int region_position[], int region_size[]);
        void saveResults(QList<Label> labels, unsigned int *voxels);
        void findCandidateRegion(itk::Image<unsigned char, 3>::IndexType &center, double scan_size, int region_position[], int region_size[]);
        bool initSupervoxels(itk::Image<unsigned char, 3> *image, QList<Label> &labels, ChannelEdges *edgesExtension);
        bool isInBounds(int x, int y, int z);
        float calculateDistance(itk::Image<unsigned char, 3>::IndexType &voxel_index, itk::Image<unsigned char, 3>::IndexType &center_index,
                                                                   unsigned char voxel_color, unsigned char center_color, float norm_quotient, bool only_spatial = false);

        ChannelPtr m_stack;
        CoreFactory *m_factory;

        SLICResult* result;
        unsigned int *voxels;
        NmVector3 spacing;
        const Bounds bounds;

        unsigned char parameter_m_s;
        unsigned char parameter_m_c;
        SLICVariant variant;
        unsigned int max_iterations;
        double tolerance;

        int max_x = 0, max_y = 0, max_z = 0, min_x = 0, min_y = 0, min_z = 0;

        //Used to avoid dividing when switching from grayscale space (0-255)
        //to CIELab intensity (0-100)
        const double color_normalization_constant = 100.0/255.0;
        bool use_sse = false;

    };

  }

}

#endif /* EXTENSIONS_SLIC_STACKSLIC_H_ */
