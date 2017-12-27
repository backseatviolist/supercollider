#include "BufferUGens.hpp"

#include "PlayBuf.hpp"
#include "BufRd.hpp"

PluginLoad(BufferUGens) {
    ft = inTable;
    DefineSimpleCantAliasUnit(PlayBuf);
    DefineSimpleUnit(BufRd);
}
