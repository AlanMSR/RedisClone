#pragma once

#include <stddef.h>
#include <stdint.h>

const size_t k_max_load_factor = 8;
const size_t k_rehashing_work = 128;

// intrusive list node
struct HNode {
    HNode* next = nullptr;
    uint64_t hcode = 0; // hash value
};

struct HTab {
    HNode** tab = nullptr; // array of slots
    size_t mask = 0; // power of two arrays size
    size_t size = 0; // number of keys
};

struct HMap {
    HTab newer;
    HTab older;
    size_t migrate_pos = 0;
};

HNode *hm_lookup(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
void   hm_insert(HMap *hmap, HNode *node);
HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *));
void   hm_clear(HMap *hmap);
size_t hm_size(HMap *hmap);