// gmGameConfig.h
#pragma once
#include <cstdint>      // uint8_t
#include <cstddef>      // size_t

namespace gm
{
    // マップサイズ（Excel側と完全一致）
    static const int MAP_CHIP_WIDTH     = 256;
    static const int MAP_CHIP_HEIGHT    = 256;

    // ゲーム世界座標系での1セル(チップ)の大きさ
    static const float CELL_SIZE = 100.0f;

    // マップデータのパス
    static const char* const MAP_FILE_PATH = "resource/map/map.bin";

    // 海流マップデータのパス
    static const char* const FLOW_STO_E_PATH = "resource/map/ocean_flow_StoE.bin";
    static const char* const FLOW_STO_N_PATH = "resource/map/ocean_flow_StoN.bin";
    static const char* const FLOW_STO_W_PATH = "resource/map/ocean_flow_StoW.bin";

    // 航路データのパス(プレフィックス)
    // "route_1.bin", "route_2.bin", ... のように1始まりの連番を後ろに付けて読み込む。
    // 何本の航路が存在するかを別途持たず、
    // このプレフィックス+連番で読み込みに失敗するまで走査する方式
    // (gmMapManager::LoadRoutes()参照)。
    static const char* const ROUTE_FILE_PATH_PREFIX = "resource/map/route_";

    // 描画範囲の定義
    static const float RENDER_DISTANCE      = 6400.0f;                              // カメラからの描画距離
    static const float RENDER_DISTANCE_SQ   = RENDER_DISTANCE * RENDER_DISTANCE;    // 距離比較を高速化するための2乗値
                                                                                    // ※ 距離の比較は本来sqrt()が必要だが、
                                                                                    //    両辺を2乗しても大小関係は変わらないため、
                                                                                    //    平方根の計算を省略できる

    // 画像ファイルパス
    static const char* const GRAPHICS_FILE_PATH__OCEAN_FLOW_ARROW       = "resource/graphics/test/wf_arrow.png";

    // VFX(エフェクト)関連パス
    static const char* const VFX_SPRITE_METADATA_CSV_PATH               = "resource/csv/tktk_sprite_metadata.csv";
    static const char* const VFX_EFFECT_GRAPHICS_DIR                    = "resource/graphics/effect/";

    // 武器関連パス
    static const char* const GRAPHICS_FILE_PATH__SPLIT_PROJECTILE_BALL  = "resource/graphics/weapon/split_projectile_ball.png"; // 割り砲弾(鉄球)用テクスチャ。黒単色を想定



    // ------------------------------------------------------------
    // 航路の可視化(NPC交易船が辿る航路を、ゲーム内にリボンメッシュとして描画する)
    // ------------------------------------------------------------
    // 素材画像(横長のグラデーション。縦方向(V)が中央最明→上下端でアルファ0、
    // 横方向(U)は無地でよい。gmRouteVisualizer参照)
    static const char* const GRAPHICS_FILE_PATH__ROUTE_RIBBON = "resource/graphics/route/route_ribbon_gradient.png";

    // 空中リボン(水平・垂直の十字クロス)を、海面(y=0)からどれだけ高い位置に固定するか。
    // 波の高さは追わず常に一定(モノレールのレールのように)。
    static const float ROUTE_RIBBON_ALTITUDE = 25.0f;

    // 空中リボン(水平・垂直とも共通)の帯の幅(world単位)
    static const float ROUTE_RIBBON_WIDTH = 60.0f;

    // 中心線(遠心的Catmull-Romで補間した曲線)をポリライン化する際のサンプリング間隔(world単位)。
    // 小さいほど滑らかになるが、頂点数・ドローコール準備コストが増える。
    inline constexpr float ROUTE_RIBBON_SAMPLE_STEP = 25.0f;

    // 描画カリング用にチャンク分割する際の、1チャンクあたりの目安の長さ(world単位)。
    // gmIceberg等と同じ距離カリング(RENDER_DISTANCE_SQ)をチャンク単位で適用する。
    static const float ROUTE_RIBBON_CHUNK_LENGTH = 700.0f;

    // 始点(S)・終点(G)付近でアルファを0まで滑らかにフェードさせる区間の長さ(world単位)
    static const float ROUTE_RIBBON_FADE_LENGTH = 150.0f;

    // UVスクロールでテクスチャが1周する距離(world単位)。値が小さいほど帯の模様が細かく繰り返す。
    static const float ROUTE_RIBBON_UV_REPEAT_LENGTH = 400.0f;

