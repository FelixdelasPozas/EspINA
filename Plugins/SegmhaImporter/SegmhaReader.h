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

#ifndef ESPINA_SEGMHA_READER_H
#define ESPINA_SEGMHA_READER_H

#include "SegmhaImporterPlugin_Export.h"

#include <Core/Factory/AnalysisReader.h>

namespace ESPINA
{
  class SegmhaImporterPlugin_EXPORT SegmhaReader
  : public IO::AnalysisReader
  {
  public:
    virtual QString type() const
    { return "SegmharReader"; }

    virtual ExtensionList supportedFileExtensions() const;

    virtual AnalysisSPtr read(const QFileInfo& file,
                              CoreFactorySPtr  factory,
                              IO::ProgressReporter *reporter = nullptr,
                              ErrorHandlerSPtr handler = ErrorHandlerSPtr());

  private:
    struct SegmentationObject
    {
      SegmentationObject(const QString &line);

      unsigned int  label;
      unsigned int  categoryId;
      unsigned char selected;
    };

    struct CategoryObject
    {
      CategoryObject(const QString &line);

      QString      name;
      unsigned int label;
      QColor       color;
    };

  private:
    Nm                 m_inclusive[3], m_exclusive[3];
  };

  using SegmhaReaderSPtr = std::shared_ptr<SegmhaReader>;

} // namespace ESPINA

#endif // ESPINA_SEGMHA_READER_H
