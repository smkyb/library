struct FIstream{
    #ifndef LOCAL
    #define fread fread_unlocked
    #endif
    
    static constexpr unsigned SIZ = 1 << 17;
    char buf[SIZ], *p1 = buf, *p2 = buf;
    
    inline char _getchar(){
        if(p1 == p2){
            p2 = (p1 = buf) + fread(buf, 1, SIZ, stdin);
            if(p1 == p2) [[unlikely]] assert(0&&"EOF");
        }
        return *p1++;
    }
    
    inline char ignore_space(){
        char c;
        while((c = _getchar()) <= 0x20);
        return c;
    }
    
    template<typename T>
    inline void _read(T& res){
        T x = 0, f = 1;
        char c = ignore_space();
        if(c == '-'){
            f = -1;
            c = _getchar();
        }
        while('0' <= c && c <= '9'){
            x = x*10 + (c-'0');
            c = _getchar();
        }
        res = x*f;
    }
    
    template<typename T>
    inline FIstream& operator>>(T& x){_read(x); return *this;}
    inline FIstream& operator>>(char& x){x = ignore_space(); return *this;}
    #ifdef _GLIBCXX_STRING
    inline FIstream& operator>>(string& x){
        string().swap(x);
        char c = ignore_space();
        while(c > 0x20){
            x.push_back(c);
            c = _getchar();
        }
        return *this;
    }
    #endif
    #ifndef LOCAL
    #undef fread
    #endif
} _cin;
#define cin _cin
#define istream FIstream

struct FOstream_Pre{
    char num[10000][4];
    constexpr FOstream_Pre():num(){
        for(int i = 0; i < 10000; i++){
            int x = i;
            for(int j = 3; j >= 0; j--){
                num[i][j] = x%10 + '0';
                x /= 10;
            }
        }
    }
} constexpr _FOstream_pre;

struct FOstream{
    #ifndef LOCAL
    #define fwrite fwrite_unlocked
    #endif
    
    static constexpr unsigned SIZ = 1 << 17;
    char buf[SIZ], *p1 = buf, *p2 = buf+SIZ;
    
    inline void _write(){
        fwrite(buf, 1, p1-buf, stdout);
        p1 = buf;
    }
    
    inline void _putchar(char c){
        if(p1 == p2) [[unlikely]] {
            _write();
        }
        *p1++ = c;
    }
    
    template<typename T>
    void _write_i(T x){
        constexpr int DIGIT_SIZ = 40;
        char num[DIGIT_SIZ], *idxp = num+DIGIT_SIZ;
        if(x < 0){
            _putchar('-');
            x = -x;
        }
        if(p2 - p1 < DIGIT_SIZ) _write();
        while(x >= 10000){
            idxp -= 4;
            memcpy(idxp, _FOstream_pre.num[x%10000], 4);
            x /= 10000;
        }
        if(x >= 1000){
            memcpy(p1, _FOstream_pre.num[x], 4);
            p1 += 4;
        } else if(x >= 100){
            memcpy(p1, _FOstream_pre.num[x]+1, 3);
            p1 += 3;
        } else if(x >= 10){
            memcpy(p1, _FOstream_pre.num[x]+2, 2);
            p1 += 2;
        } else *p1++ = x+'0';
        memcpy(p1, idxp, num+DIGIT_SIZ - idxp);
        p1 += num+DIGIT_SIZ - idxp;
    }
    
    template<typename T>
    FOstream &operator<<(const T &x) {_write_i(x); return *this;}
    FOstream &operator<<(char x) {_putchar(x); return *this;}
    FOstream &operator<<(const char *x) {
        while(*x) _putchar(*x++);
        return *this;
    }
    FOstream &operator<<(char *x) {return *this << const_cast<const char*>(x);}
    FOstream &operator<<(double x) {
        if(isnan(x)) [[unlikely]] return *this << "nan";
        char _b[70];
        snprintf(_b, sizeof(_b), "%.*f", 15, x);
        return *this << const_cast<const char*>(_b);
    }
    FOstream &operator<<(long double x) {
        if(isnan(x)) [[unlikely]] return *this << "nan";
        char _b[330];
        snprintf(_b, sizeof(_b), "%.*Lf", 15, x);
        return *this << const_cast<const char*>(_b);
    }
    #ifdef _GLIBCXX_STRING
    FOstream& operator<<(const string& x){
        for(char i : x) _putchar(i);
        return *this;
    }
    #endif
    
    ~FOstream(){
        if(p1 != buf){
            fwrite(buf, 1, p1 - buf, stdout);
        }
    }
    
    #ifndef LOCAL
    #undef fwrite
    #endif
} _cout;
#define cout _cout
#define ostream FOstream
