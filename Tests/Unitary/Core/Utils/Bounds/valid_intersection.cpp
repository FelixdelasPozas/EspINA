/*
 * Copyright (c) 2013, Jorge Peña Pastor <jpena@cesvima.upm.es>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *     names of its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY Jorge Peña Pastor <jpena@cesvima.upm.es> ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Jorge Peña Pastor <jpena@cesvima.upm.es> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "Bounds.h"

using namespace EspINA;
using namespace std;

bool Test_Bounds_Intersection(const Bounds& b1, const Bounds& b2, const Bounds& res) {

  bool pass = intersection(b1, b2) == res;

//   cout << b1 << "\t" << b2 << "\t==> " << intersection(b1, b2);
//   if (pass) {
//     cout << "  = ";
//   } else {
//     cout << " != ";
//   }
//   cout << res << endl;

  return pass;
}

int valid_intersection( int argc, char** argv )
{
  bool pass = true;

  Bounds b1{0,10,0,10,0,10};
  Bounds b2{5,15,5,15,5,15};

  if (!intersect(b1, b1)) {
    cerr << "Every bound should intersect with itself" << endl;
    pass = false;
  }

  if (!intersect(b1, b2)) {
    cerr << b1 << " intersects " << b2 << endl;
    pass = false;
  }

  if (!intersect(b2, b1)) {
    cerr << b2 << " intersects " << b1 << ". Intersection is symmetric" << endl;
    pass = false;
  }

  Bounds expectedIntersection{5,10,5,10,5,10};
  Bounds actualInteresction = intersection(b1, b2);
  if (expectedIntersection != actualInteresction) {
    cerr << "Expected bounds intersection " << expectedIntersection << " but got" << actualInteresction << " instead" << endl;
    pass = false;
  }

  if (!actualInteresction.areValid()) {
    cerr << b1 << " and " << b2 << " intersection should be a valid bounds" << endl;
    pass = false;
  }

  for (int i = 0; i < 6; ++i) 
  for (int o = 0; o < 6; o += 2)
    for (double ILB : {'[', '('})
      for (double IUB : {']', ')'})
        for (double ELB : {'[', '('})
          for (double EUB : {']', ')'}) {
            bool invalidTestOnSameBounds   = o == 2 && ((i%2 == 0 && ELB == '(' && ILB == '[') || (i%2 != 0 && EUB == ')' && IUB == ']'));
            bool invalidTestOnIntersection = o == 4;
            bool invalidTest = invalidTestOnSameBounds || invalidTestOnIntersection;
            Bounds internal{ILB,2,8,2,8,2,8,IUB};
            internal[i] += i % 2?o:-o;
            Bounds external{ELB,0,10,0,10,0,10,EUB};
            Bounds expected{internal};
            if (o > 0) {
              expected[i] = external[i];
              if (i % 2 == 0) {
                bool li;
                if (expected[i] == internal[i]) li = internal.areLowerIncluded(toAxis(i/2)) && external.areLowerIncluded(toAxis(i/2));
                else if (expected[i] < internal[i]) li = internal.areLowerIncluded(toAxis(i/2));
                else li = external.areLowerIncluded(toAxis(i/2));
                expected.setLowerInclusion(toAxis(i/2), li);
              } else {
                bool ui;
                if (expected[i] == internal[i]) ui = internal.areUpperIncluded(toAxis(i/2)) && external.areUpperIncluded(toAxis(i/2));
                else if (expected[i] > internal[i]) ui = internal.areUpperIncluded(toAxis(i/2));
                else ui = external.areUpperIncluded(toAxis(i/2));
                expected.setUpperInclusion(toAxis(i/2), ui);
              }
            }
            pass &= Test_Bounds_Intersection(internal, external, expected);
          }

  pass &= Test_Bounds_Intersection({0,2,0,2,0,2},{2,4,2,4,2,4},{'(',2,2,2,2,2,2,')'});
  pass &= Test_Bounds_Intersection({2,4,2,4,2,4},{0,2,0,2,0,2},{'(',2,2,2,2,2,2,')'});

  return !pass;
}