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


#ifndef ADDSEGMENTATION_H
#define ADDSEGMENTATION_H

#include <QUndoCommand>

// Forward declarations
class Channel;
class EspinaModel;
class Filter;
class Sample;
class Segmentation;
class TaxonomyElement;

class AddSegmentation
: public QUndoCommand
{
public:
  explicit AddSegmentation(Channel         *channel,
                           Filter          *filter,
                           Segmentation    *seg,
                           TaxonomyElement *taxonomy,
                           EspinaModel     *model
                          );
  virtual void redo();
  virtual void undo();

private:
  Sample          *m_sample;
  Channel         *m_channel;
  Filter          *m_filter;
  Segmentation    *m_seg;
  TaxonomyElement *m_taxonomy;
  EspinaModel     *m_model;
};

#endif // ADDSEGMENTATION_H
