/* (hookshot) The neighbouring area OoT gets ready, held while Link hesitates between two.

   OoT's OptimizeChangeBank manager (its update f_832b9880, once a frame) keeps up to 16 neighbouring areas ("units", 296
   bytes each from the manager's +1032: byte +0 the unit is in use, byte +1 it was asked for, +4 to +12 its centre, +292 its
   radius). Among the units whose radius holds Link it picks one (the one he heads for, else the nearest: w21, -1 for
   none), asks for it, and lets go of every other unit it had asked for. Where two radii overlap (the Market's plaza: the
   field's and the castle's) the pick flips back and forth as Link turns, and each flip throws away an area half made
   ready and starts the other one again: background threads busy for nothing, and frames late on CPUs with few cores.

   Here only a flip BACK is held: a pick of the area the game let go of less than 10 s ago replaces the one asked for once it
   has stayed the game's pick for HOS_OCB_HOLD_MS (1500 by default) without a break. Any other pick (a new area, as after a
   teleport or a walk into another place, none, or the first) is the game's own at once, so no preload starts later than the
   game would start it, except a return to the area just left. Areas are told apart by their centre and radius, not their unit's number
   (the units are numbered again in every place). HOS_OCB_HOLD_MS=0: the game's own pick every frame. HOS_OCB_LOG=1: each
   decision in the log. nx2/oot_patches.py (patch_ocb) puts the call at the head of the code that acts on the pick. */
#include "hos.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { OCB_UNITS = 16, OCB_UNIT0 = 1032, OCB_STRIDE = 296 };

static inline uint32_t ocb_byte(uint64_t a) { return *(const uint8_t *)GP(a); }

/* An area told apart by its unit's centre (+4, +8, +12: x, y, z) and radius (+292), as the update reads them to pick: the
   same area keeps them, another place's units have others (0 for an empty unit). text: the centre, for the log */
static uint64_t ocb_name(uint64_t mgr, int unit, char *text, size_t text_n) {
    if (text_n) text[0] = 0;
    if (unit < 0 || unit >= OCB_UNITS) return 0;
    const uint64_t u = mgr + OCB_UNIT0 + (uint64_t)unit * OCB_STRIDE;
    float f[4];
    memcpy(f, GP(u + 4), 12);
    memcpy(f + 3, GP(u + 292), 4);
    uint64_t h = 1469598103934665603ull;
    const unsigned char *b = (const unsigned char *)f;
    for (unsigned i = 0; i < sizeof f; i++) h = (h ^ b[i]) * 1099511628211ull;
    if (text_n) snprintf(text, text_n, "%.0f %.0f %.0f r%.0f", f[0], f[1], f[2], f[3]);
    return f[0] == 0 && f[1] == 0 && f[2] == 0 && f[3] == 0 ? 0 : h;
}

static int64_t ocb_hold_ns(void) {
    static int64_t v = -1;
    if (v < 0) {
        const char *e = getenv("HOS_OCB_HOLD_MS");
        const long ms = e && *e ? atol(e) : 1500;
        v = ms > 0 ? (int64_t)ms * 1000000 : 0;
        hos_log("ocb: a return to the neighbouring area just let go replaces the one asked for after %ld ms as the game's pick "
                "(HOS_OCB_HOLD_MS; 0: the game's own pick)", ms > 0 ? ms : 0);
    }
    return v;
}

uint64_t hos_oot_ocb_hold(Ctx *c, uint64_t mgr, uint64_t pick) {
    (void)c;
    static uint64_t last_mgr, held_name, left_name, cand_name;
    static int held = -1;
    static int64_t left_at, cand_since, last_call;
    static int log_on = -1;
    if (log_on < 0) log_on = getenv("HOS_OCB_LOG") && atoi(getenv("HOS_OCB_LOG"));
    const int64_t hold = ocb_hold_ns(), now = hos_now_ns();
    const int game = (int)(int32_t)(uint32_t)pick;
    if (mgr != last_mgr || now - last_call > 1000000000LL) held = -1, held_name = left_name = cand_name = 0;
    last_mgr = mgr, last_call = now;
    if (!hold || game == held) {
        if (game == held) cand_name = 0;
        else if (log_on) hos_log("ocb: pick %d, asked %d -> taken (no hold)", game, held);
        held = game, held_name = game >= 0 ? ocb_name(mgr, game, NULL, 0) : 0;
        return pick;
    }
    char text[48];
    const uint64_t name = game >= 0 ? ocb_name(mgr, game, text, sizeof text) : 0;
    const int held_in_use = held >= 0 && held < OCB_UNITS && ocb_byte(mgr + OCB_UNIT0 + (uint64_t)held * OCB_STRIDE) == 1;
    /* (a flip back: the game's pick is the area it let go of less than 10 s ago, while the one asked for is still in use) */
    const int back = name && name == left_name && now - left_at < 10000000000LL && held_in_use && held_name && held_name != name;
    int out = game;
    if (back) {
        if (name != cand_name) cand_name = name, cand_since = now;
        if (now - cand_since < hold) out = held;
    }
    if (out == held) {
        if (log_on) {
            static int64_t told_at;
            if (now - told_at > 250000000LL) told_at = now, hos_log("ocb: pick %d (%s) back, asked %d: held", game, text, held);
        }
        return (uint64_t)(uint32_t)held;
    }
    if (log_on) hos_log("ocb: pick %d (%s), asked %d -> taken%s", game, game >= 0 ? text : "none", held, back ? " (after the hold)" : "");
    if (held >= 0 && held_name) left_name = held_name, left_at = now;
    held = out, held_name = name, cand_name = 0;
    return (uint64_t)(uint32_t)out;
}
