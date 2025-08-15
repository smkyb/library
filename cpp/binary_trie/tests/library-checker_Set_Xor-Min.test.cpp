#define PROBLEM "https://judge.yosupo.jp/problem/set_xor_min"

#include "../../../_initialize.cpp"
#include "../binary_trie.cpp"

int main(){
    uint q, x; char t;
    cin >> q;
    
    binary_trie<uint> S;
    while(q--){
        cin >> t >> x;
        switch(t){
            case '0':
                S.insert(x);
                break;
            case '1':
                S.erase_all(x);
                break;
            case '2':
                cout << (x^S.xor_min(x)) << '\n';
                break;
        }
    }
}
