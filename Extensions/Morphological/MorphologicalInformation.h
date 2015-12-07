/*
 *
 *    Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_MORPHOLOGICAL_INFORMATION_H
#define ESPINA_MORPHOLOGICAL_INFORMATION_H

#include "Extensions/EspinaExtensions_Export.h"

// ESPINA
#include <Core/Analysis/Extensions.h>

// ITK
#include <itkLabelImageToShapeLabelMapFilter.h>
#include <itkStatisticsLabelObject.h>

namespace ESPINA
{
  class EspinaExtensions_EXPORT MorphologicalInformation
  : public SegmentationExtension
  {
    using LabelObjectType = itk::StatisticsLabelObject<unsigned int, 3>;
    using LabelMapType    = itk::LabelMap<LabelObjectType>;
    using Image2LabelFilterType = itk::LabelImageToShapeLabelMapFilter<itkVolumeType, LabelMapType>;

  public:
    static const Type TYPE;

  public:
    /** \brief MorphologicalInformation class constructor.
     * \param[in] cache, cache object for the extension.
     * \param[in] state, state object of the extension.
     */
    explicit MorphologicalInformation(const InfoCache &cache = InfoCache(),
                                      const State     &state = State());

    /** \brief MorphologicalInformation class virtual destructor.
     *
     */
    virtual ~MorphologicalInformation();

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
    { return true;}

  protected:
    virtual QVariant cacheFail(const InformationKey& tag) const;

    virtual void onExtendedItemSet(Segmentation* item);

  private:
    /** \brief Computes information values.
     *
     */
    void updateInformation() const;

  private:
    mutable QReadWriteLock m_mutex;

    Image2LabelFilterType::Pointer m_labelMap;
    mutable LabelObjectType       *m_statistic;
  };

}// namespace ESPINA

#endif // ESPINA_MORPHOLOGICAL_INFORMATION_H
