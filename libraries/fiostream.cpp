// 注意: Linux (x86-64 SSE4.1) + C++20 向け; cin/cout/istream/ostream をマクロ置換する
//       std::cin/cout・マニピュレータ(fixed, setprecision, endl 等)・getline・scanf/printf と併用不可
//       浮動小数点出力は小数点以下 15 桁固定; 整数入力はオーバーフロー非検査; 対話問題では cout.flush() 必須
#include <immintrin.h>
#ifdef __linux__
#include <cerrno>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

struct FastIstream{
    static constexpr size_t SIZ = 1 << 17;
    char buf[SIZ];
    const char *p = nullptr, *end = nullptr;
    bool inited = false;
    bool mapped = false;
    bool eof_reached = false;
    bool ok = true;
    
    alignas(16) static constexpr int8_t SHUF[17][16] = {
        {-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128},
        {-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,   0},
        {-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,   0,   1},
        {-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,   0,   1,   2},
        {-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,   0,   1,   2,   3},
        {-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,   0,   1,   2,   3,   4},
        {-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,   0,   1,   2,   3,   4,   5},
        {-128,-128,-128,-128,-128,-128,-128,-128,-128,   0,   1,   2,   3,   4,   5,   6},
        {-128,-128,-128,-128,-128,-128,-128,-128,   0,   1,   2,   3,   4,   5,   6,   7},
        {-128,-128,-128,-128,-128,-128,-128,   0,   1,   2,   3,   4,   5,   6,   7,   8},
        {-128,-128,-128,-128,-128,-128,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9},
        {-128,-128,-128,-128,-128,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10},
        {-128,-128,-128,-128,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11},
        {-128,-128,-128,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12},
        {-128,-128,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13},
        {-128,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14},
        {   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15}
    };
    
    explicit operator bool() const { return ok; }
    
    inline bool _refill(){
        if(!inited) [[unlikely]] {
            inited = true;
            #ifdef __linux__
            struct stat st;
            if(fstat(0, &st) == 0 && S_ISREG(st.st_mode)){
                if(st.st_size == 0){
                    mapped = true;
                    eof_reached = true;
                    p = end = nullptr;
                    return false;
                }
                void* ptr = mmap(nullptr, static_cast<size_t>(st.st_size), PROT_READ, MAP_PRIVATE, 0, 0);
                if(ptr != MAP_FAILED){
                    madvise(ptr, static_cast<size_t>(st.st_size), MADV_SEQUENTIAL);
                    p = static_cast<const char*>(ptr);
                    end = p + st.st_size;
                    mapped = true;
                    return true;
                }
            }
            #endif
        }
        if(mapped || eof_reached) [[unlikely]] {
            eof_reached = true;
            return false;
        }
        #ifdef __linux__
        ssize_t n;
        do {
            n = ::read(0, buf, SIZ);
        } while(n < 0 && errno == EINTR);
        if(n <= 0){
            eof_reached = true;
            p = end = buf;
            return false;
        }
        p = buf;
        end = buf + n;
        return true;
        #else
        size_t n = fread(buf, 1, SIZ, stdin);
        if(n == 0){
            eof_reached = true;
            p = end = buf;
            return false;
        }
        p = buf;
        end = buf + n;
        return true;
        #endif
    }
    
    inline int _getchar(){
        if(p == end) [[unlikely]] {
            if(!_refill()) return -1;
        }
        return static_cast<unsigned char>(*p++);
    }
    
    inline bool _skip_ws(){
        while(p < end && static_cast<unsigned char>(*p) <= 0x20) [[unlikely]] ++p;
        while(p == end) [[unlikely]] {
            if(!_refill()) return false;
            while(p < end && static_cast<unsigned char>(*p) <= 0x20) ++p;
        }
        return true;
    }
    