    // UVスクロール速度(1秒あたりに進むUV量。ROUTE_RIBBON_UV_REPEAT_LENGTHに対する割合)。
    // 各航路はS→Gの一方向のみなので、この方向へ流れる=「今どちら向きの航路か」を示す意味を持つ。
    // 0を指定するとスクロールなし(静止した帯)になる。
    static const float ROUTE_RIBBON_SCROLL_SPEED = 0.35f;

    // 海面の影用リボン(位置確認用、水平のみ・空中リボンより控えめな見た目)関連
    static const float   ROUTE_SHADOW_RIBBON_HEIGHT_OFFSET  = 2.0f;   // 水面メッシュとのZファイティング回避用の微小オフセット(海面y=0からの高さ)
    static const float   ROUTE_SHADOW_RIBBON_WIDTH_SCALE    = 0.4f;   // 空中リボン幅に対する、影リボンの幅の比率
    static const uint8_t ROUTE_SHADOW_RIBBON_ALPHA_SCALE    = 90;     // 影リボンの最大アルファ(0〜255。空中リボンより控えめにする)
    static const uint8_t ROUTE_SHADOW_RIBBON_COLOR_SCALE    = 140;    // 影リボンの頂点色に掛ける明度スケール(0〜255。暗めにする)

    // ------------------------------------------------------------
    // マップの外枠(移動可能範囲の境界)可視化(gmMapBoundaryVisualizer)。
    // 生成ロジックはgmRouteVisualizerと共通だが、以下が異なる:
    //   ・中心線はマップの四辺(直線)を一定間隔でサンプリングした閉ループ
    //   ・閉ループのため始点・終点のアルファフェードは行わない(常に不透明)
    //   ・海面の影リボンは無し(水平・垂直の2層のみ)
    //   ・専用の青系テクスチャを使う(航路のゴールドと視覚的に区別するため)
    // 見た目のスケール感(高さ・太さ・サンプリング間隔・チャンク長・UV関連)は、
    // 航路のリボンと統一するためROUTE_RIBBON_*の値をそのまま流用する。
    // ------------------------------------------------------------
    static const char* const GRAPHICS_FILE_PATH__MAP_BOUNDARY_RIBBON = "resource/graphics/route/outerframe_ribbon_gradient.png";

    // プレイヤー船がマップ外へ出ないようにする際、マップの端からどれだけ内側で止めるか(world単位)。
    // 船体のサイズぶんの余裕を持たせ、船が外枠を視覚的にはみ出さないようにするための簡易的な値
    // (厳密なコライダーサイズは使わず、固定値で十分と判断)。
    static const float MAP_BOUNDARY_MARGIN = 60.0f;

    // ------------------------------------------------------------
    // NPC交易船(gmTradeShip / gmTradeShipManager)
    // ------------------------------------------------------------
    // モデル・テクスチャのパス。
    // NOTE: S2〜S10等の専用モデルに差し替え予定だが、変換(mv1化)が済むまでの暫定として
    //       プレイヤー船と同じS1.mv1を使う(色味だけTRADE_SHIP_TINT_COLORで変える)。
    //       差し替える際はこの2つのパスを書き換えるだけでよい。
    static const char* const TRADE_SHIP_MESH_FILE_PATH          = "resource/mesh/mv/ship/TradeShip.mv1";
    static const char* const TRADE_SHIP_TEXTURE_FILE_PATH       = "resource/graphics/ship/tradeship/Textures.png";
    static const char* const TRADE_SHIP_NORMAL_MAP_FILE_PATH    = "resource/graphics/ship/tradeship/Normals.png";

    static const float       TRADE_SHIP_MESH_SCALE          = 0.01f;

    // プレイヤー船と見分けやすいよう、頂点ディフューズカラーで色味を変える(交易船=金のコンセプト)。
    // dxe::Mesh::setMtrlDiffuse()へtnl::Vector3(R,G,B)として渡す想定(0.0〜1.0)。
    // gmGameConfig.hは軽量に保ちたいため、ここではtnl::Vector3型を使わずfloat 3つに分けている。
    static const float TRADE_SHIP_TINT_COLOR_R = 1.0f;
    static const float TRADE_SHIP_TINT_COLOR_G = 0.85f;
    static const float TRADE_SHIP_TINT_COLOR_B = 0.5f;

    // コライダー(setupManualColliderへそのまま渡す値)。
    static const float TRADE_SHIP_COLLIDER_RADIUS = 40.0f;
    static const float TRADE_SHIP_COLLIDER_LENGTH = 120.0f;

    // 巡航速度(dynamics_.targetSpeedにそのまま渡す。gmShip::SPEED_LEVELSと同じ単位系)
    static const float TRADE_SHIP_CRUISE_SPEED = 0.4f;

