#pragma once
#include <vector>
#include <memory>
#include <dxe.h>

namespace gm {

    // ------------------------------------------------------------
    // スポーン管理の共通(テンプレート)
    //
    // ・一定間隔でオブジェクトを生成する
    // ・リストで管理し、毎フレームupdate/renderし、
    //  死んだ個体をリストから取り除く」という共通処理だけをここに持つ。
    //
    // 「どこに・何を生成するか」は各派生クラス(氷山/NPC船/味方船など)が
    // trySpawn()に実装する。
    //
    // TEntityは gmObjectBase 派生クラスで、update(float)/render(camera)/
    // isAlive() を持っていることを前提とする。
    // ------------------------------------------------------------
    template<typename TEntity>
    class gmSpawnerManagerBase {
    public:
        virtual ~gmSpawnerManagerBase() = default;

        void update(float deltaTime)
        {
            // スポーンタイマー
            spawnTimer_ -= deltaTime;
            if (spawnTimer_ <= 0.0f && entities_.size() < maxEntities_) {
                trySpawn();
                spawnTimer_ = rollNextInterval();
            }

            // 各エンティティの更新 + 死亡個体の除去
            for (auto it = entities_.begin(); it != entities_.end(); ) {
                (*it)->update(deltaTime);

                if (!(*it)->isAlive()) {
                    it = entities_.erase(it);
                }
                else {
                    ++it;
                }
            }
        }

        void render(const Shared<dxe::Camera>& camera)
        {
            for (auto& e : entities_) {
                e->render(camera);
            }
        }

        size_t getEntityCount() const { return entities_.size(); }
        
        // 外部(ミニマップ等)から座標の参照
        const std::vector<std::shared_ptr<TEntity>>& getEntities() const { return entities_; }


    protected:
        // 生成条件(位置・パラメータの決定と生成そのもの)は派生クラスの責務
        virtual void trySpawn() = 0;

        // 次のスポーンまでの間隔(秒)。ランダム幅などは派生クラスが決める
        virtual float rollNextInterval() const = 0;

        std::vector<std::shared_ptr<TEntity>> entities_;
        float spawnTimer_ = 0.0f;
        size_t maxEntities_ = 30; // 同時存在数の上限(暴走防止)。派生クラスのコンストラクタで上書き可
    };
}