    __attribute__((target("sse4.1")))
    static inline uint64_t _parse16(__m128i d, int len){
        d = _mm_shuffle_epi8(d, _mm_load_si128(reinterpret_cast<const __m128i*>(SHUF[len])));
        d = _mm_maddubs_epi16(d, _mm_setr_epi8(10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1, 10, 1));
        d = _mm_madd_epi16(d, _mm_setr_epi16(100, 1, 100, 1, 100, 1, 100, 1));
        d = _mm_packs_epi32(d, d);
        d = _mm_madd_epi16(d, _mm_setr_epi16(10000, 1, 10000, 1, 0, 0, 0, 0));
        uint64_t hl = static_cast<uint64_t>(_mm_cvtsi128_si64(d));
        return static_cast<uint64_t>(static_cast<uint32_t>(hl)) * 100000000ULL + (hl >> 32);
    }
    
    template<typename T>
    __attribute__((target("sse4.1")))
    inline enable_if_t<is_integral_v<T> && !is_same_v<T, bool>> _read_int(T& res){
        if(!_skip_ws()) [[unlikely]] { ok = false; return; }
        using U = make_unsigned_t<T>;
        if(end - p >= 32) [[likely]] {
            bool neg = false;
            if constexpr (is_signed_v<T>){
                if(*p == '-') [[unlikely]] { neg = true; ++p; }
                else if(*p == '+') [[unlikely]] { ++p; }
            } else {
                if(*p == '+') [[unlikely]] { ++p; }
            }
            __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
            __m128i d = _mm_sub_epi8(v, _mm_set1_epi8('0'));
            __m128i cmp = _mm_cmpgt_epi8(_mm_set1_epi8(-118), _mm_sub_epi8(v, _mm_set1_epi8(static_cast<char>('0' - 128))));
            int len = __builtin_ctz(~_mm_movemask_epi8(cmp));
            if(len == 0) [[unlikely]] { ok = false; return; }
            U x = static_cast<U>(_parse16(d, len));
            p += len;
            if(len == 16) [[unlikely]] {
                while(true){
                    while(p < end && static_cast<unsigned>(*p - '0') < 10u){
                        x = x * 10 + static_cast<U>(*p - '0');
                        ++p;
                    }
                    if(p < end || !_refill()) break;
                }
            }
            if(p < end && static_cast<unsigned char>(*p) <= 0x20) ++p;
            if constexpr (is_signed_v<T>) res = neg ? static_cast<T>(U(0) - x) : static_cast<T>(x);
            else res = x;
            return;
        }
        int c = _getchar();
        bool neg = false;
        if constexpr (is_signed_v<T>){
            if(c == '-'){ neg = true; c = _getchar(); }
            else if(c == '+'){ c = _getchar(); }
        } else {
            if(c == '+'){ c = _getchar(); }
        }
        if(c < '0' || c > '9'){
            if(c != -1) --p;
            ok = false;
            return;
        }
        U x = static_cast<U>(c - '0');
        c = _getchar();
        while('0' <= c && c <= '9'){
            x = x * 10 + static_cast<U>(c - '0');
            c = _getchar();
        }
        if(c > 0x20) --p;
        if constexpr (is_signed_v<T>) res = neg ? static_cast<T>(U(0) - x) : static_cast<T>(x);
        else res = x;
    }
    