    // 操舵AI: 現在位置から中心線上でどれだけ先の点を「狙う点」にするか(world単位)。
    // 大きいほど穏やかに、小さいほど機敏に(ただし曲がりきれず外側にカットしやすく)なる。
    static const float TRADE_SHIP_LOOKAHEAD_DISTANCE = 250.0f;

    // 操舵AI: 狙う点への角度差(ラジアン)からtargetRudderへ変換する際のゲイン。
    // 角度差×このゲイン をそのまま-1〜1にクランプしてtargetRudderにする。
    static const float TRADE_SHIP_STEERING_GAIN = 1.2f;

    // 終点(G)までの残り距離がこれ未満になったら到達とみなし、デスポーンする(world単位)
    static const float TRADE_SHIP_ARRIVAL_THRESHOLD = 80.0f;

    // 進捗停滞タイムアウト+ワープの保険。
    // 経路上の進捗(累積距離のハイウォーターマーク)がこの秒数だけ更新されなければ、
    // 最寄りの中心線上の点(＋少し先)へワープし、向きも補正する。
    static const float TRADE_SHIP_STUCK_TIMEOUT = 90.0f;
    // ワープ先を、停滞検知位置からさらにこれだけ先に進めておく(同じ地点で即再発するのを防ぐ)
    static const float TRADE_SHIP_WARP_ADVANCE_DISTANCE = 150.0f;

    // 島衝突時の一時バック挙動。SPEED_LEVELSの負の段階(後退)を使う。
    static const float TRADE_SHIP_COLLISION_BACKOFF_SPEED = -0.5f;      // バック中のtargetSpeed(SPEED_LEVELS[1]相当)
    static const float TRADE_SHIP_COLLISION_BACKOFF_DURATION = 2.0f;    // バックを続ける秒数

    // 交易船スポナー
    static const size_t TRADE_SHIP_MAX_ENTITIES = 6;            // 同時に存在できる交易船の上限(暴走防止)
    static const float  TRADE_SHIP_SPAWN_INTERVAL_MIN = 20.0f;  // 秒
    static const float  TRADE_SHIP_SPAWN_INTERVAL_MAX = 40.0f;



    // ------------------------------------------------------------
    // 船のHP・被弾・撃沈演出(gmShip共通。プレイヤー船・交易船の両方に効く)
    // ------------------------------------------------------------
    static const float SHIP_MAX_HP = 100.0f;

    // 流氷との接触ダメージ。
    // 「初回接触(または猶予明け後の再接触)は大ダメージ、以後接触し続けている間は
    //  小さな継続ダメージ(DoT)」というモデル。
    // 氷山インスタンスごとに猶予タイマーを持つため、
    // 短時間で接触→離脱→再接触を繰り返しても大ダメージが連発しない
    static const float SHIP_ICEBERG_BIG_HIT_DAMAGE      = 25.0f;    // 初回(または猶予明け)接触時の大ダメージ
    static const float SHIP_ICEBERG_DOT_DAMAGE_PER_SEC  = 5.0f;     // 接触し続けている間の継続ダメージ(秒あたり)
    static const float SHIP_ICEBERG_CONTACT_GRACE_SEC   = 3.0f;     // 大ダメージ後、同じ氷山からの大ダメージを抑止する猶予秒数

    // 撃沈演出(Destroyed状態)。
    // 「垂直方向に傾きながら沈み、完全に水面下へ潜ることで消えたとみなす」という演出を
    // gmShip::updateDestroyed()で共通実装する。
    // 傾き・沈み込みとも、経過時間の割合(t)をイーズイン(t^EASE_POWER)してから適用することで、
    // 始めはゆっくり、徐々に加速していく動きにしている。
    static const float SHIP_DESTROYED_TILT_TARGET_RAD   = 0.9f;   // 最終的な傾き角度(ラジアン。約51度)
    static const float SHIP_DESTROYED_SUBMERGE_DEPTH    = 150.0f; // 死亡時のY座標から、これだけ沈んだら完全に消えたとみなす(world単位)
    static const float SHIP_DESTROYED_DURATION          = 5.0f;   // 演出全体の尺(秒)。この時間でTILT_TARGET/SUBMERGE_DEPTHに到達する
    static const float SHIP_DESTROYED_EASE_POWER        = 2.0f;   // イーズインの強さ(t^EASE_POWER)。大きいほど始めがよりゆっくりになる

