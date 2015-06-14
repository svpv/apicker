#include "areader.h"
#include <sigc++/signal.h>
class APlayer: public AReader
{
public:
    APlayer(const char *fname);
    ~APlayer();
    void play_bg(unsigned csec);
    void play_bga()
    {
	play_bg(getpos());
    }
    void stop_bg();
    sigc::signal<void, unsigned> sig_bg_pos;
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
