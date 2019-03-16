/*

 Copyright (C) 2019 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

 This file is part of ESPINA.

 ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 */

#ifndef EXTENSIONS_HISTOGRAM_STACKHISTOGRAM_H_
#define EXTENSIONS_HISTOGRAM_STACKHISTOGRAM_H_

// ESPINA
#include <Extensions/EspinaExtensions_Export.h>
#include <Core/Utils/Histogram.h>
#include <Core/Analysis/Extensions.h>

namespace ESPINA
{
  namespace Extensions
  {
    class StackHistogramFactory;

    /** \class StackHistogram
     * \brief Computes the histogram information of a stack.
     *
     */
    class EspinaExtensions_EXPORT StackHistogram
    : public Core::StackExtension
    {
        Q_OBJECT
      public:
        static const Type TYPE;

        /** \brief StackHistogram class virtual destructor.
         *
         */
        virtual ~StackHistogram()
        {};

        virtual Type type() const
        { return TYPE; }

        virtual bool invalidateOnChange() const
        { return true; }

        virtual State state() const;

        virtual Snapshot snapshot() const;

        virtual const TypeList dependencies() const;

        virtual const InformationKeyList availableInformation() const
        { return InformationKeyList(); }

        virtual void invalidate() override
        { m_histogram = Core::Utils::Histogram(); }

        /** \brief Returns the histogram reference.
         *
         */
        const Core::Utils::Histogram &histogram()
        { checkHistogramValidity(); return m_histogram; }

        /** \brief Returns the count of the given value.
         * \param[in] value Unsigned char value.
         *
         */
        const unsigned long long values(const unsigned char value)
        { checkHistogramValidity(); return m_histogram.values(value); };

        /** \brief Returns the histogram median value.
         *
         */
        const unsigned char medianValue()
        { checkHistogramValidity(); return m_histogram.medianValue(); }

        /** \brief Returns the histogram minor value.
         *
         */
        const unsigned char minorValue()
        { checkHistogramValidity(); return m_histogram.minorValue(); }

        /** \brief Returns the histogram major value.
         *
         */
        const unsigned char majorValue()
        { checkHistogramValidity(); return m_histogram.majorValue(); }

        /** \brief Returns the histogram mode value.
         *
         */
        const unsigned char modeValue()
        { checkHistogramValidity(); return m_histogram.modeValue(); }

        /** \brief Returns the number of values in the histogram.
         *
         */
        const unsigned long long count()
        { checkHistogramValidity(); return m_histogram.count(); }

        virtual const QString toolTipText() const;

      signals:
        void progress(int);

      protected:
        virtual void onExtendedItemSet(ChannelPtr item)
        { loadSnapshot(); }

        virtual QVariant cacheFail(const InformationKey &key) const
        { return QVariant(); }

      private:
        /** \brief StackHistogram class constructor.
         * \param[in] factory Core factory pointer.
         *
         */
        explicit StackHistogram(CoreFactory *factory);

        /** \brief Checks if the histogram is empty and computes it if it is.
         *
         */
        void checkHistogramValidity();

        /** \brief Loads the histogram data from disk if it exists.
         *
         */
        void loadSnapshot();

        Core::Utils::Histogram m_histogram; /** stack histogram.                                                */
        CoreFactory           *m_factory;   /** core factory needed to create ChannelEdges extension if needed. */
        QMutex                 m_lock;      /** protects the extension computation.                             */

        friend class StackHistogramFactory;
    };

    using StackHistogramSPtr = std::shared_ptr<StackHistogram>;
  
  } // namespace Extensions
} // namespace ESPINA

#endif // EXTENSIONS_HISTOGRAM_STACKHISTOGRAM_H_