    // 傾きは全体の前半で完了させ、沈み込みは傾きが終わりかけた頃から始めて
    // 全体の終わりで完了する、という時間差を付ける(SHIP_DESTROYED_DURATIONに対する割合で指定)。
    // 例: TILT_PORTION=0.6, SINK_START_PORTION=0.4なら、傾きは0〜60%の区間で完了し、
    // 沈み込みは40%〜100%の区間で進む(40〜60%の間は両方同時に進む重なり区間)。
    static const float SHIP_DESTROYED_TILT_PORTION = 0.6f;
    static const float SHIP_DESTROYED_SINK_START_PORTION = 0.25f;

    
    // ------------------------------------------------------------
    // ウォレット(資金・溶かす経験値)関連
    // ------------------------------------------------------------
    // 交易船が終点(G)へ満タンHPで到着した場合の資金報酬(基準値)。
    // 実際の報酬 = この値 × (到着時のHP ÷ 最大HP)。ダメージを受けているほど減額される。
    static const int TRADE_SHIP_ARRIVAL_REWARD_BASE = 1000;

    // 流氷への溶かすダメージ1ポイントあたりに得られる、溶かす経験値の量。
    // 1.0でダメージ量とそのまま等倍(通常弾・炎放射どちらのダメージにも同じ比率で適用される)。
    static const float MELT_EXP_PER_DAMAGE_POINT = 1.0f;



    // ------------------------------------------------------------
    // プレイヤーカメラ(gmPlayerCameraController)
    // ------------------------------------------------------------
    // ズーム(マウスホイールで0.0〜1.0に蓄積)のうち、この値未満なら周回モード、
    // 以上ならエイムモードとして扱う境界値。
    static const float CAMERA_ZOOM_AIM_THRESHOLD = 0.1f;

    // マウスホイールのノッチ1回あたりに、zoomRatio_を増減させる量。
    static const float CAMERA_ZOOM_STEP_PER_WHEEL_NOTCH = 0.05f;

    // 周回モード: カメラ〜プレイヤー船の距離(world単位)。
    // zoomRatio_ = 0 でDIST_MAX、CAMERA_ZOOM_AIM_THRESHOLDでDIST_MINになるよう線形補間する。
    static const float CAMERA_ORBIT_DIST_MAX = 500.0f;
    static const float CAMERA_ORBIT_DIST_MIN = 200.0f;

    // 周回モード: カメラ高さ(y座標)の下限・上限(world単位)。
    // 海面(y=0)に潜らない下限と、船を真上から見下ろしすぎない上限。
    static const float CAMERA_ORBIT_HEIGHT_MIN = 30.0f;
    static const float CAMERA_ORBIT_HEIGHT_MAX = 900.0f;

    // 周回モード: マウス移動量に対する視点回転の感度(gmKyleFreeCameraControllerと同じ値を採用)
    static const float CAMERA_ORBIT_MOUSE_SENSITIVITY = 0.005f;

    // 周回モード: pitch(見下ろし角。負値=見下ろす方向)の可動範囲(ラジアン)
    static const float CAMERA_ORBIT_PITCH_MIN = -1.4f;      // ほぼ真上から見下ろす
    static const float CAMERA_ORBIT_PITCH_MAX = -0.6f;      // 錯覚で水平に近づくと船と照準の距離が縮んだようにみえるのでその直前あたりで制限する(目視で調整した値)

    // 周回モード: 狙い先(=カメラの注視点。船からの水平距離)の範囲(world単位)。
    // pitch(見下ろし角)をCAMERA_ORBIT_PITCH_MIN〜MAXの範囲で0.0〜1.0に正規化し、
    // その割合でMIN〜MAXへ線形補間する(急角度=MIN側の近距離、水平に近い=MAX側の遠距離)。
    //
    // 武器の最大射程(WEAPON_*_MAX_RANGE)は使わず、固定値にしている。
    // 理由: 溶かす弾/割る弾の射程(1500)をそのまま使うと、周回モードのままでも遠くを狙えてしまい、
    // ズームインしてエイムモードに入る意味が薄れる。逆に火炎放射の射程(200)をそのまま使うと、
    // 周回モードの可動域が極端に狭くなってしまう。武器種によって周回モードの操作感(距離の可動域)が
    // 変わるのは操作性として不自然なため、武器に依存しない固定範囲にする
    // (ただし実際の攻撃対象は、別途武器の最大射程でクランプする。
    //  gmPlayerCameraController::updateOrbitMode()参照)。
    static const float CAMERA_ORBIT_AIM_DIST_MIN = 10.0f;   // 暫定値。テストで調整
    static const float CAMERA_ORBIT_AIM_DIST_MAX = 600.0f;  // 暫定値。テストで調整


