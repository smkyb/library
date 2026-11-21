// 注意: 文字種は 'a'..'z' と第2引数の 1文字 extra_char (既定 '_') のみ (それ以外の文字は配列外参照 UB)
//       空文字列 "" は insert / count / erase のいずれでも無視される
struct trie{
    struct node_t{
        array<int, 27> ch;
        int sub_cnt = 0;
        int end_cnt = 0;
        node_t(){ ch.fill(-1); }
    };
    vector<node_t> node;
    vector<int> reuse;
    vector<pair<int, int>> path_buf;
    char ex;
    
    trie(int reserve_size = 1, char extra_char = '_') : ex(extra_char) {
        node.reserve(max(1, reserve_size));
        make_node();
    }
    
    int make_node(){
        if(reuse.empty()){
            node.emplace_back();
            return static_cast<int>(node.size()) - 1;
        }
        int res = reuse.back();
        reuse.pop_back();
        node[res] = node_t();
        return res;
    }
    
    void insert(const string& s){
        if(s.empty()) return;
        int now = 0;
        for(char ch : s){
            int c = (ch == ex) ? 26 : (ch - 'a');
            int nx = node[now].ch[c];
            if(nx == -1){
                nx = make_node();
                node[now].ch[c] = nx;
            }
            node[nx].sub_cnt++;
            now = nx;
        }
        node[now].end_cnt++;
    }
    
    int count(const string& s) const {
        if(s.empty()) return 0;
        int now = 0;
        for(char ch : s){
            int c = (ch == ex) ? 26 : (ch - 'a');
            int nx = node[now].ch[c];
            if(nx == -1) return 0;
            now = nx;
        }
        return node[now].end_cnt;
    }
    
    int erase(const string& s, int limit = numeric_limits<int>::max()){
        if(s.empty() || limit <= 0) return 0;
        path_buf.clear();
        int now = 0;
        for(char ch : s){
            int c = (ch == ex) ? 26 : (ch - 'a');
            int nx = node[now].ch[c];
            if(nx == -1) return 0;
            path_buf.emplace_back(now, c);
            now = nx;
        }
        int del = min(limit, node[now].end_cnt);
        if(del == 0) return 0;
        node[now].end_cnt -= del;
        for(int i = static_cast<int>(path_buf.size()) - 1; i >= 0; --i){
            auto [p, c] = path_buf[i];
            int child = node[p].ch[c];
            node[child].sub_cnt -= del;
            if(node[child].sub_cnt == 0){
                reuse.push_back(child);
                node[p].ch[c] = -1;
            }
        }
        return del;
    }
};