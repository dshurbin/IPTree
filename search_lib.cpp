#include <arpa/inet.h>
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

/**
 * load a bunch of IP addresses from a file
 * handler-name-1
 * >aa.bb.cc.dd
 * >ee.ff.gg.hh
 * ...
 * handler-name-N
 * >ii.jj.kk.ll
 * >mm.nn.oo.pp
 * 
*/
void load_tree(char* file_name)
{
    FILE* f;
    f = fopen(file_name, "r");
    std::string signature;
    char buf[200];
    while((fgets(buf, 200, f))){
        std::string line(buf);
        if (line.length() > 2){
          line.erase(line.length()-1);
        }
        if (!line.length()){
          continue;
        }
        if (line[0] != '>'){
          signature = line;
        } else
        if(signature.length()){
            line = line.substr(1);
            ip_tree::add_ip_address(line, signature, ip_tree::dummy_handler);
        } 

    }
    fclose(f);
}

int main()
{
    ip_tree::tree = NULL;
//==== Here are the possible using examples
//    load_tree("list.txt");
//    std::string json = build_json(tree);
//    printf("json: %s\n", json.c_str());

//    ip_tree::node_t* addr = ip_tree::find_addr(0x268476c7);
//    ip_tree::node_t* addr = ip_tree::find_addr(0x25eb300a);
//    std::string json = ip_tree::build_json(addr);

    ip_tree::add_ip_subnet("192.168.170.0/23", "test", ip_tree::dummy_handler);
    std::string json = ip_tree::build_json(ip_tree::tree);
    printf("json: %s\n", json.c_str());
    uint32_t ip;
    std::string ip_str = "192.168.171.28";
    ip = htonl(inet_addr(ip_str.c_str()));
    printf("match: ip: %s 0x%08X result: %d\n", ip_str.c_str(), ip, ip_tree::match_addr(ip));
    return 0;
}