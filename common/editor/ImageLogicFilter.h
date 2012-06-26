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


#ifndef IMAGELOGICFILTER_H
#define IMAGELOGICFILTER_H

#include <model/Filter.h>
#include <model/Segmentation.h>

static const QString ILF = "EditorToolBar::ImageLogicFilter";

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

  class ILFArguments;

public:
  explicit ImageLogicFilter(QList<Segmentation *> input, Operation op);
  explicit ImageLogicFilter(Arguments args);
  virtual ~ImageLogicFilter();

  void run();

  /// Implements Model Item Interface
  virtual QString id() const;
  virtual QVariant data(int role) const;
  virtual QString serialize() const;

  /// Implements Filter Interface
  virtual int numProducts() const;
  virtual Segmentation* product(int index) const;
  virtual int numberOutputs() const{}
  virtual EspinaVolume* output(int i) const{}

  virtual pqData preview();
  virtual QWidget* createConfigurationWidget();

protected:
virtual vtkAlgorithmOutput* output(unsigned int outputNb){ Q_ASSERT(false);return NULL;}

private:
  ILFArguments *m_args;

  pqFilter     *m_filter;
  Segmentation *m_seg;
};


class ImageLogicFilter::ILFArguments
: public Arguments
{
public: 
  static const ModelItem::ArgumentId INPUT;
  static const ModelItem::ArgumentId OPERATION;

public:
  explicit ILFArguments(){}
  explicit ILFArguments(const Arguments args);

  void setInput(QList<Segmentation *> input)
  {
    QString inputs;
    foreach(Segmentation *seg, input)
    {
      if (!inputs.isEmpty())
	inputs.append(",");
      inputs.append(seg->id());
    }
    (*this)[INPUT] = inputs;
  }

  void setOperation(Operation op)
  {
    m_operation = op;
    (*this)[OPERATION] = QString::number(op);
  }

  Operation operation() const {return m_operation;}

private:
  Operation m_operation;
};

#endif // IMAGELOGICFILTER_H
