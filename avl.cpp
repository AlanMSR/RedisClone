#include <stddef.h>
#include <stdint.h>
#include <cassert>
#include <algorithm>

#include "avl.hpp"

static void avl_update(AVLNode* node) {
    node->height = 1 + std::max(avl_height(node->left), avl_height(node->right));
    node->cnt = 1 + avl_cnt(node->left) + avl_cnt(node->right);
}

static uint8_t avl_get_height_diff(AVLNode* node) {
    uintptr_t ptr = (uintptr_t)node->parent;
    return ptr & 0b11;
}

static AVLNode* avl_get_parent(AVLNode* node) {
    uintptr_t ptr = (uintptr_t)node->parent;
    return (AVLNode*)(ptr & (~0b11)); // clear LSB
}

static AVLNode* rot_left(AVLNode* node) {
    AVLNode* parent = node->parent;
    AVLNode* new_node = node->right;
    AVLNode* inner = new_node->left;
    
    node->right = inner;
    if (inner)
        inner->parent = node;

    new_node->parent = parent;
    new_node->left = node;
    node->parent = new_node;

    avl_update(node);
    avl_update(new_node);
    return new_node;
}

static AVLNode* rot_right(AVLNode* node) {
    AVLNode* parent = node->parent;
    AVLNode* new_node = node->left;
    AVLNode* inner = new_node->right;
    
    node->left = inner;
    if (inner)
        inner->parent = node;

    new_node->parent = parent;
    new_node->right = node;
    node->parent = new_node;

    avl_update(node);
    avl_update(new_node);
    return new_node;
}

static AVLNode* avl_fix_left(AVLNode* node) {
    if (avl_height(node->left->left) < avl_height(node->right->right))
        node->left = rot_left(node->left);
    
    return rot_right(node);
}

static AVLNode* avl_fix_right(AVLNode* node) {
    if (avl_height(node->right->right) < avl_height(node->right->left))
        node->right = rot_right(node->right);
    
    return rot_left(node);
}

AVLNode* avl_fix(AVLNode* node) {
    while (true) {
        AVLNode** from = &node; // subtree saved
        AVLNode* parent = node->parent;
        if (parent) {
            // attaching subtree to parent
            from = parent->left == node ? &parent->left : &parent->right;
        }

        avl_update(node);
        // fixing height difference of 2
        uint32_t left_height = avl_height(node->left);
        uint32_t right_height = avl_height(node->right);
        if (left_height == right_height + 2) {
            *from = avl_fix_left(node);
        } else if (left_height + 2 == right_height) {
            *from = avl_fix_right(node);
        }

        // if root node, were done
        if (!parent)
            return *from;
        
        // continue to parent node, height might be changed
        node = parent;
    }
}

static AVLNode* avl_del_easy(AVLNode* node) {
    assert(!node->left || !node->right);
    AVLNode* child = node->left ? node->left : node->right;
    AVLNode* parent = node->parent;

    // update childs parent
    if (child)
        child->parent = parent;

    if (!parent)
        return child;
    
    AVLNode** from = parent->left == node ? &parent->left : &parent->right;
    *from = child;

    // rebalancing
    return avl_fix(parent);
}

AVLNode* avl_del(AVLNode* node) {
    // 1 or 0 child
    if (!node->left || !node->right) {
        return avl_del_easy(node);
    }

    //find the successor
    AVLNode* victim = node->right;
    while (victim->left)
        victim = victim->left;
    
    AVLNode* root = avl_del_easy(victim);
    // swap with successor
    *victim = *node;
    if (victim->left)
        victim->left->parent = victim;
    if (victim->right)
        victim->right->parent = victim;
    
    AVLNode** from = &root;
    AVLNode* parent = node->parent;
    if (parent)
        from = parent->left == node ? &parent->left : &parent->right;
    
    *from = victim;
    return root;
}