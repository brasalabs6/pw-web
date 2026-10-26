#include "A3DTrackData.h"
#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <stdexcept>
#define CHECK(x)                                                               \
  do {                                                                         \
    if (!(x))                                                                  \
      throw std::runtime_error(#x);                                            \
  } while (false)
int main(int argc, char **argv) {
  if (argc == 2) {
    AFile input;
    CHECK(input.Open(argv[1], AFILE_OPENEXIST | AFILE_BINARY));
    std::array<std::int32_t, 24> header{};
    DWORD bytes = 0;
    CHECK(input.Read(header.data(), 96, &bytes) && bytes == 96);
    CHECK(header[0] == 0x41534b45 && header[1] == 5);
    CHECK(header[2] > 0 && header[2] <= 4096);
    CHECK(header[3] > 0 && header[3] <= 4096);
    for (int i = 0; i < header[2]; ++i) {
      AString name;
      CHECK(input.ReadString(name));
      std::array<unsigned char, 144> bone{};
      CHECK(input.Read(bone.data(), bone.size(), &bytes) &&
            bytes == bone.size());
      std::int32_t children;
      std::memcpy(&children, bone.data() + 12, 4);
      CHECK(children >= 0 && children <= header[2]);
      CHECK(input.Seek(children * 4, AFILE_SEEK_CUR));
    }
    for (int i = 0; i < header[3]; ++i) {
      DWORD type;
      CHECK(input.Read(&type, 4, &bytes) && bytes == 4);
      CHECK(type == 15);
    }
    int animated = 0;
    std::size_t samples = 0;
    for (int i = 0; i < header[3]; ++i) {
      AString name;
      CHECK(input.ReadString(name));
      std::array<int, 3> joint;
      CHECK(input.Read(joint.data(), 12, &bytes) && bytes == 12);
      A3DTRACKSAVEDATA data;
      CHECK(a3d_LoadTrackInfo(&input, &data));
      A3DVector3Track position;
      CHECK(position.Create(data.iNumKey, data.iFrameRate, data.iNumSegment));
      CHECK(position.Load(&input));
      CHECK(a3d_LoadTrackInfo(&input, &data));
      A3DQuaternionTrack rotation;
      CHECK(rotation.Create(data.iNumKey, data.iFrameRate, data.iNumSegment));
      CHECK(rotation.Load(&input));
      rotation.SetInterType(TRACKDATA_INTERTYPE_SLERP);
      const auto time =
          position.GetTrackStartTime() +
          (position.GetTrackEndTime() - position.GetTrackStartTime()) / 2;
      const auto v = position.GetKeyValue(time);
      const auto q = rotation.GetKeyValue(time);
      CHECK(std::isfinite(v.x) && std::isfinite(q.w));
      for (int t = position.GetTrackStartTime();
           t <= position.GetTrackEndTime(); t += 37) {
        auto point = position.GetKeyValue(t);
        auto orientation = rotation.GetKeyValue(t);
        CHECK(std::isfinite(point.x) && std::isfinite(point.y) &&
              std::isfinite(point.z));
        CHECK(std::isfinite(orientation.x) && std::isfinite(orientation.y) &&
              std::isfinite(orientation.z) && std::isfinite(orientation.w));
        ++samples;
      }
      bool moves = false;
      const auto first = position.GetKeyValueByIndex(0);
      for (int k = 1; k < position.GetNumKeys(); ++k) {
        const auto value = position.GetKeyValueByIndex(k);
        moves |= value.x != first.x || value.y != first.y || value.z != first.z;
      }
      const auto firstRotation = rotation.GetKeyValueByIndex(0);
      for (int k = 1; k < rotation.GetNumKeys(); ++k)
        moves |= rotation.GetKeyValueByIndex(k) != firstRotation;
      if (moves)
        ++animated;
    }
    CHECK(animated > 0);
    std::cout << "PASS REAL BON v5 joints=" << header[3]
              << " tracks=" << header[3] * 2 << " animatedJoints=" << animated
              << " samples=" << samples << " through original A3DTrackData\n";
    return 0;
  }

  A3DFloatTrack scalar;
  CHECK(scalar.Create(2, 10, 1));
  scalar.SetSegmentTime(0, 0, 100);
  scalar.SetSegmentKeyFrame(0, 0, 1);
  scalar.SetKeyValue(0, 0);
  scalar.SetKeyValue(1, 2);
  CHECK(std::abs(scalar.GetKeyValue(50) - 1) < .0001f);
  A3DVector3Track track;
  CHECK(track.Create(2, 10, 1));
  track.SetSegmentTime(0, 0, 100);
  track.SetSegmentKeyFrame(0, 0, 1);
  track.SetKeyValue(0, A3DVECTOR3(0, 0, 0));
  track.SetKeyValue(1, A3DVECTOR3(2, 4, 6));
  auto middle = track.GetKeyValue(50);
  CHECK(std::abs(middle.x - 1) < .0001f);
  CHECK(std::abs(middle.y - 2) < .0001f);
  CHECK(track.GetKeyValue(-5).x == 0);
  CHECK(track.GetKeyValue(500).x == 2);
  A3DQuaternionTrack rotation;
  CHECK(rotation.Create(2, 10, 1));
  rotation.SetSegmentTime(0, 0, 100);
  rotation.SetSegmentKeyFrame(0, 0, 1);
  rotation.SetKeyValue(0, A3DQUATERNION(1, 0, 0, 0));
  rotation.SetKeyValue(1, A3DQUATERNION(A3DVECTOR3(0, 0, 1), 1.57079632679f));
  rotation.SetInterType(TRACKDATA_INTERTYPE_SLERP);
  A3DMATRIX4 matrix;
  rotation.GetKeyValue(50).ConvertToMatrix(matrix);
  auto v = A3DVECTOR3(1, 0, 0) * matrix;
  CHECK(std::abs(v.x - .70710678f) < .0001f);
  AFile file;
  CHECK(file.Open("track-test.bin", AFILE_CREATENEW | AFILE_BINARY));
  CHECK(track.Save(&file));
  CHECK(file.Close());
  CHECK(file.Open("track-test.bin", AFILE_OPENEXIST));
  A3DTRACKSAVEDATA info;
  CHECK(a3d_LoadTrackInfo(&file, &info));
  A3DVector3Track loaded;
  CHECK(loaded.Create(info.iNumKey, info.iFrameRate, info.iNumSegment));
  CHECK(loaded.Load(&file));
  CHECK(loaded.GetKeyValue(50).x == middle.x);
  CHECK(file.Close());
  FILE *raw = std::fopen("track-test.bin", "wb");
  CHECK(raw);
  std::fwrite("MOXB", 1, 4, raw);
  std::fclose(raw);
  CHECK(file.Open("track-test.bin", AFILE_OPENEXIST));
  CHECK(!loaded.Load(&file));
  CHECK(!loaded.FakeLoad(&file));
  CHECK(loaded.GetKeyValue(50).x == middle.x);
  CHECK(file.Close());
  A3DVector3Track invalid;
  CHECK(!invalid.Create(0, 10, 1));
  CHECK(!invalid.Create(2, 0, 1));
  std::remove("track-test.bin");
  std::cout << "PASS original vector/quaternion track sampling, AFile "
               "roundtrip, truncation atomicity and invalid counts\n";
}
