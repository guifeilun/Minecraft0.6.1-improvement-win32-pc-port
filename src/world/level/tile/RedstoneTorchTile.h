#ifndef NET_MINECRAFT_WORLD_LEVEL_TILE__RedstoneTorchTile_H__
#define NET_MINECRAFT_WORLD_LEVEL_TILE__RedstoneTorchTile_H__

#include "TorchTile.h"

// Redstone torch — same orientation logic as TorchTile (auxValue 1..5 is
// the mount face, 5 = floor) but acts as a redstone NOT-gate:
//   - When lit (id 76): emits a signal in all directions EXCEPT into its
//     support block, and strongly powers the block opposite its support
//     (i.e. above an upright torch).
//   - Turns off when its support block is directly powered by something
//     else, and turns back on when that power goes away. Toggle is
//     scheduled 2 ticks out to break instant feedback loops; vanilla
//     burnout is not implemented so a deliberate fast oscillator will
//     just toggle at the tick rate without ever fusing.
//
// Two tile IDs registered: notGate_on (76, lit) and notGate_off (75,
// unlit). Both share this class; the constructor's `_lit` parameter
// drives signal output and toggle direction.
class RedstoneTorchTile: public TorchTile
{
    typedef TorchTile super;
public:
    RedstoneTorchTile(int id, int tex, bool lit)
    :   super(id, tex),
        _lit(lit)
    {
    }

    // Inherit TorchTile placement, orientation, support-loss handling,
    // and clip() shape. We override only the redstone bits.

    void animateTick(Level*, int, int, int, Random*) {
        // Smoke/flame particles look out of place on the redstone torch;
        // the renderer already draws a dim red glow. Leave the unlit
        // variant entirely silent.
    }

    bool isSignalSource() { return _lit; }

    // Run TorchTile::onPlace first (it picks the mount direction) and
    // then queue a state-check so a freshly placed torch on an already
    // powered block flips immediately to its correct variant.
    void onPlace(Level* level, int x, int y, int z) {
        super::onPlace(level, x, y, z);
        level->addToTickNextTick(x, y, z, this->id, 2);
        // setTileAndData fired tileUpdated for our 6 direct neighbours
        // already — but anything wired off the block we power (a door
        // on top of the dirt that's on top of us, say) is 2 cells away
        // and needs an explicit poke.
        notifyAffected(level, x, y, z);
    }

    // Weak power: any direction the torch faces outward — i.e. anything
    // except into its support block. dir is "direction from receiver to
    // us"; supportDirFromReceiver() returns that opposite mapping.
    bool getSignal(LevelSource* level, int x, int y, int z, int dir) {
        if (!_lit) return false;
        int mount = level->getData(x, y, z) & 7;
        return dir != supportDirFromReceiver(mount);
    }

    // Strong power: the block directly above us is always strongly
    // powered, regardless of mount (vanilla rule — a wall torch still
    // pushes a hard signal straight up to whatever's sitting on the
    // block it's attached to). For wall-mounted torches we also
    // strongly power the block opposite the mount face — the "front"
    // of the torch.
    //
    // dir is "direction from receiver to us": dir==0 means the
    // receiver is above us looking down.
    bool getDirectSignal(Level* level, int x, int y, int z, int dir) {
        if (!_lit) return false;
        if (dir == 0) return true;
        int mount = level->getData(x, y, z) & 7;
        return dir == oppositeOfSupportFromReceiver(mount);
    }

    // We need to react when the world around our support changes. The
    // base class' neighborChanged checks support survival; we also queue
    // a delayed re-evaluation if our lit state contradicts whether our
    // support is now strongly powered.
    void neighborChanged(Level* level, int x, int y, int z, int type) {
        super::neighborChanged(level, x, y, z, type);

        // If the torch tile is still present (support-survival check
        // didn't blow it away), schedule a state-check tick.
        int t = level->getTile(x, y, z);
        if (t != this->id) return;

        if (shouldToggle(level, x, y, z)) {
            level->addToTickNextTick(x, y, z, this->id, 2);
        }
    }

    // Random/scheduled tick: re-evaluate, and if state still wants to
    // flip, swap to the other variant.
    // Both variants drop a "redstone torch" item (the on form, id 76)
    // since that's the only ItemInstance the game recognises for it.
    int getResource(int /*data*/, Random* /*random*/) {
        return Tile::notGate_on ? Tile::notGate_on->id : this->id;
    }

