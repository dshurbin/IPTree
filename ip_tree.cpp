#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <string>
#include <cstring>
/**
 * Created by Dmitry A. Shurbin 02 Jun 2024
 * Copyright (C) 2024 Dmitry A. Shurbin
 * You may use the software freely for non-commercial purposes.
 * Any commercial use without my permission is illegal. Contact: dshurbin@gmail.com
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

extern "C" {
namespace ip_tree {
/**
 * handler_t describes handler to deal with a matched IP address
*/
typedef struct handler_t {
    char name[50];
    int (*func)(uint32_t ip);
    handler_t* next;
} handler_t;

/**
 * node_t describes an IP address tree of trees
*/
typedef struct node_t {
//    uint32_t address;               // node current IP address. If it is at the middle, then it's just xx.yy.0.0
    uint8_t key;                    // the octet value itself
    int height;                     // AVL tree height
    struct node_t* left;
    struct node_t* right;
    struct node_t* subtree;         // pointer for the subsequent octets tree
    struct handler_t* handler_list;
} node_t;

node_t* tree = NULL;

int dummy_handler(uint32_t ip) {
    return 1;
}

/**
 * returns a biggest from two
*/
static inline int max(int a, int b) {
    return (a > b) ? a : b;
}

/**
 * returns AVL height
*/
static inline int height(struct node_t *node) {
    if (node == NULL) {
        return 0;
    }

    return node->height;
}

/**
 * Add a new node
*/
struct node_t *new_node(uint8_t key, uint32_t address) {
    struct node_t *node = (struct node_t *)
    malloc(sizeof(struct node_t));
    node->key = key;
//    node->address = address;
    node->subtree = NULL;
    node->left = NULL;
    node->right = NULL;
    node->height = 1;
    node->handler_list = NULL;

    return (node);
}

/**
 * rotate AVL subtree RIGHT
*/
struct node_t *right_rotate(struct node_t *y) {
    struct node_t *x = y->left;
    struct node_t *t2 = x->right;

    x->right = y;
    y->left = t2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}

/**
 * rotate AVL subtree LEFT
*/
struct node_t *left_rotate(struct node_t *x) {
    struct node_t *y = x->right;
    struct node_t *t2 = y->left;

    y->left = x;
    x->right = t2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

/**
 * balance AVL tree
*/
int get_balance(struct node_t *node) {
    if (node == NULL) {
        return 0;
    }
    
    return height(node->left) - height(node->right);
}

/**
 * Find a correct position for node insertion and performs the insertion
*/
struct node_t *insert_node(struct node_t *node, uint8_t key, uint32_t address) {
  if (node == NULL) {
    return (new_node(key, address));
  }
    

    if (key < node->key) {
        node->left = insert_node(node->left, key, address);
    } else if (key > node->key) {
        node->right = insert_node(node->right, key, address);
    } else {
        return node;
    }

    // Update the balance factor of each node and
    // Balance the tree
    node->height = 1 + max(height(node->left), height(node->right));

    int balance = get_balance(node);
    if (balance > 1 && key < node->left->key) {
        return right_rotate(node);
    }
    if (balance < -1 && key > node->right->key) {
        return left_rotate(node);
    }

    if (balance > 1 && key > node->left->key) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    if (balance < -1 && key < node->right->key) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

struct node_t *min_value_node(struct node_t *node) {
    struct node_t *current = node;
    while (current->left != NULL) {
        current = current->left;
    }

    return current;
}

/**
 * find a node by key (octet)
*/
struct node_t* find_node(struct node_t *node, uint8_t key)
{
    // Find the node and delete it
    if (node == NULL) {
        return node;
    }
    
    if (key < node->key) {
        node = find_node(node->left, key);
    } else if (key > node->key) {
        node = find_node(node->right, key);
    }

  return node;
}

/**
 * Find a node by key (octet) and delete it
*/
struct node_t *delete_node(struct node_t *root, uint8_t key) {
    if (root == NULL) {
        return root;
    }
    if (key < root->key) {
        root->left = delete_node(root->left, key);
    } else if (key > root->key) {
        root->right = delete_node(root->right, key);
    } else {
        if ((root->left == NULL) || (root->right == NULL)) {
            struct node_t *temp = root->left ? root->left : root->right;
            if (temp == NULL) {
                temp = root;
                root = NULL;
            } else {
                *root = *temp;
            }            
            free(temp);
        } else {
            struct node_t *temp = min_value_node(root->right);
            root->key = temp->key;
            root->right = delete_node(root->right, temp->key);
        }
    }

