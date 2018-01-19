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
  /** \class SegmhaReader
   * \brief Implements a reader for old EspINA segmha file format.
   *
   */
  class SegmhaImporterPlugin_EXPORT SegmhaReader
  : public IO::AnalysisReader
  {
    public:
      virtual QString type() const
      { return "SegmhaReader"; }

      virtual ExtensionList supportedFileExtensions() const;

      virtual AnalysisSPtr read(const QFileInfo& file,
                                CoreFactorySPtr  factory,
                                IO::ProgressReporter *reporter = nullptr,
                                ErrorHandlerSPtr handler = ErrorHandlerSPtr());

    private:
      /** \struct SegmentationObject
       * \brief Segmentation object data.
       *
       */
      struct SegmentationObject
      {
        /** \brief SegmentationObject struct constructor.
         * \param[in] line QString containing the segmentation data joined by ";".
         *
         */
        explicit SegmentationObject(const QString &line);

        unsigned int  label;      /** segmentation numbert.              */
        unsigned int  categoryId; /** segmentation category id.          */
        unsigned char selected;   /** true if selected, false otherwise. */
      };

      /** \brief CategoryObject
       * \brief Category object data (Taxomony).
       */
      struct CategoryObject
      {
        /** \brief CategoryObject struct constructor.
         * \param[in] line QString containing the category data joined by ";".
         *
         */
        explicit CategoryObject(const QString &line);

        QString      name;  /** category name.  */
        unsigned int label; /** category id.    */
        QColor       color; /** category color. */
      };

    private:
      Nm m_inclusive[3]; /** Counting frame inclusion margins. */
      Nm m_exclusive[3]; /** Counting frame exclusion margins. */
  };

  using SegmhaReaderSPtr = std::shared_ptr<SegmhaReader>;

} // namespace ESPINA

#endif // ESPINA_SEGMHA_READER_H
