/*

 Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>

 This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ESPINA_MORPHOLOGICAL_EDITION_FILTER_H
#define ESPINA_MORPHOLOGICAL_EDITION_FILTER_H

#include "Filters/EspinaFilters_Export.h"

// ESPINA
#include "Core/Analysis/Filter.h"

// ITK
#include <itkImageToImageFilter.h>
#include <itkCommand.h>

namespace ESPINA
{
  class EspinaFilters_EXPORT MorphologicalEditionFilter
  : public Filter
  {
      Q_OBJECT
  public:
    /** \brief MorphologicalEditionFilter class virtual destructor.
     *
     */
    virtual ~MorphologicalEditionFilter();

    virtual void restoreState(const State& state);

    virtual State state() const;

    /** \brief Returns the radius of the morphological operation.
     *
     */
    unsigned int radius() const
    { return m_radius; }

    /** \brief Sets the radius of the morphological operation
     * \param[in] radius radius of the morphological operation.
     *
     */
    void setRadius(int radius);

    /** \brief Returs true if the output is empty.
     *
     * Morphological operations like erode can destroy the segmentation.
     *
     */
    bool isOutputEmpty()
    { return m_isOutputEmpty; }

  signals:
    void radiusModified(int value);

  protected:
    /** \brief MorphologicalEditionFilter class constructor.
     * \param[in] inputs list of input smart pointers.
     * \param[in] type type of the morphological operation.
     * \param[in] scheduler scheduler smart pointer.
     *
     */
    explicit MorphologicalEditionFilter(InputSList          inputs,
                                        const Filter::Type &type,
                                        SchedulerSPtr       scheduler);

    virtual Snapshot saveFilterSnapshot() const;

    virtual bool needUpdate() const;

    virtual bool needUpdate(Output::Id id) const;

    virtual bool ignoreStorageContent() const;

    virtual bool areEditedRegionsInvalidated();

    /** \brief Checks if the output is empty after execution
     * and creates the output if it's not.
     *
     */
    void finishExecution(itkVolumeType::Pointer output);

  protected:
    int  m_radius, m_prevRadius;
    bool m_isOutputEmpty;
  };

  using MorphologicalEditionFilterPtr  = MorphologicalEditionFilter *;
  using MorphologicalEditionFilterSPtr = std::shared_ptr<MorphologicalEditionFilter>;
} // namespace ESPINA

#endif // ESPINA_MORPHOLOGICAL_EDITION_FILTER_H