    __attribute__((target("sse4.1")))
    inline bool _read_token(string& tok){
        tok.clear();
        if(!_skip_ws()) [[unlikely]] { ok = false; return false; }
        const __m128i bias = _mm_set1_epi8(-128);
        const __m128i lim = _mm_set1_epi8(-95);
        while(true){
            const char* start = p;
            while(end - p >= 16){
                __m128i v = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p));
                int mask = _mm_movemask_epi8(_mm_cmpgt_epi8(lim, _mm_xor_si128(v, bias)));
                if(mask != 0){
                    p += __builtin_ctz(mask);
                    goto token_chunk_done;
                }
                p += 16;
            }
            while(p < end && static_cast<unsigned char>(*p) > 0x20) ++p;
            token_chunk_done:
            if(tok.empty()) tok.assign(start, static_cast<size_t>(p - start));
            else tok.append(start, static_cast<size_t>(p - start));
            if(p < end){
                ++p;
                break;
            }
            if(!_refill()) break;
        }
        return true;
    }
    
    template<typename T>
    inline void _read_float(T& x){
        if(!_skip_ws()) [[unlikely]] { ok = false; return; }
        const char* start = p;
        while(p < end && static_cast<unsigned char>(*p) > 0x20) ++p;
        if(p < end || mapped || eof_reached) [[likely]] {
            const char* first = (start < p && *start == '+') ? start + 1 : start;
            auto [ptr, ec] = from_chars(first, p, x);
            if(p < end) ++p;
            if(ec != errc{} || ptr == first) ok = false;
            return;
        }
        string s(start, static_cast<size_t>(p - start));
        while(_refill()){
            start = p;
            while(p < end && static_cast<unsigned char>(*p) > 0x20) ++p;
            s.append(start, static_cast<size_t>(p - start));
            if(p < end){
                ++p;
                break;
            }
        }
        const char* first = (!s.empty() && s[0] == '+') ? s.data() + 1 : s.data();
        auto [ptr, ec] = from_chars(first, s.data() + s.size(), x);
        if(ec != errc{} || ptr == first) ok = false;
    }
    
    template<typename T, typename = enable_if_t<is_integral_v<T> && !is_same_v<T, bool> && !is_same_v<T, char>>>
    inline FastIstream& operator>>(T& x){ _read_int(x); return *this; }
    inline FastIstream& operator>>(bool& x){
        unsigned long long v = 0;
        _read_int(v);
        if(ok) x = (v != 0);
        return *this;
    }
    inline FastIstream& operator>>(char& x){
        if(!_skip_ws()) [[unlikely]] { ok = false; return *this; }
        x = *p++;
        return *this;
    }
    inline FastIstream& operator>>(string& x){
        _read_token(x);
        return *this;
    }
    inline FastIstream& operator>>(float& x){ _read_float(x); return *this; }
    inline FastIstream& operator>>(double& x){ _read_float(x); return *this; }
    inline FastIstream& operator>>(long double& x){ _read_float(x); return *this; }
} fast_cin;

struct FastOstream{
    static constexpr size_t SIZ = 1 << 18;
    struct Digits4{
        char num[10000][4];
    };
    static constexpr Digits4 digits = []{
        Digits4 d{};
        for(int i = 0; i < 10000; i++){
            int x = i;
            for(int j = 3; j >= 0; j--){
                d.num[i][j] = static_cast<char>(x % 10 + '0');
                x /= 10;
            }
        }
        return d;
    }();
    
    char buf[SIZ], *p1 = buf, *p2 = buf + SIZ;
    
    static inline void _raw_write(const char* s, size_t len){
        #ifdef __linux__
        while(len > 0){
            ssize_t w = ::write(1, s, len);
            if(w > 0){
                s += w;
                len -= static_cast<size_t>(w);
            } else if(w < 0 && errno == EINTR){
                continue;
            } else {
                break;
            }
        }
        #else
        fwrite(s, 1, len, stdout);
        #endif
    }
    
    inline void _write(){
        _raw_write(buf, static_cast<size_t>(p1 - buf));
        p1 = buf;
    }
    
    inline void _putchar(char c){
        if(p1 == p2) [[unlikely]] _write();
        *p1++ = c;
    }
    
    inline void _write_str(const char* s, size_t len){
        if(len <= static_cast<size_t>(p2 - p1)) [[likely]] {
            memcpy(p1, s, len);
            p1 += len;
        } else {
            if(p1 != buf) _write();
            if(len >= SIZ) _raw_write(s, len);
            else { memcpy(p1, s, len); p1 += len; }
        }
    }
    
    void flush(){
        if(p1 != buf) _write();
        #ifndef __linux__
        fflush(stdout);
        #endif
    }
    
