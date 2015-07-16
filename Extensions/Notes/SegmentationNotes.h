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
#include <Core/Analysis/Extension.h>

namespace ESPINA
{

  class EspinaExtensions_EXPORT SegmentationNotes
  : public SegmentationExtension
  {
  public:
    static const Key  NOTES;
    static const Type TYPE;

  public:
    /** \brief SegmentationNotes class constructor.
     * \param[in] infoCache cache object.
     *
     */
    explicit SegmentationNotes(const InfoCache& infoCache = InfoCache());

    /** \brief SegmentationNotes class virtual destructor.
     *
     */
    virtual ~SegmentationNotes();

    virtual Type type() const
    { return TYPE; }

    virtual bool invalidateOnChange() const
    { return false; }

    virtual State state() const
    { return State(); }

    virtual Snapshot snapshot() const
    { return Snapshot(); }

    virtual TypeList dependencies() const
    { return TypeList(); }

    virtual bool validCategory(const QString& classification) const
    { return true; }

    virtual KeyList availableInformation() const;

    virtual QString toolTipText() const override;

    /** \brief Sets the notes.
     *
     */
    void setNotes(const QString &note);

    /** \brief Returns the notes.
     *
     */
    QString notes() const
    { return cachedInfo(NOTES).toString(); }

  protected:
    virtual void onExtendedItemSet(Segmentation* item)
    {}

    virtual QVariant cacheFail(const QString& key) const;
  };
} // namespace ESPINA

#endif // ESPINA_SEGMENTATION_NOTES_H
