struct PlayBuf : public Unit
{
    double m_phase;
    float m_prevtrig;
    float m_fbufnum;
    float m_failedBufNum;
    SndBuf *m_buf;
};

void PlayBuf_next_aa(PlayBuf *unit, int inNumSamples);
void PlayBuf_next_ak(PlayBuf *unit, int inNumSamples);
void PlayBuf_next_ka(PlayBuf *unit, int inNumSamples);
void PlayBuf_next_kk(PlayBuf *unit, int inNumSamples);
void PlayBuf_Ctor(PlayBuf* unit);
