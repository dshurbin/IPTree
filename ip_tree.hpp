#pragma once
#include <stdint.h>
#include <string>
#include "ip_tree.hpp"
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

extern "C" node_t* tree;
extern "C" int dummy_handler(uint32_t ip);
extern "C" struct node_t* find_node(struct node_t *node, uint8_t key);
extern "C" std::string build_json(struct node_t *node);
extern "C" struct node_t *find_addr(uint32_t address);
extern "C" int iterate_handlers(handler_t* handler_list, uint32_t address);
extern "C" int match_addr(uint32_t address);
extern "C" bool add_ip_address(std::string ip, std::string signature, int (*func)(uint32_t ip));
extern "C" bool add_ip_subnet(std::string subnet, std::string signature, int (*func)(uint32_t ip));

}