    if (root == NULL) {
        return root;
    }
    // Rebalance the tree after deletion
    root->height = 1 + max(height(root->left), height(root->right));

    int balance = get_balance(root);
    if (balance > 1 && get_balance(root->left) >= 0) {
        return right_rotate(root);
    }

    if (balance > 1 && get_balance(root->left) < 0) {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }

    if (balance < -1 && get_balance(root->right) <= 0) {
        return left_rotate(root);
    }

    if (balance < -1 && get_balance(root->right) > 0) {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }

    return root;
}

/**
 * build JSON, describing node
*/
std::string build_json(struct node_t *node)
{
    std::string result;
    if (!node){
        return "null";
    }
    result = "{\"key\":"+std::to_string(node->key)+",";
//    result = result + "\"address\":"+std::to_string(node->address)+",";
    result = result + "\"subtree\":";    
    result = result + build_json(node->subtree);

    result = result+",\"left\":";    
    result = result + build_json(node->left);
    result = result+",\"right\":";    
    result = result + build_json(node->right);
    if (node->handler_list){
        result = result + ",\"signatures\":[";
        handler_t *sig = node->handler_list;
        while(sig){
          result = result + "\"" + sig->name + "\"";
          if (sig->next){
              result = result + ",";
          }
          sig = sig->next;
        }
        result = result + "]";
    }
    result = result + "}";

    return result;
}

/**
 * find address
*/
struct node_t *find_addr(uint32_t address)
{
    uint8_t key = address >> 24;
    node_t *node = find_node(tree, key);
    if (!node){
        return NULL;
    }

    key = (address >> 16) & 0xff;
    node = find_node(node->subtree, key);
    if (!node){
        return NULL;
    }

    key = (address >> 8) & 0xff;
    node = find_node(node->subtree, key);
    if (!node){
        return NULL;
    }

    key = address & 0xff;
    node = find_node(node->subtree, key);

