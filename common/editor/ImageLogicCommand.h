/*
 <one line to give the program's name and a brief idea of what it does.>
 Copyright (C) 2012  Jorge Peña Pastor <jpena@cesvima.upm.es>

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

#ifndef IMAGELOGICCOMMAND_H
#define IMAGELOGICCOMMAND_H

#include <QUndoStack>

#include "ImageLogicFilter.h"

class Segmentation;

class ImageLogicCommand: public QUndoCommand
{
public:
  explicit ImageLogicCommand(QList<Segmentation *> segmentations,
                             ImageLogicFilter::Operation op);

  virtual void redo();
  virtual void undo();

private:
  ImageLogicFilter     *m_filter;
  Segmentation         *m_seg;
  TaxonomyNode         *m_tax;
  QList<Segmentation *> m_input;
};

#endif // IMAGELOGICCOMMAND_H
