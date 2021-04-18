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
#include "SegmentationNotes.h"
#include <GUI/Utils/Format.h>

using namespace ESPINA;
using namespace ESPINA::Core;
using namespace ESPINA::Extensions;
using namespace ESPINA::GUI::Utils::Format;

const QString SegmentationNotes::TYPE  = "SegmentationNotes";

const SegmentationExtension::InformationKey  SegmentationNotes::NOTES(SegmentationNotes::TYPE, "Notes");

//------------------------------------------------------------------------
SegmentationNotes::SegmentationNotes(const InfoCache& infoCache)
: SegmentationExtension(infoCache)
{
}

//------------------------------------------------------------------------
const SegmentationExtension::InformationKeyList SegmentationNotes::availableInformation() const
{
  InformationKeyList keys;

  keys << NOTES;

  return keys;
}

//------------------------------------------------------------------------
const QString SegmentationNotes::toolTipText() const
{
  const QString WS  = "&nbsp;"; // White space
  const QString TAB = WS+WS+WS;

  QString toolTip;

  if (!notes().isEmpty())
  {
    QString firstLine = notes().left(20);
    if (firstLine.length() == 20)
      firstLine = firstLine.replace(17, 3, "...");
    toolTip = createTable(":/espina/note.svg", firstLine);
  }

  return toolTip;
}


//------------------------------------------------------------------------
void SegmentationNotes::setNotes(const QString &note)
{
  updateInfoCache(NOTES, note);
}

//------------------------------------------------------------------------
QVariant SegmentationNotes::cacheFail(const InformationKey& key) const
{
  return QVariant();
}
