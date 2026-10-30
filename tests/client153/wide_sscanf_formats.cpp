// U04-B regression coverage: the wide-char scan surface the audited call
// sites use, driving the REAL shipped parser (platform/common/widestr.cpp)
// through the three aliases the game code actually calls:
//   - the swscanf inline alias (widestr.h — AWScriptFile.h, afsdk copy,
//     AUIManager, AUICommon, DlgProduce, globaldataman, EC_PetWords),
//   - a_sscanf (AChar.h under UNICODE — DlgWikiFeature, DlgWikiSkill,
//     EC_GameSession, EC_UIHelper, ElementClient),
//   - wide_sscanf directly (the seam entry point).
//
// Formats and inputs come from the real call sites:
//   AUIManager.cpp:455      %X       font outline color (hex ARGB)
//   globaldataman.cpp:470   %f,%f,%f transport target position
//   AUICommon.cpp:1702      %d:%d    emotion set:index
//   DlgProduce.cpp:1204     %d/%d    produce has/need counters
//   DlgWikiFeature.cpp:52   %d-%d    wiki range min-max (== 2 checked)
//   EC_UIHelper.cpp:436     %f%f%f%d item position + id (== 4 checked)
//   EC_GameSession.cpp:5233 %x       emotion id (hex)
//   ElementClient.cpp:1185  %d:%s    server port:ip command line
//   EC_PetWords.cpp:152,205 %f / %d  pet script values (!= 1 checked)
//
// MSVC scanf semantics asserted here (the original build's CRT):
//   - an optional sign is accepted by every integer conversion
//     (d, i, u, x, X) per the C standard's fscanf contract;
//   - an integer conversion with no digits is a matching failure: the
//     return does not count it and the output argument is untouched;
//   - %f accepts exponent notation ("1.5e2" == 150.0);
//   - %i auto-detects the base (0x hex, leading 0 octal, else decimal).
#define UNICODE
#include "ABaseDef.h"
#include "AChar.h"
#include <cstdio>

namespace {

int failures = 0;

void check(const char *label, bool ok) {
  if (!ok) {
    ++failures;
    std::printf("FAIL %s\n", label);
  } else {
    std::printf("ok %s\n", label);
  }
}

// The swscanf alias the game headers see (widestr.h inline).
int scan_alias(const wchar_t *text, const wchar_t *format, ...) {
  va_list args;
  va_start(args, format);
  const int converted = pw153::platform::wide_vsscanf(text, format, args);
  va_end(args);
  return converted;
}

}  // namespace

