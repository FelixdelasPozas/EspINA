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

#include "SegFileReader.h"
#include <Core/IO/SegFile.h>

using namespace EspINA;
using namespace EspINA::IO;

//-----------------------------------------------------------------------------
AnalysisReader::ExtensionList SegFileReader::supportedFileExtensions() const
{
  ExtensionList supportedExtensions;

  Extensions extensions;
  extensions << "seg";

  supportedExtensions["EspINA Analysis"] = extensions;

  return supportedExtensions;
}


//-----------------------------------------------------------------------------
AnalysisSPtr SegFileReader::read(const QFileInfo& file,
                                 CoreFactorySPtr  factory,
                                 ErrorHandlerSPtr handler)
{
  return SegFile::load(file, factory, handler);
}