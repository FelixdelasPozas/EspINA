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

#ifndef ESPINA_ANALYSIS_UTILS_H
#define ESPINA_ANALYSIS_UTILS_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include <Core/Analysis/Analysis.h>

namespace ESPINA
{
  /** \brief Merges two analysis into one.
   * \param[in] lhs analysis smart pointer.
   * \param[in] rhs analysis smart pointer.
   *
   *  WARNING: merged analysis references will be invalidated.
   *
   */
  AnalysisSPtr EspinaCore_EXPORT merge(AnalysisSPtr& lhs, AnalysisSPtr& rhs);

  /** \brief Returns the sample with the same name in a list of sample smart pointers.
   * \param[in] sample sample smart pointer.
   * \param[in] samples list of sample smart pointers.
   *
   */
  SampleSPtr EspinaCore_EXPORT findSample(SampleSPtr sample, SampleSList samples);

  /** \brief Returns the channel with the same name in a list of channel smart pointers.
   * \param[in] channel sample smart pointer.
   * \param[in] channels list of channel smart pointers.
   *
   */
  ChannelSPtr EspinaCore_EXPORT findChannel(ChannelSPtr sample, ChannelSList channels);

  /** \brief Returns the smart pointer that contains a specified raw pointer in a list of smart pointers.
   * \param[in] item T raw pointer.
   * \param[in] list list of T smart pointers.
   *
   */
  template<typename T>
  std::shared_ptr<T> find(T *item, QList<std::shared_ptr<T>> list)
  {
    for(auto smartItem : list)
    {
      if (smartItem.get() == item) return smartItem;
    }

    return std::shared_ptr<T>();
  }

  /** \brief Returns the suggested cardinality of an id into a list of elements.
   * \param[in] id query id
   * \param[in] list list of elements.
   *
   */
  template<typename T>
  QString SuggestId(const QString& id, const T& list)
  {
    QRegExp regExp("\\([0-9]+\\)");
    int strippedNum = 0;
    QString strippedId = id;
    auto pos = regExp.lastIndexIn(id);
    if(pos == -1)
    {
      strippedNum = regExp.capturedTexts().last().mid(1, regExp.capturedTexts().last().length()-2).toInt();
      strippedId = strippedId.remove(pos, regExp.capturedTexts().last().length()).simplified();
    }

    int count = 0;
    for(auto item: list)
    {
      int itemNum = 0;
      QString itemId = item->id();

      auto idPos = regExp.lastIndexIn(itemId);
      if(idPos != -1)
      {
        itemNum = regExp.capturedTexts().last().mid(1, regExp.capturedTexts().last().length()-2).toInt();
        itemId = itemId.remove(pos, regExp.capturedTexts().last().length()).simplified();
      }

      if(itemId.startsWith(strippedId))
      {
        count = std::max(count, itemNum + 1);
      }
    }

    if(list.isEmpty())
    {
      count = strippedNum;
    }

    auto suggestedId = id;
    if(strippedNum != 0)
    {
      pos = regExp.lastIndexIn(id);
      suggestedId.replace(pos + 1, regExp.capturedTexts().last().length() - 2, QString::number(count));
    }
    else
    {
      if(count != 0) suggestedId += QObject::tr(" (%1)").arg(count);
    }

    return suggestedId;
  }

  /** \brief Returns the first unused unsigned integer number for a segmentation in an analysis.
   * \param[in] analysis analysis smart pointer.
   *
   */
  unsigned int EspinaCore_EXPORT firstUnusedSegmentationNumber(const AnalysisSPtr analysis);

  /** \brief Returns the axon connection of the given synapse or nullptr if the synapse is not yet connected to an axon.
   * \param[in] synapse Synapse category segmentation raw pointer.
   *
   */
  SegmentationSPtr EspinaCore_EXPORT axonOf(const SegmentationPtr synapse);

  /** \brief Returns the dendrite connection of the given synapse or nullptr if the synapse is not yet connected to a dendrite.
   * \param[in] synapse Synapse category segmentation raw pointer.
   *
   */
  SegmentationSPtr EspinaCore_EXPORT dendriteOf(const SegmentationPtr synapse);

}


#endif // ESPINA_NM_VECTOR_3_H
