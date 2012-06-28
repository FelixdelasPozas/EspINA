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


#ifndef IMAGELOGICFILTER_H
#define IMAGELOGICFILTER_H

#include <model/Filter.h>
#include <model/Segmentation.h>

class ImageLogicFilter
: public Filter
{
public:
  enum Operation
  {
    ADDITION,
    SUBSTRACTION,
    NOSIGN
  };
  static const QString TYPE;

  static const ModelItem::ArgumentId OPERATION;

  class ILFArguments
  : public Arguments
  {
  public:
    explicit ILFArguments(){}
    explicit ILFArguments(const Arguments args) : Arguments(args) {}

    void setOperation(Operation op)
    {
      (*this)[OPERATION] = QString::number(op);
    }

    Operation operation() const {return Operation((*this)[OPERATION].toInt());}
  };

public:
  explicit ImageLogicFilter(NamedInputs inputs, Arguments args);
  virtual ~ImageLogicFilter();


  /// Implements Model Item Interface
  virtual QString id() const;
  virtual QVariant data(int role=Qt::DisplayRole) const;
  virtual QString serialize() const;

  /// Implements Filter Interface
  void run();
  virtual int numberOutputs() const;
  virtual EspinaVolume* output(OutputNumber i) const;
  virtual bool prefetchFilter();

  virtual QWidget* createConfigurationWidget();

private:
  NamedInputs   m_inputs;
  ILFArguments  m_args;

  EspinaVolume *m_volume;
};



#endif // IMAGELOGICFILTER_H
