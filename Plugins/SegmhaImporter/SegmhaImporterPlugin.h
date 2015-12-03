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
#ifndef ESPINA_SEGMHA_IMPORTER_H
#define ESPINA_SEGMHA_IMPORTER_H

#include "SegmhaImporterPlugin_Export.h"

// ESPINA
#include <Support/Plugin.h>
#include "SegmhaReader.h"

// Qt
#include <QUndoCommand>

namespace ESPINA
{
  /** \class SegmhaFilterFactory
   * \brief Factory for Segmha importer filters.
   *
   */
  class SegmhaImporterPlugin_EXPORT SegmhaFilterFactory
  : public FilterFactory
  {
    public:
      static const Filter::Type SEGMHA_FILTER;    /** segmha filter signature. */
      static const Filter::Type SEGMHA_FILTER_V4; /** segmha filter old signature. */

      virtual FilterTypeList providedFilters() const;

      virtual FilterSPtr createFilter(InputSList inputs, const Filter::Type& filter, SchedulerSPtr scheduler) const throw (Unknown_Filter_Exception);
    private:
      mutable DataFactorySPtr m_dataFactory; /** data factory for this filters provider */
  };

  /** \class SegmhaImporterPlugin
   * \brief Plugin to import old segmha EspINA files.
   *
   */
  class SegmhaImporterPlugin_EXPORT SegmhaImporterPlugin
  : public Support::Plugin
  {
      Q_OBJECT
      Q_INTERFACES(ESPINA::Support::Plugin)

    public:
      /** \brief Class SegmhaImporterPlugin class constructor.
       *
       */
      explicit SegmhaImporterPlugin();

      /** \brief Class SegmhaImporterPlugin class virtual destructor.
       *
       */
      virtual ~SegmhaImporterPlugin();

      virtual void init(Support::Context &context);

      virtual FilterFactorySList filterFactories() const;

      virtual AnalysisReaderSList analysisReaders() const;
  };
} // namespace ESPINA

#endif// SEGMHAIMPORTER_H
