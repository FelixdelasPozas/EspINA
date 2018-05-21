/*

 Copyright (C) 2018 Felix de las Pozas Alvarez <fpozas@cesvima.upm.es>

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

#ifndef EXTENSIONS_DENDRITE_INFORMATION_H_
#define EXTENSIONS_DENDRITE_INFORMATION_H_

#include <Extensions/EspinaExtensions_Export.h>

// ESPINA
#include <Core/Analysis/Connections.h>
#include <Core/Analysis/Extensions.h>
#include <Core/Analysis/Data/SkeletonData.h>
#include <Core/Analysis/Data/SkeletonDataUtils.h>

namespace ESPINA
{
  namespace Extensions
  {
    class SkeletonInformationFactory;

    /** \class DendriteSkeletonInformation
     * \brief Extension that provides information about skeletal dendrites.
     *
     */
    class EspinaExtensions_EXPORT DendriteSkeletonInformation
    : public Core::SegmentationExtension
    {
      public:
        struct SpineInformation
        {
          QString name;               /** spine name.                                               */
          QString parentName;         /** name of the dendrite of the spine.                        */
          bool    complete;           /** true if not truncated.                                    */
          bool    branched;           /** true if branched.                                         */
          double  length;             /** complete length in nanometers.                            */
          int     numSynapses;        /** number of connections.                                    */
          int     numAsymmetric;      /** number of connections to asymmetric synapses.             */
          int     numAsymmetricHead;  /** number of connections to asymmetric synapses on the head. */
          int     numAsymmetricNeck;  /** number of connections to asymmetric synapses on the neck. */
          int     numSymmetric;       /** number of connections to symmetric synapses.              */
          int     numSymmetricHead;   /** number of connections to symmetric synapses on the head.  */
          int     numSymmetricNeck;   /** number of connections to symmetric synapses on the neck.  */
          int     numAxons;           /** number of axons contacted.                                */
          int     numAxonsInhibitory; /** number of inhibitory axons contacted.                     */
          int     numAxonsExcitatory; /** number of excitatory axons contacted.                     */

          SpineInformation(): complete{false}, branched{false}, length{0.0}, numSynapses{0}, numAsymmetric{0}, numAsymmetricHead{0}, numAsymmetricNeck{0},
                              numSymmetric{0}, numSymmetricHead{0}, numSymmetricNeck{0}, numAxons{0}, numAxonsInhibitory{0}, numAxonsExcitatory{0} {};
        };

        static const Type TYPE;

        /** \brief DendriteSkeletonInformation class virtual destructor.
         *
         */
        virtual ~DendriteSkeletonInformation()
        {};

        virtual QString type() const
        { return TYPE; }

        virtual State state() const;

        virtual Snapshot snapshot() const;

        virtual TypeList dependencies() const
        { return TypeList(); }

        virtual bool invalidateOnChange() const
        { return true; }

        virtual InformationKeyList availableInformation() const;

        virtual bool validCategory(const QString& classificationName) const
        { return classificationName.startsWith("Dendrite"); }

        virtual bool validData(const OutputSPtr output) const
        { return hasSkeletonData(output); }

        const QList<struct SpineInformation> spinesInformation() const;

      protected:
        static const QString SPINE_SNAPSHOT_FILE;

        virtual QVariant cacheFail(const InformationKey& tag) const;

        virtual void onExtendedItemSet(Segmentation* item);

        virtual void invalidateImplementation() override;

      private:
        /** \brief Computes information values and spine information.
         *
         */
        void updateInformation() const;

        /** \brief Computes spines information.
         * \param[in] definition Skeleton definition struct.
         * \param[in] paths Skeleton paths.
         * \param[in] hierarchy Skeleton paths hierarchy nodes.
         * \param[in] connections Dendrite connections.
         *
         */
        void updateSpineInformation(const Core::SkeletonDefinition         &definition,
                                    const Core::PathList                   &paths,
                                    const QList<Core::PathHierarchyNode *> &hierarchy,
                                    const Core::Connections                &connections) const;

        /** \brief DendriteSkeletonInformation class constructor.
         * \param[in] infoCache cache object.
         *
         */
        explicit DendriteSkeletonInformation(const InfoCache& infoCache = InfoCache());

        mutable QReadWriteLock                 m_mutex;  /** data protection mutex for concurrent access.                          */
        mutable InformationKeyList             m_keys;   /** information keys in this extension.                                   */
        mutable QList<struct SpineInformation> m_spines; /** spine information cache, invalidated with the rest & lazy generation. */

        friend class SkeletonInformationFactory;
    };
  
  } // namespace Extensions
} // namespace ESPINA

#endif // EXTENSIONS_DENDRITE_INFORMATION_H_
