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

// ESPINA
#include "ChangeSegmentationNotes.h"
#include <Core/Analysis/Segmentation.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Notes/SegmentationNotes.h>
#include <GUI/Model/CategoryAdapter.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;

//------------------------------------------------------------------------
ChangeSegmentationNotes::ChangeSegmentationNotes(SegmentationAdapterPtr segmentation,
                                                 const QString&         note,
                                                 ModelFactory          *factory,
                                                 QUndoCommand*          parent)
: QUndoCommand  {parent}
, m_segmentation{segmentation}
, m_formerNote  {note}
, m_factory     {factory}
{
}

//------------------------------------------------------------------------
void ChangeSegmentationNotes::redo()
{
  swapNotes();
}

//------------------------------------------------------------------------
void ChangeSegmentationNotes::undo()
{
  swapNotes();
}

//------------------------------------------------------------------------
void ChangeSegmentationNotes::swapNotes()
{
  QString currentNote;

  {
    auto extensions = m_segmentation->extensions();

    if (extensions->hasExtension(SegmentationNotes::TYPE))
    {
      currentNote = extensions->get<SegmentationNotes>()->notes();
    }
  }

  if (currentNote.isEmpty() && !m_formerNote.isEmpty())
  {
    auto notesExtension = retrieveOrCreateSegmentationExtension<SegmentationNotes>(m_segmentation, m_factory);
    notesExtension->setNotes(m_formerNote);
  }
  else
  {
    if (!currentNote.isEmpty() && !m_formerNote.isEmpty())
    {
      auto notesExtension = retrieveOrCreateSegmentationExtension<SegmentationNotes>(m_segmentation, m_factory);
      notesExtension->setNotes(m_formerNote);
    }
    else
    {
      if (!currentNote.isEmpty() && m_formerNote.isEmpty())
      {
        safeDeleteExtension<SegmentationNotes>(m_segmentation);
      }
    }
  }

  m_formerNote = currentNote;

  m_segmentation->notifyModification();
}
