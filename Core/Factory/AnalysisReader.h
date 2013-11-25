/*
 *    <one line to give the program's name and a brief idea of what it does.>
 *    Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 *    This program is free software: you can redistribute it and/or modify
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


#ifndef ESPINA_ANALYSIS_READER_H
#define ESPINA_ANALYSIS_READER_H

#include "EspinaCore_Export.h"

#include <Core/Analysis/Analysis.h>
#include <Core/IO/ErrorHandler.h>

namespace EspINA
{
  namespace IO
  {
    class EspinaCore_EXPORT AnalysisReader
    {
    public:
      using Extensions  = QStringList;
      using Description = QString;
      using ExtensionDescriptionList = QStringList;
      using ExtensionList = QMap<Description, Extensions>;

    public:
      virtual ~AnalysisReader() {}

      virtual QString type() const = 0;

      ExtensionDescriptionList fileExtensionDescriptions() const
      {
        ExtensionDescriptionList list;
        ExtensionList extensions = supportedFileExtensions();

        for (auto description : extensions.keys())
        {
          list << QString("%1 (*.%2)").arg(description, extensions[description].join(" *."));
        }

        return list;
      }

      virtual ExtensionList supportedFileExtensions() const = 0;

      virtual AnalysisSPtr read(const QFileInfo& file,
                                CoreFactorySPtr  factory,
                                ErrorHandlerSPtr handler = ErrorHandlerSPtr()) = 0;
    };
  } // namespace IO

  using FileExtensions     = IO::AnalysisReader::ExtensionDescriptionList;
  using AnalysisReaderPtr  = IO::AnalysisReader *;
  using AnalysisReaderList = QList<AnalysisReaderPtr>;
  using AnalysisReaderSPtr = std::shared_ptr<IO::AnalysisReader>;

}// namespace EspINA

#endif // ESPINA_ANALYSIS_READER_H