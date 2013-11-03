/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2013 Félix de las Pozas Álvarez <felixdelaspozas@gmail.com>

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

#include "Core/Analysis/Sample.h"
#include "Core/Analysis/Channel.h"
#include "Core/Analysis/Filter.h"
#include "Core/Analysis/Segmentation.h"
#include "Core/Analysis/Analysis.h"
#include "Core/Analysis/Classification.h"
#include "Core/Analysis/Category.h"
#include <Core/MultiTasking/Scheduler.h>

#include <iostream>
#include <QDebug>

using namespace EspINA;

class DummyFilter
: public Filter
{
  public:
    explicit DummyFilter()
    : Filter(OutputSList(), "Dummy", SchedulerSPtr(new Scheduler(10000000)))
    , m_output(new Output(this, 0)) {}
    virtual OutputSPtr output(Output::Id id) const { return m_output; }

  protected:
    virtual Snapshot saveFilterSnapshot() const{}
    virtual bool needUpdate() const {}
    virtual bool needUpdate(Output::Id id) const {}
    virtual DataSPtr createDataProxy(Output::Id id, const Data::Type& type) {}
    virtual void execute() {}
    virtual void execute(Output::Id id) {}
    virtual bool invalidateEditedRegions(){ return false; }

  private:
    OutputSPtr m_output;
};

int segmentation_save_state(int argc, char** argv)
{
  FilterSPtr filter{new DummyFilter()};
  SegmentationSPtr segmentation{new Segmentation(filter, 0)};

  State forgedState;
  forgedState = QString("ID=") + segmentation->uuid().toString() + QString(";");

  segmentation->setNumber(1);
  forgedState += QString("NUMBER=1;");

  segmentation->modifiedByUser("AUser");
  forgedState += QString("USERS=AUser;");

  forgedState += QString("OUTPUT=0;");

  Classification classification;
  classification.createCategory(QString("Prueba"), classification.root());

  segmentation->setCategory(classification.category(QString("Prueba")));
  forgedState += QString("CATEGORY=Prueba;");

  return (segmentation->saveState() != forgedState);
}
