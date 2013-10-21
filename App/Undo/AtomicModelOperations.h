/*
    Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
        * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
        * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#ifndef ATOMICMODELOPERATIONS_H
#define ATOMICMODELOPERATIONS_H

#include <QUndoCommand>

#include <Core/Model/EspinaModel.h>

namespace EspINA
{
  class ModelAdapter;

#define AtomicCommand(TYPE)                                \
  class Add##TYPE##Command                                 \
  : public QUndoCommand                                    \
  {                                                        \
  public:                                                  \
    explicit Add##TYPE##Command(TYPE##SPtr item,           \
                              EspinaModel   *model,        \
                              QUndoCommand  *parent = 0)   \
    : QUndoCommand(parent)                                 \
    , m_model(model)                                       \
    { m_items << item; }                                   \
                                                           \
    explicit Add##TYPE##Command(TYPE##SList items,         \
                              EspinaModel    *model,       \
                              QUndoCommand   *parent = 0)  \
    : QUndoCommand(parent)                                 \
    , m_model(model)                                       \
    , m_items(items) {}                                    \
                                                           \
    virtual void redo()                                    \
    { m_model->add##TYPE(m_items); }                       \
                                                           \
    virtual void undo()                                    \
    {                                                      \
      foreach (TYPE##SPtr item, m_items)                   \
        m_model->remove##TYPE(item);                       \
    }                                                      \
                                                           \
  private:                                                 \
    EspinaModel *m_model;                                  \
    TYPE##SList  m_items;                                  \
  };                                                       \
  class Remove##TYPE##Command                              \
  : public QUndoCommand                                    \
  {                                                        \
  public:                                                  \
    explicit Remove##TYPE##Command(TYPE##SPtr item,        \
                              EspinaModel   *model,        \
                              QUndoCommand  *parent = 0)   \
    : QUndoCommand(parent)                                 \
    , m_model(model)                                       \
    { m_items << item; }                                   \
                                                           \
    explicit Remove##TYPE##Command(TYPE##SList items,      \
                              EspinaModel    *model,       \
                              QUndoCommand   *parent = 0)  \
    : QUndoCommand(parent)                                 \
    , m_model(model)                                       \
    , m_items(items) {}                                    \
                                                           \
    virtual void redo()                                    \
    {                                                      \
      foreach (TYPE##SPtr item, m_items)                   \
        m_model->remove##TYPE(item);                       \
    }                                                      \
                                                           \
    virtual void undo()                                    \
    { m_model->add##TYPE(m_items); }                       \
                                                           \
  private:                                                 \
    EspinaModel *m_model;                                  \
    TYPE##SList  m_items;                                  \
  };

  AtomicCommand(Sample)

  AtomicCommand(Channel)

  AtomicCommand(Segmentation)

  AtomicCommand(Filter)

  class AddRelationCommand
  : public QUndoCommand
  {
  public:
    explicit AddRelationCommand(ModelItemSPtr  ancestor,
                                ModelItemSPtr  succesor,
                                const QString &relation,
                                ModelAdapter   *model);
    virtual void redo();
    virtual void undo();

  private:
    ModelAdapter *m_model;

    ModelItemSPtr m_ancestor;
    ModelItemSPtr m_succesor;
    const QString m_relation;
  };

  class RemoveRelationCommand
  : public QUndoCommand
  {
  public:
    explicit RemoveRelationCommand(ModelItemSPtr  ancestor,
                                ModelItemSPtr  succesor,
                                const QString &relation,
                                ModelAdapter   *model);
    virtual void redo();
    virtual void undo();

  private:
    ModelAdapter *m_model;

    ModelItemSPtr m_ancestor;
    ModelItemSPtr m_succesor;
    const QString m_relation;
  };
} // namespace EspINA

#endif // ATOMICMODELOPERATIONS_H
