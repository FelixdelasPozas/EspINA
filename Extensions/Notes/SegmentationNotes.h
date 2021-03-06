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

#ifndef ESPINA_SEGMENTATION_NOTES_H
#define ESPINA_SEGMENTATION_NOTES_H

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/Analysis/Extensions.h>

namespace ESPINA
{
  namespace Extensions
  {
    class SegmentationNotesFactory;

    /** \class SegmentationNotes
     * \brief Implements a extension to add textual notes to a segmentation.
     *
     */
    class EspinaExtensions_EXPORT SegmentationNotes
    : public Core::SegmentationExtension
    {
    public:
      static const Type TYPE;

      // Stores the notes of a segmentation
      static const InformationKey NOTES;

    public:
      /** \brief SegmentationNotes class virtual destructor.
       *
       */
      virtual ~SegmentationNotes()
      {}

      virtual Type type() const override
      { return TYPE; }

      virtual bool invalidateOnChange() const override
      { return false; }

      virtual State state() const override
      { return State(); }

      virtual Snapshot snapshot() const override
      { return Snapshot(); }

      virtual const TypeList dependencies() const override
      { return TypeList(); }

      virtual bool validCategory(const QString& classification) const
      { return true; }

      virtual bool validData(const OutputSPtr output) const
      { return true; }

      virtual const InformationKeyList availableInformation() const override;

      virtual const QString toolTipText() const override;

      /** \brief Sets the notes.
       *
       */
      void setNotes(const QString &note);

      /** \brief Returns the notes.
       *
       */
      QString notes() const
      { return cachedInfo(createKey(NOTES)).toString(); }

    protected:
      virtual void onExtendedItemSet(SegmentationPtr item) override
      {}

      virtual QVariant cacheFail(const InformationKey& key) const override;

    private:
      /** \brief SegmentationNotes class constructor.
       * \param[in] infoCache cache object.
       *
       */
      explicit SegmentationNotes(const InfoCache& infoCache = InfoCache());

      friend class SegmentationNotesFactory;
    };

    using NotesExtensionPtr  = SegmentationNotes *;
    using NotesExtensionSPtr = std::shared_ptr<SegmentationNotes>;

  } // namespace Extensions
} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_NOTES_H
