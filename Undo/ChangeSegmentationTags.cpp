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
#include "ChangeSegmentationTags.h"

#include <Core/Analysis/Segmentation.h>
#include <Extensions/ExtensionUtils.h>
#include <Extensions/Tags/SegmentationTags.h>
#include <GUI/ModelFactory.h>

using namespace ESPINA;
using namespace ESPINA::Extensions;

//------------------------------------------------------------------------
ChangeSegmentationTags::ChangeSegmentationTags(SegmentationAdapterPtr segmentation,
                                               const QStringList&     tags,
                                               ModelFactory          *factory,
                                               QUndoCommand*          parent)
: QUndoCommand  {parent}
, m_segmentation{segmentation}
, m_tags        {tags}
, m_factory     {factory}
{
}

//------------------------------------------------------------------------
void ChangeSegmentationTags::redo()
{
  swapTags();
}

//------------------------------------------------------------------------
void ChangeSegmentationTags::undo()
{
  swapTags();
}

//------------------------------------------------------------------------
void ChangeSegmentationTags::swapTags()
{
  QStringList currentTags;

  auto extensions = m_segmentation->extensions();

  if (extensions->hasExtension(SegmentationTags::TYPE))
  {
    currentTags = retrieveExtension<SegmentationTags>(extensions)->tags();
  }

  if(!m_tags.isEmpty())
  {
    SegmentationTagsSPtr tagsExtension = nullptr;

    if (!extensions->hasExtension(SegmentationTags::TYPE))
    {
      auto extension = m_factory->createSegmentationExtension(SegmentationTags::TYPE);
      extensions->add(extension);
    }

    tagsExtension = retrieveExtension<SegmentationTags>(extensions);
    tagsExtension->setTags(m_tags);
  }
  else
  {
    if(!currentTags.isEmpty())
    {
      safeDeleteExtension<SegmentationTags>(m_segmentation);
    }
  }

  m_tags = currentTags;

  m_segmentation->notifyModification();
}
