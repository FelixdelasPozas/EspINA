/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef ESPINA_ID_UTILS_H
#define ESPINA_ID_UTILS_H

// ESPINA
#include <Core/Analysis/Analysis.h>
#include <Core/Utils/AnalysisUtils.h>

// Qt
#include <QInputDialog>

namespace ESPINA
{
	/* \brief Sets the id of an item with a previous user confirmation.
	 * \param[in] item, item to set id.
	 * \param[in] id, id to seek confirmation.
	 * \param[in] list, list of items.
	 *
	 */
  template<typename I, typename L>
  void SetUniqueIdWithUserConfirmation(I& item, QString id, const L& list)
  {
    bool alreadyUsed = false;
    bool accepted    = true;

    for (auto listItem : list)
    {
      if (listItem != item)
        alreadyUsed |= item->id() == id;
    }

    if (alreadyUsed)
    {
      QString suggestedId = SuggestId(id, list);
      while (suggestedId != id)
      {
        id = QInputDialog::getText(nullptr,
                                   QObject::tr("Id already used"),
                                   QObject::tr("Introduce new id (or accept suggested one)"),
                                   QLineEdit::Normal,
                                   suggestedId,
                                   &accepted);
        suggestedId = SuggestId(id, list);
      }
    }

    item->setId(id);
  }
}


#endif // ESPINA_NM_VECTOR_3_H
