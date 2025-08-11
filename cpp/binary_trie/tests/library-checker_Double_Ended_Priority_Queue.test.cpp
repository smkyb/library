#define PROBLEM "https://judge.yosupo.jp/problem/double_ended_priority_queue"

#include "../../../_initialize.cpp"
#include "../binary_trie.cpp"
constexpr int num = 1000000000;

int main(){
    int n, q;
    cin >> n >> q;
    
    binary_trie<unsigned> tree;
    
    for(int i = 0; i < n; i++){
        int x;
        cin >> x;
        tree.insert(x+num);
    }
    
    while(q--){
        int t, x;
        cin >> t;
        switch(t){
            case 0:
                cin >> x;
                tree.insert(x+num);
                break;
            case 1:
                cout << (x = tree[0])-num << '\n';
                tree.erase(x);
                break;
            case 2:
                cout << (x = tree[-1])-num << '\n';
                tree.erase(x);
                break;
        }
    }
}