    return node;
}

/**
 * iterates all handlers in chain and returns immediately if any returns something > 0
*/
int iterate_handlers(handler_t* handler_list, uint32_t address) {
    handler_t* current = handler_list;
    int res = 0;
    while(current){
        res = current->func(address);
        if (res > 0){
            return res;
        }
        current = current->next;
    }

    return res;
}

/**
 * returns 0 if either nothing found or address handler is empty,
 * or handler returns 0
 * if a handler returns any value > 0 returns immediately with the value
*/
int match_addr(uint32_t address) {
    uint8_t key = address >> 24;
    node_t *node = find_node(tree, key);
    int res;

    if (!node){
        return 0;
    }
    if (node->handler_list != NULL) {
        res = iterate_handlers(node->handler_list, address);
        if (res > 0) {
            return res;
        }
    }

    key = (address >> 16) & 0xff;
    node = find_node(node->subtree, key);
    if (!node){
        return 0;
    }
    if (node->handler_list != NULL) {
        res = iterate_handlers(node->handler_list, address);
        if (res > 0) {
            return res;
        }
    }

    key = (address >> 8) & 0xff;
    node = find_node(node->subtree, key);
    if (!node){
        return 0;
    }
    if (node->handler_list != NULL) {
        res = iterate_handlers(node->handler_list, address);
        if (res > 0) {
            return res;
        }
    }

    key = address & 0xff;
    node = find_node(node->subtree, key);
    if (node->handler_list != NULL) {
        res = iterate_handlers(node->handler_list, address);
        if (res > 0) {
            return res;
        }
    }

    return 0;
}

/**
 * Add handler to a node
*/
handler_t* add_handler(node_t* node, std::string signature, int (*func)(uint32_t ip))
{
    handler_t* current = node->handler_list;
    handler_t* prev = node->handler_list;
    int count = 0;
    while(current){
      prev = current;
      if (!strcmp(current->name, signature.c_str())){
          return NULL;
      }
      count++;
      current = current->next;
    }
    handler_t* handler = (struct handler_t *) malloc(sizeof(struct handler_t));
    strcpy(handler->name, signature.c_str());
    handler->func = func;
    handler->next = NULL;
    if (!prev){
        node->handler_list = handler;
    } else {
        prev->next = handler;
    }

    return handler;
}

/**
 * Insert new IP address
*/
bool add_ip_address(std::string ip, std::string signature, int (*func)(uint32_t ip))
{
    uint32_t address;
    address = inet_addr(ip.c_str());
    if (address == 0xFFFFFFFF){
      return false;
    }
    address = htonl(address);
    uint8_t key = address >> 24;
    tree = insert_node(tree, key, 0);

    node_t* node = find_node(tree, key);
    key = (address >> 16) & 0xff;
    node->subtree = insert_node(node->subtree, key, 0);

    node = find_node(node->subtree, key);
    key = (address >> 8) & 0xff;
    node->subtree = insert_node(node->subtree, key, 0);

    node = find_node(node->subtree, key);
    key = address & 0xff;
    node->subtree = insert_node(node->subtree, key, address);

    node = find_node(node->subtree, key);

    add_handler(node, signature, func);

    return true;
}

/**
 * Insert new IP subnet
*/
bool add_ip_subnet(std::string subnet, std::string signature, int (*func)(uint32_t ip))
{
    std::size_t pos = subnet.find("/");
    if (pos == std::string::npos) {
        return false;
    }
    std::string ip = subnet.substr(0,pos);
    int bits;
    try {
        bits = std::stoi(subnet.substr(pos+1));
    } catch(...) {
        return false; // bits are not a number
    }
    if (bits >= 32 || bits < 8) {
        return false;
    }

    uint32_t address;
    address = inet_addr(ip.c_str());
    if (address == 0xFFFFFFFF){
      return false;
    }
    address = htonl(address);

    if ((address << bits) > 0) { // check bounds
        return false;
    }

    uint32_t mask = 0;
    for (int i=0; i < bits; i++) {
        mask >>= 1;
        mask |= 0x80000000;
    }
    //=== all checks and preparations are done here ====
    uint8_t key = address >> 24;
    tree = insert_node(tree, key, 0);
    node_t* node = find_node(tree, key);

    if (bits == 8) {
        add_handler(node, signature, func);
        return true;
    }

    if (bits < 16) {
        uint8_t count = ~((mask >> 16) & 0xff);
        key = (address >> 16) & 0xff;        // add all possible values of the next (2nd) octet to apply handle
        for (uint8_t i=0; i < count; i++) {
            uint8_t tKey = key | i;
            node->subtree = insert_node(node->subtree, tKey, 0);
            node_t* new_node = find_node(node->subtree, tKey);
            add_handler(new_node, signature, func);
        }

        return true;
    }

    key = (address >> 16) & 0xff;
    node->subtree = insert_node(node->subtree, key, 0);
    node = find_node(node->subtree, key);
    if (bits == 16) {
        add_handler(node, signature, func);
        return true;
    }
    if (bits < 24) {
        uint8_t count = ~((mask >> 8) & 0xff);
        key = (address >> 8) & 0xff;        // add all possible values of the next (3rd) octet to apply handle
        for (uint8_t i=0; i <= count; i++) {
            uint8_t tKey = key | i;
            printf("tKey: %d\n", tKey);
            node->subtree = insert_node(node->subtree, tKey, 0);
            node_t* new_node = find_node(node->subtree, tKey);
            add_handler(new_node, signature, func);
        }

        return true;
    }

    key = (address >> 8) & 0xff;
    node->subtree = insert_node(node->subtree, key, 0);
    node = find_node(node->subtree, key);
    if (bits == 24) {
        add_handler(node, signature, func);
        return true;
    }
    uint8_t count = ~(mask & 0xff);
    key = address & 0xff;                  // add all possible values of the last octet to apply handle
    for (uint8_t i=0; i < count; i++) {
        uint8_t tKey = key | i;
        node->subtree = insert_node(node->subtree, tKey, 0);
        node_t* new_node = find_node(node->subtree, tKey);
        add_handler(new_node, signature, func);
    }

    return true;
}
} // namespace
} // extern