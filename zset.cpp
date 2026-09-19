#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "zset.hpp"
#include "utils.hpp"
#include "buffer.hpp"

// helper struct for hashtable lookups
struct HKey {
    HNode node;
    const char* name = nullptr;
    size_t len = 0;
};

static ZNode* znode_new(const char* name, size_t len, double score) {
    ZNode* node = (ZNode*)malloc(sizeof(ZNode) + len); // struct + array, change later to string?
    avl_init(&node->tree);
    node->hmap.next = nullptr;
    node->hmap.hcode = str_hash((uint8_t *)name, len);
    node->score = score;
    node->len = len;
    node->tree.height = 1;
    node->tree.cnt = 1;
    memcpy(&node->name[0], name, len);
    return node;
}

static void znode_del(ZNode* node) {
    free(node);
}

static bool hcmp(HNode* node, HNode* key) {
    ZNode* znode = container_of(node, ZNode, hmap);
    HKey* hkey =  container_of(key, HKey, node);
    if (znode->len != hkey->len)
        return false;
    
    return 0 == memcmp(znode->name, hkey->name, znode->len);
}

ZNode* zset_lookup(ZSet* zset, const char* name, size_t len) {
    if (!zset->root)
        return nullptr;
    
    HKey key;
    key.node.hcode = str_hash((uint8_t*)name, len);
    key.name= name;
    key.len = len;
    HNode* found = hm_lookup(&zset->hmap, &key.node, &hcmp);
    return found ? container_of(found, ZNode, hmap) : nullptr;
}

static bool zless(AVLNode* lhs, AVLNode* rhs) {
    ZNode* zl = container_of(lhs, ZNode, tree);
    ZNode* zr = container_of(rhs, ZNode, tree);
    if (zl->score != zr->score)
        return zl->score < zr->score;
    
    int rv = memcmp(zl->name, zr->name, std::min(zl->len, zr->len));
    return (rv != 0) ? (rv < 0) : (zl->len < zr->len);
}

static bool zless(AVLNode* lhs, double score, const char* name, size_t len) {
    ZNode* zl = container_of(lhs, ZNode, tree);
    if (zl->score != score)
        return zl->score < score;
    
    int rv = memcmp(zl->name, name, std::min(zl->len, len));
    if (rv != 0)
        return rv < 0;
    
    return zl->len < len;
}

static void tree_insert(ZSet* zset, ZNode* node) {
    AVLNode* parent = nullptr;
    AVLNode** from = &zset->root;
    
    while (*from) {
        parent = *from;
        from = zless(&node->tree, parent) ? &parent->left : &parent->right;
    }

    *from = &node->tree;
    node->tree.parent = parent;
    zset->root = avl_fix(&node->tree);
}

static void zset_update(ZSet* zset, ZNode* node, double score) {
    zset->root = avl_del(&node->tree);
    avl_init(&node->tree);
    node->score = score;
    tree_insert(zset, node);
}

bool zset_insert(ZSet* zset, const char* name, size_t len, double score) {
    if (ZNode* node = zset_lookup(zset, name, len)) {
        zset_update(zset, node, score);
        return false;
    }
    ZNode* node = znode_new(name, len, score);
    hm_insert(&zset->hmap, &node->hmap);
    tree_insert(zset, node);
    return true;
}

void zset_delete(ZSet* zset, ZNode* node) {
    HKey key;
    key.node.hcode = node->hmap.hcode;
    key.name = node->name;
    key.len = node->len;
    HNode* found = hm_delete(&zset->hmap, &key.node, &hcmp);
    assert(found);
    zset->root = avl_del(&node->tree);
    // deallocate
    znode_del(node);
}

// -----------
//   ZQUERY
// -----------
ZNode* zset_seekge(ZSet *zset, double score, const char *name, size_t len) {
    AVLNode* found = nullptr;
    for (AVLNode* node = zset->root; node; ) {
        if (zless(node, score, name, len)) {
            node = node->right;
        } else {
            found = node;
            node = node->left;
        }
    }
    return found ? container_of(found, ZNode, tree) : nullptr;
}

AVLNode* avl_offset(AVLNode *node, int64_t offset) {
    int64_t pos = 0;
    while (offset != pos) {
        if (pos < offset && pos + avl_cnt(node->right) >= offset) {
            // target is inside left subtree
            node = node->right;
            pos += avl_cnt(node->left) + 1;
        } else if (pos > offset && pos - avl_cnt(node->left) <= offset) {
            // target is inside right subtree
            node = node->left;
            pos -= avl_cnt(node->right) + 1;
        } else {
            AVLNode* parent = node->parent;
            if (!parent)
                return nullptr;
            
            if (parent->right == node) {
                pos -= avl_cnt(node->left) + 1;
            } else {
                pos += avl_cnt(node->right) + 1; 
            }
            node = parent;
        }
    }
    return node;
}

ZNode* znode_offset(ZNode *node, int64_t offset) {
    AVLNode* tnode = node ? avl_offset(&node->tree, offset) : nullptr;
    return tnode ? container_of(tnode, ZNode, tree) : nullptr;
}

static void tree_dispose(AVLNode* node) {
    if (!node) return;
    tree_dispose(node->left);
    tree_dispose(node->right);
    znode_del(container_of(node, ZNode, tree));
}

void zset_clear(ZSet* zset) {
    hm_clear(&zset->hmap);
    tree_dispose(zset->root);
    zset->root = nullptr;
}