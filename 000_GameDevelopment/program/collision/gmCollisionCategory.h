#pragma once


namespace gm {

    // ------------------------------------------------------------
    // 衝突判定用のカテゴリ分け
    // ------------------------------------------------------------
    enum class gmCollisionCategory {
        None,       // 衝突判定の対象外
        Ship,       // 船(プレイヤー/NPC問わず、今のところ区別しない)
        Island,     // 島
        Iceberg,    // 流氷
        Projectile, // (将来)砲弾・火炎などプレイヤーの攻撃手段
    };

    // ------------------------------------------------------------
    // カテゴリの組み合わせごとに「そもそも判定するかどうか」を決めるルール表。
    // Unityでいう「Layer Collision Matrix」に相当するもの。
    // 詳細は設計メモ資料に記載している。
    //
    // 【一覧(true=判定する / false=判定しない)】
    //                Ship    Island  Iceberg Projectile
    //   Ship          true    true    true    false
    //   Island        true    false   false   false
    //   Iceberg       true    false   true    true
    //   Projectile    false   false   true    false
    //
    //   Ship-Ship      : true  (船同士は衝突し、移動抑止したい)
    //   Ship-Island    : true  (座礁として移動抑止したい)
    //   Ship-Iceberg   : true  (座礁として移動抑止したい)
    //   Ship-Projectile: false (攻撃は流氷のみが対象。船への攻撃は意味をなさない)
    //   Island-Island  : false (通常起こりえないケース)
    //   Island-Iceberg : false (海流ベクトルの定義上ほぼ起こらないが、念のためOFF)
    //   Island-Projectile: false (攻撃は流氷のみが対象。島への攻撃は意味をなさない)
    //   Iceberg-Iceberg: true  (起こりにくいが、起きた場合は相応の相互作用を持たせたい)
    //   Iceberg-Projectile: true (砲弾/火炎で流氷を溶かす、本作の中心的な攻撃手段)
    //   Projectile-Projectile: false (砲弾同士が当たっても意味をなさない)
    //
    //   ※ None が絡む組み合わせは全てfalse(カテゴリ未設定＝判定対象外という安全側の既定動作)
    // ------------------------------------------------------------
    inline bool CanCollide(gmCollisionCategory a, gmCollisionCategory b)
    {
        using C = gmCollisionCategory;

        // enumの並び順(None, Ship, Island, Iceberg, Projectile)に対応した5x5の表。
        // 行=a, 列=b として引く。Noneの行・列は全てfalseにしてある。
        static constexpr bool matrix[5][5] = {
            /*                None,   Ship,   Island, Iceberg, Projectile */
            /* None       */ { false, false,  false,  false,   false },
            /* Ship       */ { false, true,   true,   true,    false },
            /* Island     */ { false, true,   false,  false,   false },
            /* Iceberg    */ { false, true,   false,  true,    true  },
            /* Projectile */ { false, false,  false,  true,    false },
        };

        return matrix[static_cast<int>(a)][static_cast<int>(b)];
    }
}
