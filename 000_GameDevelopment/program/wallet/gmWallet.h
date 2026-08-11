// gmWallet.h
#pragma once

namespace gm {

    // ------------------------------------------------------------
    // プレイヤーの資金・溶かす経験値を保持するウォレット。
    //
    // 資金(funds_)は交易船の到着報酬のように、まとまった単位でしか加算されないため
    // int で十分だが、溶かす経験値(meltExp_)は炎放射のように毎フレーム細かい
    // ダメージ量(deltaTime秒あたりの微小な値)から換算されるため、int だと
    // 毎フレームの端数切り捨てで実質ほとんど加算されない、という問題が起きる。
    // そのため meltExp_ だけ float で保持し、表示側(将来のHUD実装)で丸める想定。
    // ------------------------------------------------------------
    class gmWallet {
    public:
        // 資金を加算する(負の値を渡せば減算にも使える。今のところ加算のみで使う想定)
        void addFunds(int amount) {
            funds_ += amount;

#ifdef _DEBUG
            char buf[128];
            sprintf_s(buf, "addFunds: +%d (total=%d)\n", amount, funds_);
            OutputDebugStringA(buf);
#endif
        }
        int getFunds() const { return funds_; }

        // 溶かす経験値を加算する
        void addMeltExp(float amount) {
            meltExp_ += amount;

#ifdef _DEBUG
            char buf[128];
            sprintf_s(buf, "addMeltExp: +%.3f (total=%.3f)\n", amount, meltExp_);
            OutputDebugStringA(buf);
#endif
        }
        float getMeltExp() const { return meltExp_; }
        int getMeltExpAsInt() const { return static_cast<int>(meltExp_); } // 表示用

    private:
        int   funds_ = 0;
        float meltExp_ = 0.0f;
    };

}
