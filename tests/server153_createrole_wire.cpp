// CreateRole wire-format conformance test (server153 <-> PWBR v145 client).
//
// Evidence (2026-10-27): the real PWBR 1.5.3 v145 client (build 2555, 2018)
// sends CreateRole (type=84) with a 1704-byte body; the server decoded it
// with "Decode Error" because upstream commit 8aa72d3 had added a 24th
// field (Octets rank) to RoleInfo that this client does not send. The fix
// removed rank (23 fields). This test drives the SHIPPED marshal/unmarshal
// code (src/server153/cnet/rpcdata/roleinfo + share marshal) and proves:
//
//  1. A CreateRole built exactly the way the client builds it
//     (EC_LoginUIMan::NewCharacterImpl: 39 equipment slots of GRoleInventory(0),
//     16-byte UTF-16LE name, 176-byte custom_data blob) marshals to exactly
//     1704 bytes — the size of the real captured packet.
//  2. The first 64 bytes match, byte for byte, the prefix captured from the
//     real client (glinkd [decode] dump, 2026-10-27).
//  3. The server's unmarshal consumes the whole packet and every field
//     roundtrips (no Marshal::Exception, stream fully consumed).
//
// Build (32-bit, same toolchain flags as the daemons):
//   g++ -w -m32 -std=gnu++11 -fpermissive \
//     -Isrc/server153/share/io -Isrc/server153/share/common \
//     -Isrc/server153/share/perf -Isrc/server153/share/rpc \
//     -Isrc/server153/cnet/rpcdata -Isrc/server153/cnet/inl \
//     tests/server153_createrole_wire.cpp -o /tmp/createrole_wire && /tmp/createrole_wire

#include "roleinfo"
#include <cstdio>
#include <cstring>
#include <cstdlib>

using namespace GNET;
typedef GNET::Marshal::OctetsStream OctetsStream;

static int failures = 0;
#define CHECK(cond, msg) do { \
    if (!(cond)) { std::printf("FAIL: %s\n", msg); failures++; } \
    else { std::printf("ok:   %s\n", msg); } \
} while (0)

// First 64 bytes of the real client packet, captured by the glinkd [decode]
// dump on 2026-10-27 (dist/server153/runtime/logs/glinkd.log).
static const unsigned char CAPTURED_PREFIX[64] = {
    0x00,0x0f,0x42,0x40, 0x00,0xa7,0x5a,0xc2, 0xff,0xff,0xff,0xff, 0x00,0x00,0x0a,
    0x00,0x00,0x00,0x01, 0x00,0x00,0x00,0x00, 0x10,
    0x66,0x00,0x64,0x00,0x63,0x00,0x63,0x00,0x63,0x00,0x63,0x00,0x69,0x00,0x77,0x00,
    0x80,0xb0,
    0x01,0x70,0x00,0x10,0x80,0x80,0x80,0x00,0x4f,0x06,0x4f,0x06,0x32,0x81,0x81,0x00,
    0x03,0x06,0x81,0x80,0x80,0x80
};

int main() {
    // --- build the packet exactly like the client (EC_LoginUIMan.cpp) ---
    RoleInfo info;
    info.roleid = -1;
    info.gender = 0;
    info.race = 0;
    info.occupation = 10;          // class chosen in the creation UI
    info.level = 1;                // constructor default, kept explicit
    info.level2 = 0;
    // name: 8 ACHARs (UTF-16LE on Windows) -> 16 bytes, as captured
    const char name16[16] = { 'f',0,'d',0,'c',0,'c',0,'c',0,'c',0,'i',0,'w',0 };
    info.name.replace(name16, sizeof(name16));
    // custom_data: PLAYER_CUSTOMIZEDATA blob, 176 bytes as captured
    unsigned char custom[176];
    std::memset(custom, 0, sizeof(custom));
    std::memcpy(custom, CAPTURED_PREFIX + 42, 22); // captured head of the blob
    info.custom_data.replace(custom, sizeof(custom));
    // equipment: IVTRSIZE_EQUIPPACK (= SIZE_ALL_EQUIPIVTR = 39) empty slots
    for (int i = 0; i < 39; i++)
        info.equipment.add(GRoleInventory(0));

    const int userid = 1000000;             // 0x000F4240, as captured
    const unsigned int localsid = 0x00a75ac2u; // as captured
    Octets referid;                        // empty, as the client sends

    OctetsStream os;
    os << userid << localsid << info << referid;

    char msg[128];
    std::snprintf(msg, sizeof(msg), "wire size is 1704 (got %zu)", os.size());
    CHECK(os.size() == 1704, msg);

    const unsigned char *wire = (const unsigned char *)os.begin();
    CHECK(std::memcmp(wire, CAPTURED_PREFIX, 64) == 0,
          "first 64 bytes match the real client capture");

    // --- server-side unmarshal must consume it cleanly ---
    int out_userid = 0;
    unsigned int out_localsid = 0;
    RoleInfo out_info;
    Octets out_referid;
    bool threw = false;
    try {
        OctetsStream is(os);
        is >> out_userid >> out_localsid >> out_info >> out_referid;
        CHECK(is.eos(), "unmarshal consumed the whole packet");
    } catch (Marshal::Exception &) {
        threw = true;
    }
    CHECK(!threw, "unmarshal does not throw Marshal::Exception");

    CHECK(out_userid == userid, "userid roundtrips");
    CHECK(out_localsid == localsid, "localsid roundtrips");
    CHECK(out_info.roleid == -1, "roleid roundtrips (-1 = new role)");
    CHECK(out_info.gender == 0 && out_info.race == 0, "gender/race roundtrip");
    CHECK(out_info.occupation == 10, "occupation roundtrips");
    CHECK(out_info.level == 1 && out_info.level2 == 0, "level/level2 roundtrip");
    CHECK(out_info.name.size() == 16, "name is 16 bytes");
    CHECK(out_info.custom_data.size() == 176, "custom_data is 176 bytes");
    CHECK(out_info.equipment.size() == 39, "equipment has 39 slots");
    CHECK(out_referid.size() == 0, "referid is empty");

    if (failures) {
        std::printf("%d check(s) FAILED\n", failures);
        return 1;
    }
    std::printf("all checks passed\n");
    return 0;
}