    void tick(Level* level, int x, int y, int z, Random* random) {
        // Skip super::tick — TorchTile's tick re-derives the mount
        // direction via onPlace if data==0, which would clobber ours
        // mid-flight. Keep just the support-survival side-effect.
        // (TorchTile::onPlace already ran when we were placed.)
        (void)random;

        if (!shouldToggle(level, x, y, z)) return;

        int data = level->getData(x, y, z);
        // Swap to the opposite variant, preserving mount data.
        Tile* swapTo = _lit ? Tile::notGate_off : Tile::notGate_on;
        if (!swapTo) return;
        level->setTileAndData(x, y, z, swapTo->id, data);
        // Same reason as onPlace: setTileAndData notifies direct
        // neighbours only. The "block we power then the block beyond
        // that" chain (e.g. torch → dirt → door) needs the second
        // step explicitly.
        notifyAffected(level, x, y, z);
    }

private:
    bool _lit;

    // Wake the block we strongly power, plus its neighbours, so any
    // tile two cells out (a door above the dirt above us, a wire next
    // to a stone we're powering through, etc.) gets a neighborChanged
    // and can react to the new signal state. The torch's own direct
    // neighbours are already poked by setTileAndData/tileUpdated; this
    // fills in the one-extra-hop case.
    void notifyAffected(Level* level, int x, int y, int z) {
        int mount = level->getData(x, y, z) & 7;
        int fx = x, fy = y, fz = z;
        switch (mount) {
            case 1: fx += 1; break;  // mount -X → strong-powered block at +X
            case 2: fx -= 1; break;
            case 3: fz += 1; break;
            case 4: fz -= 1; break;
            case 5: default: fy += 1; break;  // floor mount → block above
        }
        level->updateNeighborsAt(fx, fy, fz, this->id);
        // The block directly above is also strongly powered (the dir==0
        // rule in getDirectSignal) regardless of mount. For floor-mount
        // it's the same cell we just poked.
        if (mount != 5) {
            level->updateNeighborsAt(x, y + 1, z, this->id);
        }
    }

    // True if our `_lit` state disagrees with the power state of our
    // support block — i.e. the NOT-gate output disagrees with the input.
    bool shouldToggle(Level* level, int x, int y, int z) {
        bool powered = supportPowered(level, x, y, z);
        // Lit when input is unpowered, unlit when input is powered.
        bool wantLit = !powered;
        return wantLit != _lit;
    }

    bool supportPowered(Level* level, int x, int y, int z) {
        int mount = level->getData(x, y, z) & 7;
        int sx = x, sy = y, sz = z;
        switch (mount) {
            case 1: sx -= 1; break;  // mounted on -X face → support at -X
            case 2: sx += 1; break;
            case 3: sz -= 1; break;
            case 4: sz += 1; break;
            case 5: default: sy -= 1; break;  // floor mount → support below
        }
        // Both strong (a wire on top of the support block, or a lever
        // attached to it) and weak power (an adjacent wire at the same
        // level, or another redstone torch attached to a neighbouring
        // face) count. This is more permissive than vanilla — vanilla
        // only flips on strong power — but matches the simpler mental
        // model the player has: "anything red-ish next to my block
        // turns the torch off". The mount-direction check in
        // getSignal()/getDirectSignal() prevents us from including
        // ourselves in either query.
        return level->hasDirectSignal(sx, sy, sz)
            || level->hasNeighborSignal(sx, sy, sz);
    }

    // Maps mount face → dir-from-receiver value for "the support side".
    // mount=5 (floor): support at -Y of torch. Receiver at -Y looks UP
    // toward torch → dir=1.
    static int supportDirFromReceiver(int mount) {
        switch (mount) {
            case 1: return 5;  // support at -X; receiver there looks +X to us
            case 2: return 4;
            case 3: return 3;
            case 4: return 2;
            case 5: default: return 1;  // floor mount: support below, looks up
        }
    }

    // Opposite of supportDirFromReceiver — the "front" of the torch.
    static int oppositeOfSupportFromReceiver(int mount) {
        switch (mount) {
            case 1: return 4;  // front = +X; receiver at +X looks -X (4) to us
            case 2: return 5;
            case 3: return 2;
            case 4: return 3;
            case 5: default: return 0;  // floor mount: front = up; receiver above looks down (0)
        }
    }
};

#endif
