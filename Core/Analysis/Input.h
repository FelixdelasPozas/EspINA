/*
 *
 * Copyright (C) 2014  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This file is part of ESPINA.

    ESPINA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef ESPINA_INPUT_H
#define ESPINA_INPUT_H

#include "Core/EspinaCore_Export.h"

// ESPINA
#include "Output.h"

namespace ESPINA {

  /** \class Input
   *
   *  \brief Allows filter outputs to be used as inputs for other filters
   *
   *  Keep valid output pointers even when filter outputs are replaced
   */
  class EspinaCore_EXPORT Input
  {
  public:
 		/** brief Input class constructor.
 		 * \param[in] filter, filter object smart pointer.
 		 * \param[in] output, output obejct smart pointer.
 		 *
 		 */
    explicit Input(FilterSPtr filter, OutputSPtr output);

 		/** brief Returns the filter associated to this object.
 		 *
 		 */
    FilterSPtr filter() const
    { return m_filter; }

 		/** brief Returns the filter associated to this object.
 		 *
 		 */
    OutputSPtr output() const
    { return m_output; }

  private:
    FilterSPtr m_filter;
    OutputSPtr m_output;
  };

  using InputSPtr  = std::shared_ptr<Input>;
  using InputSList = QList<InputSPtr>;

	/** brief Builds and returns the output of the specified filter and output id as an input.
	 * \param[in] filter, filter object smart pointer.
	 * \param[in] id, output object id.
	 *
	 */
  InputSPtr  EspinaCore_EXPORT  getInput(FilterSPtr filter, Output::Id id);

	/** brief Builds and returns the list of outputs of a filter as input objects.
	 *
	 */
  InputSList EspinaCore_EXPORT getInputs(FilterSPtr filter);
}

#endif // ESPINA_INPUT_H
