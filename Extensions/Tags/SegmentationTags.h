/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef ESPINA_SEGMENTATION_TAGS
#define ESPINA_SEGMENTATION_TAGS

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/Analysis/Extensions.h>
#include <GUI/Model/SegmentationAdapter.h>

// Qt
#include <QStringListModel>

namespace ESPINA
{
  namespace Extensions
  {
    class SegmentationTagsFactory;

    /** \class SegmentationTags
     * \brief Extends the Segmentation class with tagging information.
     *
     */
    class EspinaExtensions_EXPORT SegmentationTags
    : public Core::SegmentationExtension
    {
      public:
        static const Key TAGS;
        static const Type TYPE;

        /** \brief returns the list of available tags.
         *
         */
        static QStringList availableTags();

      public:
        /** \brief SegmentationTags class virtual destructor.
         *
         */
        virtual ~SegmentationTags();

        virtual Type type() const override
        { return TYPE; }

        virtual bool invalidateOnChange() const override
        { return false; }

        virtual State state() const override
        { return m_tags.join(";"); }

        virtual Snapshot snapshot() const override
        { return Snapshot(); }

        virtual const TypeList dependencies() const override
        { return TypeList(); }

        virtual bool validCategory(const QString& classificationName) const
        { return true; }

        virtual bool validData(const OutputSPtr output) const
        { return true; }

        virtual const InformationKeyList availableInformation() const override;

        virtual const QString toolTipText() const override;

        /** \brief Adds a tag.
         * \param[in] tag text string.
         *
         */
        void addTag(const QString &tag);

        /** \brief Adds multiple tags.
         * \param[in] tags text string list.
         *
         */
        void addTags(const QStringList &tags);

        /** \brief Removes a tag.
         * \param[in] tag text string.
         *
         */
        void removeTag(const QString &tag);

        /** \brief Sets the tags.
         * \param[in] tags text string list.
         *
         */
        void setTags(const QStringList &tags);

        /** \brief Returns the tags.
         *
         */
        QStringList tags() const;

      protected:
        virtual void onExtendedItemSet(SegmentationPtr item) override;

        virtual QVariant cacheFail(const InformationKey& tag) const override;

      private:
        /** \brief SegmentationTags class constructor.
         * \param[in] infoCache, cache object.
         *
         */
        explicit SegmentationTags(const InfoCache& infoCache = InfoCache());

        /** \brief Returns trimmed tag (spaces removed at the beginning and end of the string).
         * \param[in] tag text string.
         *
         */
        void addTagImplementation(const QString &tag);

        /** \brief Adds the tags to the available tags counter.
         * \param[in] tag text string.
         *
         */
        void addToAvailableTags(const QString &tag);

        /** \brief Removes the tag from the available tags counter.
         * \param[in] tag text string.
         *
         */
        void removeFromAvailableTags(const QString &tag);

        QStringList m_tags;

        static QReadWriteLock s_mutex;
        static QMap<QString, unsigned int> s_availableTags;

        friend class SegmentationTagsFactory;
    };

    using SegmentationTagsPtr  = SegmentationTags *;
    using SegmentationTagsSPtr = std::shared_ptr<SegmentationTags>;
  } // namespace Extensions
} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_TAGS
