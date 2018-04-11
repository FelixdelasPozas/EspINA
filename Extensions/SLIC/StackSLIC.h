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
        virtual ~StackSLIC()
        {};

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

      protected:
        virtual void onExtendedItemSet(Channel* item) {}

        virtual QVariant cacheFail(const InformationKey& tag) const
        { return QVariant(); }

      protected slots:
        void onComputeSLIC();
        void onSLICComputed();

      signals:
        void computeFinished();

      private:
        class SLICComputeTask;
        SchedulerSPtr m_scheduler; /** application scheduler. */
        CoreFactory  *m_factory;   /** core factory.          */
    };

    class StackSLIC::SLICComputeTask
    : public Task
    {
        Q_OBJECT
      public:
        explicit SLICComputeTask(ChannelPtr stack, SchedulerSPtr scheduler, CoreFactory *factory);
        virtual ~SLICComputeTask()
        {};

      private:
        virtual void run();

        ChannelPtr m_stack;
        CoreFactory *m_factory;

        unsigned int parameter_m_s;
        unsigned int parameter_m_c;
        SLICVariant variant;

        typedef struct Label
        {
          itk::Image<unsigned char, 3>::IndexType center;
          int m_c;
          int m_s;
          double norm_quotient;
        } Label;

        typedef struct Voxel
        {
          unsigned int label;
          double distance;
        } Voxel;
    };
  }
}

#endif /* EXTENSIONS_SLIC_STACKSLIC_H_ */
