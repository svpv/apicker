#include "areader.h"
class APlayer: public AReader
{
public:
    APlayer(const char *fname);
    ~APlayer();
    void play_bg(unsigned csec);
    void stop_bg();
protected:
    void process(short  *data, size_t n) override;
    void process(short **data, size_t n) override;
    void process(float  *data, size_t n) override;
    void process(float **data, size_t n) override;
//private:
public:
    class Ctx;
    Ctx *m_ctx;
};
