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

#include "io_testing_support.h"

#include <Core/Analysis/Analysis.h>

using namespace std;
using namespace EspINA;

bool operator!=(Analysis& lhs, Analysis& rhs)
{
  if (print(lhs.classification()) != print(rhs.classification()))
  {
    cerr << "Classification missmatch" << endl;
    return true;
  }

  if (lhs.samples().size() != rhs.samples().size())
  {
    cerr << "Samples size missmatch" << endl;
    return true;
  }

  if (lhs.channels().size() != rhs.channels().size())
  {
    cerr << "Channels size missmatch" << endl;
    return true;
  }

  if (lhs.segmentations().size() != rhs.segmentations().size())
  {
    cerr << "Segmentations size missmatch" << endl;
    return true;
  }

  if (lhs.content()->vertices().size() != rhs.content()->vertices().size())
  {
    cerr << "Content vertices size missmatch" << endl;
    return true;
  }

  if (lhs.content()->edges().size() != rhs.content()->edges().size())
  {
    cerr << "Contetn edges size missmatch" << endl;
    return true;
  }

  if (lhs.relationships()->vertices().size() != rhs.relationships()->vertices().size())
  {
    cerr << "Relationships vertices size missmatch" << endl;
    return true;
  }

  if (lhs.relationships()->edges().size() != rhs.relationships()->edges().size())
  {
    cerr << "Relationships edges size missmatch" << endl;
    return true;
  }

  return false;
}
