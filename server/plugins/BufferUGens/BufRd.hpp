struct BufRd : public Unit
{
    float m_fbufnum;
    float m_failedBufNum;
    SndBuf *m_buf;
};

void BufRd_Ctor(BufRd *unit);
void BufRd_next_4(BufRd *unit, int inNumSamples);
void BufRd_next_2(BufRd *unit, int inNumSamples);
void BufRd_next_1(BufRd *unit, int inNumSamples);
