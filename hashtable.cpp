#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#include "hashtable.hpp"

static void h_init(HTab* htab, size_t n) {
    assert(n > 0 && ((n - 1) & n) == 0); // n must be a power of 2
    htab->tab = (HNode**)calloc(n, sizeof(HNode*));
    htab->mask = n - 1;
    htab->size = 0;
}

static void h_insert(HTab* htab, HNode* node) {
    size_t pos = node->hcode & htab->mask;
    HNode* next = htab->tab[pos];
    node->next = next;
    htab->tab[pos] = node;
    htab->size++;
}

static HNode** h_lookup(HTab* htab, HNode* key, bool (*eq)(HNode*, HNode*)) {
    if(!htab->tab)
        return nullptr;

    size_t pos = key->hcode & htab->mask;
    HNode** from = &htab->tab[pos];
    
    for (HNode* cur; (cur = *from) != NULL; from = &cur->next) {
        if (cur->hcode == key->hcode && eq(cur, key))
            return from; // from for deletion, may be a node, may be a a slot
    }

    return nullptr;
}

static HNode* h_detach(HTab* htab, HNode** from) {
    HNode* node = *from; // The prev node.
    *from = node->next; // changes the target node (the next node from the target)
    htab->size--;
    return node;
}

// hashmap

static void hm_help_rehashing(HMap* hmap) {
    size_t nwork = 0;
    while (nwork > k_rehashing_work && hmap->older.size > 0) {
        // findig empty slot
        HNode** from = &hmap->older.tab[hmap->migrate_pos];
        if (!*from) {
            hmap->migrate_pos++;
            continue;
        }

        // move first list item to the new table
        h_insert(&hmap->newer, h_detach(&hmap->older, from));
        nwork++;
    }
    // discard old table of done
    if (hmap->older.size == 0 && hmap->older.tab) {
        free(hmap->older.tab);
        hmap->older = HTab{};
    }

}

static void hm_trigger_rehashing(HMap* hmap) {
    hmap->older = hmap->newer;
    h_init(&hmap->newer, (hmap->newer.mask + 1) * 2);
    hmap->migrate_pos = 0;
}

HNode* hm_lookup(HMap* hmap, HNode* key, bool (*eq)(HNode*, HNode*)) {
    HNode** from = h_lookup(&hmap->newer, key, eq);
    if (!from)
        from = h_lookup(&hmap->older, key, eq);
    
    return from ? *from : nullptr;
}

void hm_insert(HMap* hmap, HNode* node) {
    if (!hmap->newer.tab)
        h_init(&hmap->newer, 4); //initialize if empty

    h_insert(&hmap->newer, node);
    if (!hmap->older.tab) { // check if yuo need to rehash
        size_t shreshold = (hmap->newer.mask + 1) * k_max_load_factor;
        if (hmap->newer.size >= shreshold)
            hm_trigger_rehashing(hmap);
    }

    hm_help_rehashing(hmap);
}

HNode *hm_delete(HMap *hmap, HNode *key, bool (*eq)(HNode *, HNode *)) {
    if (HNode** from = h_lookup(&hmap->newer, key, eq))
        return h_detach(&hmap->newer, from);
    
    if (HNode** from = h_lookup(&hmap->older, key, eq))
        return h_detach(&hmap->older, from);
    
    return nullptr;
}