    template<typename T>
    inline void _write_int(T x){
        constexpr int DIGIT_SIZ = 40;
        static_assert(DIGIT_SIZ <= SIZ);
        if(p2 - p1 < DIGIT_SIZ) [[unlikely]] _write();
        using U = make_unsigned_t<T>;
        U u;
        if constexpr (is_signed_v<T>){
            if(x < 0){ *p1++ = '-'; u = U(0) - static_cast<U>(x); }
            else u = static_cast<U>(x);
        } else {
            u = x;
        }
        char num[DIGIT_SIZ], *idxp = num + DIGIT_SIZ;
        if constexpr (sizeof(U) > 4){
            while(u >= 100000000ULL){
                uint32_t rem = static_cast<uint32_t>(u % 100000000ULL);
                u /= 100000000ULL;
                idxp -= 8;
                memcpy(idxp + 4, digits.num[rem % 10000u], 4);
                memcpy(idxp, digits.num[rem / 10000u], 4);
            }
        }
        uint32_t u32 = static_cast<uint32_t>(u);
        while(u32 >= 10000u){
            idxp -= 4;
            memcpy(idxp, digits.num[u32 % 10000u], 4);
            u32 /= 10000u;
        }
        if(u32 >= 1000u){ memcpy(p1, digits.num[u32], 4); p1 += 4; }
        else if(u32 >= 100u){ memcpy(p1, digits.num[u32] + 1, 3); p1 += 3; }
        else if(u32 >= 10u){ memcpy(p1, digits.num[u32] + 2, 2); p1 += 2; }
        else *p1++ = static_cast<char>(u32 + '0');
        size_t tail = static_cast<size_t>(num + DIGIT_SIZ - idxp);
        memcpy(p1, idxp, tail);
        p1 += tail;
    }
    
    template<typename T, typename = enable_if_t<is_integral_v<T> && !is_same_v<T, bool> && !is_same_v<T, char>>>
    inline FastOstream& operator<<(const T& x){ _write_int(x); return *this; }
    inline FastOstream& operator<<(bool x){ _putchar(x ? '1' : '0'); return *this; }
    inline FastOstream& operator<<(char x){ _putchar(x); return *this; }
    inline FastOstream& operator<<(const char* x){ if(x) _write_str(x, strlen(x)); return *this; }
    inline FastOstream& operator<<(char* x){ return *this << const_cast<const char*>(x); }
    inline FastOstream& operator<<(float x){ return *this << static_cast<double>(x); }
    inline FastOstream& operator<<(double x){
        if(isnan(x)) [[unlikely]] { _write_str("nan", 3); return *this; }
        if(isinf(x)) [[unlikely]] { signbit(x) ? _write_str("-inf", 4) : _write_str("inf", 3); return *this; }
        char b[64];
        auto [ptr, ec] = to_chars(b, b + sizeof(b), x, chars_format::fixed, 15);
        if(ec == errc{}) [[likely]] { _write_str(b, static_cast<size_t>(ptr - b)); return *this; }
        string s(512, '\0');
        auto [ptr2, ec2] = to_chars(s.data(), s.data() + s.size(), x, chars_format::fixed, 15);
        if(ec2 == errc{}) _write_str(s.data(), static_cast<size_t>(ptr2 - s.data()));
        return *this;
    }
    inline FastOstream& operator<<(long double x){
        if(isnan(x)) [[unlikely]] { _write_str("nan", 3); return *this; }
        if(isinf(x)) [[unlikely]] { signbit(x) ? _write_str("-inf", 4) : _write_str("inf", 3); return *this; }
        char b[64];
        auto [ptr, ec] = to_chars(b, b + sizeof(b), x, chars_format::fixed, 15);
        if(ec == errc{}) [[likely]] { _write_str(b, static_cast<size_t>(ptr - b)); return *this; }
        string s(5200, '\0');
        auto [ptr2, ec2] = to_chars(s.data(), s.data() + s.size(), x, chars_format::fixed, 15);
        if(ec2 == errc{}) _write_str(s.data(), static_cast<size_t>(ptr2 - s.data()));
        return *this;
    }
    inline FastOstream& operator<<(const string& x){ _write_str(x.data(), x.size()); return *this; }
    ~FastOstream(){ if(p1 != buf) _write(); }
} fast_cout;

#define cin fast_cin
#define istream FastIstream
#define cout fast_cout
#define ostream FastOstream
