/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jorge Peña Pastor <jpena@cesvima.upm.es>
 *
 * This program is free software: you can redistribute it and/or modify
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

#ifndef ESPINA_SEGFILE_V5_H
#define ESPINA_SEGFILE_V5_H

#include "Core/IO/SegFileInterface.h"
#include <Core/Analysis/Storage.h>

namespace EspINA {
  namespace IO {
    namespace SegFile {
      class SegFile_V5
      : public SegFileInterface
      {
      public:
        static const QString FORMAT_INFO_FILE;

      public:
        virtual AnalysisSPtr load(QuaZip&         zip,
                                  CoreFactorySPtr factory = CoreFactorySPtr(),
                                  ErrorHandlerPtr handler = nullptr);

        virtual void save(AnalysisPtr     analysis, 
                          QuaZip&         zip,
                          ErrorHandlerPtr handler = nullptr);
      private:
        void loadContent(AnalysisSPtr            analysis,
                         QuaZip&                 zip,
                         Persistent::StorageSPtr storage,
                         CoreFactorySPtr         factory,
                         ErrorHandlerPtr         handler = nullptr);

        void loadRelations(AnalysisSPtr    analysis,
                           QuaZip&         zip,
                           ErrorHandlerPtr handler = nullptr);

        PersistentSPtr parseVertex(AnalysisSPtr          analysis,
                                   DirectedGraphSPtr     content,
                                   DirectedGraph::Vertex roVertex,
                                   CoreFactorySPtr       factory, 
                                   ErrorHandlerPtr       handler = nullptr);
      };
    }
  }
}

#endif // ESPINA_SEGFILE_V5_H
