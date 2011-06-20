#ifndef ESPINASAVEDATAREACTION_H
#define ESPINASAVEDATAREACTION_H

class QString;

class EspinaSaveDataReaction
{
public:
  /** It is the same behaviour as pqSaveDataReaction::saveActiveData The difference
   *  is that this does not use the configurable options that the extension file could have.
   */
  static bool saveActiveData(const QString& filename);
};

#endif // ESPINASAVEDATAREACTION_H