int main() {
  // ---- %X: the AUIManager font outline color (AUIManager.cpp:455).
  {
    int color = 0;
    check("hex %X uppercase digits", swscanf(L"FF00FF00", L"%X", &color) == 1 &&
                                        color == static_cast<int>(0xFF00FF00u));
    color = 0;
    check("hex %X lowercase digits", swscanf(L"ff00ff00", L"%X", &color) == 1 &&
                                        color == static_cast<int>(0xFF00FF00u));
    color = 0;
    check("hex %X mixed digits", swscanf(L"AbCd1234", L"%X", &color) == 1 &&
                                     color == static_cast<int>(0xABCD1234u));
  }
  // ---- %x: the EC_GameSession emotion id (EC_GameSession.cpp:5233).
  {
    int emotion = 0;
    check("hex %x value", swscanf(L"1a2b3c", L"%x", &emotion) == 1 && emotion == 0x1a2b3c);
  }
  // ---- %f: the AWScriptFile/EC_PetWords/AWString float token.
  {
    float value = -1.0f;
    check("float integer token", swscanf(L"160", L"%f", &value) == 1 && value == 160.0f);
    check("float negative", swscanf(L"-200", L"%f", &value) == 1 && value == -200.0f);
    check("float decimal", swscanf(L"0.25", L"%f", &value) == 1 && value == 0.25f);
    check("float leading dot", swscanf(L".5", L"%f", &value) == 1 && value == 0.5f);
    check("float exponent", swscanf(L"1.5e2", L"%f", &value) == 1 && value == 150.0f);
    check("float negative exponent", swscanf(L"-2.5E-1", L"%f", &value) == 1 && value == -0.25f);
    check("float trailing garbage stops at token", swscanf(L"160abc", L"%f", &value) == 1 &&
                                                        value == 160.0f);
  }
  // ---- %f,%f,%f: the globaldataman transport target (globaldataman.cpp:470).
  {
    float x = 0, y = 0, z = 0;
    check("float list commas", swscanf(L"123.5, -200, 0.25", L"%f,%f,%f", &x, &y, &z) == 3 &&
                                  x == 123.5f && y == -200.0f && z == 0.25f);
  }
  // ---- %d:%d: the AUICommon emotion pair (AUICommon.cpp:1702).
  {
    int set = 0, index = 0;
    check("int pair colon", swscanf(L"12:34", L"%d:%d", &set, &index) == 2 && set == 12 &&
                               index == 34);
  }
  // ---- %d/%d: the DlgProduce has/need counters (DlgProduce.cpp:1204).
  {
    int has = 0, need = 0;
    check("int pair slash", swscanf(L"3/7", L"%d/%d", &has, &need) == 2 && has == 3 &&
                               need == 7);
  }
  // ---- %d-%d: the DlgWikiFeature/DlgWikiSkill range (checked == 2).
  {
    int min = 0, max = 0;
    check("int pair dash via a_sscanf",
          a_sscanf(L"10-50", _AL("%d-%d"), &min, &max) == 2 && min == 10 && max == 50);
  }
  // ---- %f%f%f%d: the EC_UIHelper item info (checked == 4).
  {
    float px = 0, py = 0, pz = 0;
    int id = 0;
    check("position plus id via a_sscanf",
          a_sscanf(L"-1.5 2.0 0.5 42", _AL("%f%f%f%d"), &px, &py, &pz, &id) == 4 &&
              px == -1.5f && py == 2.0f && pz == 0.5f && id == 42);
  }
  // ---- %d:%s: the ElementClient port:ip command (ElementClient.cpp:1185).
  {
    int port = 0;
    wchar_t ip[64] = L"sentinel";
    check("port then ip string", swscanf(L"29000:192.168.0.1", L"%d:%s", &port, ip) == 2 &&
                                   port == 29000 && wcscmp(ip, L"192.168.0.1") == 0);
  }
  // ---- Optional sign on every integer conversion (C standard fscanf
  // contract; MSVC accepts it). EC_GameSession feeds %x; DlgProduce and
  // the wiki dialogs feed %d.
  {
    int value = 0;
    check("signed decimal", swscanf(L"-42", L"%d", &value) == 1 && value == -42);
    check("signed hex %x", swscanf(L"-1a", L"%x", &value) == 1 && value == -0x1a);
    check("signed hex %X", swscanf(L"-1A", L"%X", &value) == 1 && value == -0x1a);
    check("plus-signed decimal", swscanf(L"+42", L"%d", &value) == 1 && value == 42);
  }
  // ---- %i base auto-detection (the spec set widestr.h claims).
  {
    int value = 0;
    check("%i decimal", swscanf(L"42", L"%i", &value) == 1 && value == 42);
    check("%i hex prefix", swscanf(L"0x1a", L"%i", &value) == 1 && value == 0x1a);
    check("%i octal prefix", swscanf(L"017", L"%i", &value) == 1 && value == 15);
  }
  // ---- Invalid inputs: MSVC matching failure — the conversion is not
  // counted and the output argument keeps its previous value. The game
  // branches on this (EC_PetWords.cpp:152,205 check != 1; the wiki
  // dialogs check == 2; EC_UIHelper checks == 4).
  {
    int value = 77;
    check("garbage int not converted", swscanf(L"abc", L"%d", &value) == 0 && value == 77);
    check("empty int not converted", swscanf(L"", L"%d", &value) == 0 && value == 77);
    check("sign-only int not converted", swscanf(L"-", L"%d", &value) == 0 && value == 77);
    check("garbage hex not converted", swscanf(L"zz", L"%x", &value) == 0 && value == 77);
    float fvalue = 88.0f;
    check("garbage float not converted", swscanf(L"abc", L"%f", &fvalue) == 0 &&
                                             fvalue == 88.0f);
    check("empty float not converted", swscanf(L"", L"%f", &fvalue) == 0 && fvalue == 88.0f);
    int has = 55, need = 66;
    check("garbage pair not converted",
          swscanf(L"abc/def", L"%d/%d", &has, &need) == 0 && has == 55 && need == 66);
    check("half pair converts only the first",
          swscanf(L"10/", L"%d-%d", &has, &need) == 1 && has == 10 && need == 66);
    int set = 33, index = 44;
    check("colon pair stops at garbage",
          swscanf(L"12:x", L"%d:%d", &set, &index) == 1 && set == 12 && index == 44);
  }
  // ---- The direct seam entry and the swscanf alias agree (the alias is
  // what every game header calls; both must be the same parser).
  {
    int direct = 0, aliased = 0;
    const int direct_converted = pw153::platform::wide_sscanf(L"7", L"%d", &direct);
    const int aliased_converted = scan_alias(L"7", L"%d", &aliased);
    check("seam and alias agree", direct_converted == 1 && aliased_converted == 1 &&
                                     direct == 7 && aliased == 7);
  }
  if (failures) {
    std::printf("%d failures\n", failures);
    return 1;
  }
  std::printf("all U04-B wide scan format checks passed\n");
  return 0;
}
