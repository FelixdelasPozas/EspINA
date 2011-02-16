/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Jorge Peña <jorge.pena.pastor@gmail.com>

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


#ifndef ESPINA_H
#define ESPINA_H

#include <QMap>
#include <QVector>
#include <QString>

class ProcessingTrace;
class TaxonomyNode;
//! Espina Interactive Neuron Analyzer
class EspINA
{
public:
  typedef enum {ELECTRONE, OPTICAL} Microscopy;
public:
    EspINA(Microscopy type);
    virtual ~EspINA();
    
    // Sample managing
    void loadSample();
    
private:
  TaxonomyNode *m_tax;
  QMap<QString, QVector<QString> > m_segmentations;
  ProcessingTrace *m_analysis;
};

#endif // ESPINA_H
