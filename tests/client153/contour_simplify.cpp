#include "CharContour.h"
#include "CharVerticesBuffer.h"
int main() {
  CCharContour contour;
  CCharVerticesBuffer* b = contour.GetCharVerticesBuffer();
  if (!b->Create(4)) return 1;
  double points[4][2] = {{0,0},{1,0},{1,1},{0,1}};
  for (int i=0;i<4;++i) if (!b->SetVerticesAt(i, points[i])) return 2;
  contour.Simplify(0.99f, 0.0f);
  return contour.GetCharVerticesBuffer()->GetVerticesCount() < 3;
}
