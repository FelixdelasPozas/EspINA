/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013  <copyright holder> <email>

    This program is free software: you can redistribute it and/or modify
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


#ifndef UNLOADSAMPLECOMMAND_H
#define UNLOADSAMPLECOMMAND_H

#include <QtGui/QUndoStack>
#include <Core/Model/Sample.h>


namespace EspINA
{

  class UnloadSampleCommand
  : public QUndoCommand
  {
  public:
    explicit UnloadSampleCommand(SampleSPtr    sample,
                                 EspinaModel  *model,
                                 QUndoCommand *parent = 0);

  virtual void redo();

  virtual void undo();

  private:
    EspinaModel *m_model;
    SampleSPtr   m_sample;
  };

} // namespace EspINA

#endif // UNLOADSAMPLECOMMAND_H
