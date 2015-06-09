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

#ifndef ESPINA_CHANGE_SEGMENTATION_TAGS_H
#define ESPINA_CHANGE_SEGMENTATION_TAGS_H

#include "Undo/EspinaUndo_Export.h"

// ESPINA
#include <GUI/Model/SegmentationAdapter.h>

// Qt
#include <QUndoCommand>
#include <QStringList>

namespace ESPINA
{
  class EspinaUndo_EXPORT ChangeSegmentationTags
  : public QUndoCommand
  {
  public:
  	/** \brief ChangeSegmentationTags class constructor.
  	 * \param[in] segmentation, raw pointer of the segmentation adapter.
  	 * \param[in] tags, list of new tags.
  	 * \param[in] parent, raw pointer of the QUndoCommand parent of this one.
  	 *
  	 */
    explicit ChangeSegmentationTags(SegmentationAdapterPtr segmentation,
                                    const QStringList&  tags,
                                    QUndoCommand*       parent = nullptr);

    /** \brief Overrides QUndoCommand::redo().
     *
     */
    virtual void redo() override;

    /** \brief Overrides QUndoCommand::undo().
     *
     */
    virtual void undo() override;

  private:
    /** \brief Helper method to swap new-old tags.
     *
     */
    void swapTags();

  private:
    SegmentationAdapterPtr m_segmentation;
    QStringList            m_tags;
  };
} // namespace ESPINA

#endif // CHANGESEGMENTATIONTAGS_H
