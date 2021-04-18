#ifndef ESPINA_TEST_SUPPORT_H
#define ESPINA_TEST_SUPPORT_H

#include <VolumeBounds.h>

namespace ESPINA
{
  //-----------------------------------------------------------------------------
  inline bool operator==(const VolumeBounds &lhs, const Bounds &rhs)
  {
    for (int i = 0; i < 6; ++i) {
      if (!areEqual(lhs[i], rhs[i])) return false;
    }

    for (Axis dir : {Axis::X, Axis::Y, Axis::Z}) {
      if (!rhs.areLowerIncluded(dir) || rhs.areUpperIncluded(dir))
	return false;
    }

    return true;
  }

  //-----------------------------------------------------------------------------
  inline bool operator!=(const VolumeBounds &lhs, const Bounds &rhs)
  {
    return !(lhs == rhs);
  }
}

#endif // ESPINA_TEST_SUPPORT_H