    // エイムモード: カメラ位置をプレイヤー船の位置からどれだけ高く持ち上げるか(world単位)。
    // 文字通り船の原点(y=0)にカメラを置くと、海面(y=0)へのレイキャストが
    // ほぼ水平・数値的に不安定になるため、見張り台程度の高さを持たせている。
    static const float CAMERA_AIM_HEIGHT_OFFSET = 100.0f;

    // エイムモード: マウス移動量に対する視点回転の感度(周回モードよりやや繊細にしている)
    static const float CAMERA_AIM_MOUSE_SENSITIVITY = 0.003f;

    // エイムモード: pitchの可動範囲(ラジアン)
    static const float CAMERA_AIM_PITCH_MIN = -1.0f;    // 自船を消しているのでその矛盾を映さない相当の角度
    static const float CAMERA_AIM_PITCH_MAX = -0.02f;   // ほぼ水平(0だと海面と平行になり狙い先が求まらないため僅かに残す)

    // エイムモード: 攻撃の最大射程(world単位)。狙い先(海面上の点)をこの距離までに制限する。
    static const float WEAPON_PROJECTILE_MAX_RANGE      = 1500.0f;  // 溶かす弾・割る弾で共有(軌道関連の値が同じため)
    static const float WEAPON_FLAMETHROWER_MAX_RANGE    = 200.0f;   // 火炎放射

    // エイムモード: ズーム値(閾値〜1.0)に応じて狭めるカメラの画角(度)。
    // 閾値でFOV_WIDE、zoomRatio_=1.0でFOV_NARROWになるよう線形補間する。
    static const float CAMERA_AIM_FOV_WIDE_DEG = 50.0f;
    static const float CAMERA_AIM_FOV_NARROW_DEG = 15.0f;



    // ------------------------------------------------------------
    // 武器選択(gmWeaponSelectionState)・リカバリ
    // フェーズ1.4仕様。値はすべて暫定(テストで調整する前提。クールタイムのみ確定値)。
    // ------------------------------------------------------------
    // 各武器のリキャストタイム(秒)。溶かす弾・割る弾は軌道関連の値が同じなので同じ値にしている。
    static const float WEAPON_MELT_BULLET_RECAST_SEC    = 6.0f;
    static const float WEAPON_BREAK_BULLET_RECAST_SEC   = 6.0f;
    static const float WEAPON_FLAMETHROWER_RECAST_SEC   = 3.0f;

    // リカバリ(5キー): 1回の発動で回復する総量(最大HPに対する割合)
    static const float RECOVERY_HEAL_RATIO              = 0.3f;         // 1回のリカバリ操作で３割回復
    // リカバリ: 回復総量を、何秒かけてじわじわ回復するか
    static const float RECOVERY_HEAL_DURATION_SEC       = 5.0f;
    // リカバリ: クールタイム(秒)
    static const float RECOVERY_COOLDOWN_SEC            = 90.0f;

    

    // ------------------------------------------------------------
    // サウンド(gmSoundManager)
    // パン(左右)のみで簡易的な位置表現をおこなう。距離は音量減衰のみで表現する。
    // いずれも「発火した瞬間に1回だけ計算し、以後は再生中でも更新しない」仕様
    // (再生中もリアルタイムに追従させると、カメラを振った時に耳障りになりうるため)。
    // ------------------------------------------------------------
    // パン: この距離以内は中央(パン無効)として扱う。近距離での耳障りな左右の揺れを防ぐ。
    static const float SOUND_PAN_DEADZONE_DIST = 300.0f;
    // パン: この距離で最大パン値(SOUND_PAN_MAX_VALUE)に達する。
    static const float SOUND_PAN_MAX_DIST = 2000.0f;
    // パン: 最大値(DxLibの仕様上は±255まで指定できるが、完全に片耳へ寄り切ると
    // 耳障りなため、少し余裕を持たせて抑えている)。
    static const int   SOUND_PAN_MAX_VALUE = 180;

    // 音量: この距離で最小音量(SOUND_VOLUME_MIN)まで減衰する。
    static const float SOUND_VOLUME_MAX_DIST = 3000.0f;
    static const int   SOUND_VOLUME_MIN = 40;   // 遠距離でも完全に無音にはしない
    static const int   SOUND_VOLUME_MAX = 255;

    // 同時発音数の制限: 同じ名前のSEを、同時にこの数までしか鳴らさない
    // (流氷の密集地帯をまとめて割った場合等、同じ音が過剰に重なるのを防ぐ)。
    static const int   SOUND_MAX_CONCURRENT_SAME_SE = 4;

    // BGMフェードインのデフォルト所要時間(秒)。タイトル画面等、演出として徐々に音量を
    // 上げたい場面で使う。
    static const float SOUND_BGM_FADE_IN_DURATION_SEC = 6.0f;
}
