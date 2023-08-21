
class Sha1 {
public:
    Sha1();
    virtual ~Sha1();

    Sha1(const Sha1&) = delete;
    Sha1& operator=(const Sha1&) = delete;

    void update(const uint8_t* input, size_t len);
    void finish(uint8_t* out_buff);
private:
    void* data;
};
