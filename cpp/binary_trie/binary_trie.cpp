template <typename T>
struct binary_trie{
    private:
    
    struct node_t{
        T value;
        int count = 0;
        int child[2] = {-1, -1};
        int width = 0;
        node_t& operator=(const node_t& o){
            value = o.value;
            count = o.count;
            child[0] = o.child[0];
            child[1] = o.child[1];
            width = o.width;
            return *this;
        }
    };
    
    using uT = typename make_unsigned<T>::type;
    static constexpr T one = 1;
    static constexpr int bit_width = sizeof(T) * 8;
    vector<node_t> node;
    int root = -1;
    int siz = 0;
    
    //[l, r)のマスクを返す
    T mask(int l, int r) const {
        if(r == bit_width) return ~((one<<l)-1);
        return (((one<<r)-1) & ~((one<<l)-1));
    }
    
    //[l, r)bitを取り出す
    T masked(T v, int l, int r) const {
        return mask(l, r) & v;
    }
    
    //上位から数えて初めて異なるbitを返す。無ければ-1
    int diff_bit(T x, T y) const {
        return bit_width-1 - countl_zero(uT(x^y));
    }
    
    //ノードを作って返す
    int make_node(T v){
        node.emplace_back();
        node.back().value = v;
        return ssize(node)-1;
    }
    
    public:
    
    binary_trie(){
        root = make_node(0);
        make_node(0);
        make_node(1);
        node[0].width = bit_width-1;
        node[0].child[0] = 1;
        node[0].child[1] = 2;
    }
    
    binary_trie<T>& operator=(binary_trie<T>&& o) noexcept {
        node = move(o.node);
        root = o.root;
        siz = o.siz;
    }
    
    binary_trie(binary_trie&& o) noexcept {
        *this = move(o);
    }
    
    void insert(T v){
        int pos = root;
        int bit = bit_width;
        siz++;
        while(node[pos].child[0] != -1){
            T mv = masked(v, bit-node[pos].width, bit);
            T mnv = masked(node[pos].value, bit-node[pos].width, bit);
            if(mv != mnv){
                int diff = diff_bit(mv, mnv);
                bool b = (mv>>diff)&1;
                int inter = make_node(node[pos].value);
                int leaf = make_node(v);
                node[inter] = node[pos];
                node[inter].width -= (bit - diff - 1);
                node[pos].child[b] = leaf;
                node[pos].child[!b] = inter;
                node[pos].count++;
                node[pos].width = bit - diff - 1;
                bit -= node[pos].width;
                node[leaf].width = bit;
                pos = leaf;
            } else {
                node[pos].count++;
                bit -= node[pos].width;
                pos = node[pos].child[(v>>(bit-1))&1];
            }
        }
        
        if(node[pos].value == v){
            node[pos].count++;
            return;
        }
        
        int diff = diff_bit(v, node[pos].value);
        bool b = (v>>diff)&1;
        int leafv = make_node(v);
        int leafn = make_node(node[pos].value);
        node[pos].child[b] = leafv;
        node[pos].child[!b] = leafn;
        node[leafn].count = node[pos].count;
        node[pos].count += 1;
        node[leafv].width = node[leafn].width = node[pos].width - (bit - diff - 1);
        node[pos].width = bit - diff - 1;
        node[leafv].count = 1;
    }
    
    int count(T v) const {
        int pos = root;
        int bit = bit_width;
        while(node[pos].child[0] != -1){
            T mv = masked(v, bit-node[pos].width, bit);
            T mnv = masked(node[pos].value, bit-node[pos].width, bit);
            if(mv != mnv) return 0;
            bit -= node[pos].width;
            pos = node[pos].child[(v>>(bit-1))&1];
        }
        
        if(node[pos].value != v) return 0;
        else return node[pos].count;
    }
    
    void erase(T v, int n = -1){
        if(n == -1) n = count(v);
        if(n == 0) return;
        int pos = root;
        int bit = bit_width;
        siz -= n;
        while(node[pos].child[0] != -1){
            node[pos].count -= n;
            bit -= node[pos].width;
            pos = node[pos].child[(v>>(bit-1))&1];
        }
        node[pos].count -= n;
    }
    
    T xor_min(T v) const {
        assert(siz != 0);
        int pos = root;
        int bit = bit_width;
        while(node[pos].child[0] != -1){
            v ^= masked(node[pos].value, bit-node[pos].width, bit);
            int c0 = node[pos].child[0], c1 = node[pos].child[1];
            bit -= node[pos].width;
            if(node[c0].count == 0){
                pos = c1;
            } else if(node[c1].count == 0){
                pos = c0;
            } else {
                T v0 = v ^ masked(node[c0].value, bit-node[c0].width, bit);
                T v1 = v ^ masked(node[c1].value, bit-node[c1].width, bit);
                if(v0 < v1) pos = c0;
                else pos = c1;
            }
        }
        return v ^ masked(node[pos].value, bit-node[pos].width, bit);
    }
    
    T xor_max(T v) const {
        assert(siz != 0);
        int pos = root;
        int bit = bit_width;
        while(node[pos].child[0] != -1){
            v ^= masked(node[pos].value, bit-node[pos].width, bit);
            int c0 = node[pos].child[0], c1 = node[pos].child[1];
            bit -= node[pos].width;
            if(node[c0].count == 0){
                pos = c1;
            } else if(node[c1].count == 0){
                pos = c0;
            } else {
                T v0 = v ^ masked(node[c0].value, bit-node[c0].width, bit);
                T v1 = v ^ masked(node[c1].value, bit-node[c1].width, bit);
                if(v0 > v1) pos = c0;
                else pos = c1;
            }
        }
        return v ^ masked(node[pos].value, bit-node[pos].width, bit);
    }
    
    T get_largest(int k) const {
        assert(0 <= k && k < siz);
        k++;
        int pos = root;
        while(node[pos].child[0] != -1){
            if(k <= node[node[pos].child[1]].count){
                pos = node[pos].child[1];
            } else {
                k -= node[node[pos].child[1]].count;
                pos = node[pos].child[0];
            }
        }
        return node[pos].value;
    }
    
    T get_smallest(int k) const {
        return get_largest(siz - k - 1);
    }
    
    int size() const {
        return siz;
    }
    
    //以下の要素の個数
    int order(T v) const {
        int res = 0;
        int pos = root;
        int bit = bit_width;
        while(node[pos].child[0] != -1){
            T mv = masked(v, bit-node[pos].width, bit);
            T mnv = masked(node[pos].value, bit-node[pos].width, bit);
            if(mv < mnv){
                return res;
            } else if(mv > mnv){
                return res + node[pos].count;
            } else {
                bit -= node[pos].width;
                bool b = (v>>(bit-1))&1;
                if (b){
                    res += node[node[pos].child[0]].count;
                }
                pos = node[pos].child[b];
            }
        }
        if(node[pos].value <= v) res += node[pos].count;
        return res;
    }
    
    T lower_bound(T v) const {
        int ord = v ? order(v-1) : 0;
        if(siz == ord) return -1;
        else return get_smallest(ord);
    }
    
    T less_bound(T v) const {
        int ord = order(v);
        if(ord == 0) return -1;
        else return get_smallest(ord-1);
    }
    
    void reserve(int n){
        node.reserve(2*n+9);
    }
};
