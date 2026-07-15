#pragma once
#include "../tnl/tnl_util.h"
#include "../tnl/tnl_vector2f.h"
#include "../tnl/tnl_seek_unit.h"
#include "dxe_data.h"
#include "dxe_build_setting.h"

namespace dxe {

	/* マルチスレッドを利用するプロジェクトでは tnl_build_setting.h の TNL_BUILD_SWITCH_USE_MULTI_THREDING を true に設定してください */
#if DXE_BUILD_SWITCH_USE_MULTI_THREDING
#define DXE_FONT_TEXT_LOCK_GUARD_MUTEX() std::lock_guard<std::mutex> lock(mutex_);
#else
#define DXE_SPRITE_LOCK_GUARD_MUTEX()
#endif

	class SpriteResouce;
	class SpriteAnimationController;

	class SpriteDesc {
	public :
		SpriteDesc() = default;
		SpriteDesc(const std::string& file_path) : image_file_path_(file_path){}
		std::string image_file_path_;
	};
	class SpriteDivGraphDesc : public SpriteDesc {
	public :
		int32_t all_frame_num_ = 0;
		tnl::Vector2i frame_num_;
		tnl::Vector2i size_;
	};
	class SpriteAnimationDesc : public SpriteDivGraphDesc {
	public:
		float once_playing_time_;
		tnl::SeekUnit::ePlayMode play_mode_;
		tnl::SeekUnit::ePlayDir play_dir_ = tnl::SeekUnit::ePlayDir::FORWARD;
	};


	class Sprite final {
	private:
		Sprite() = default;
	public:
		~Sprite() = default;

		enum class eBlendMode {
			None = DX_BLENDMODE_NOBLEND,
			Alpha = DX_BLENDMODE_ALPHA,
			Alpha4 = DX_BLENDMODE_ALPHA_X4,
			Add = DX_BLENDMODE_ADD,
			Add4 = DX_BLENDMODE_ADD_X4,
			Sub = DX_BLENDMODE_SUB,
			Mul = DX_BLENDMODE_MUL
		};

		enum class eTransformMode {
			Simple,
			RotScale,
			RotLocateScaleXY,
		};

		enum class eAnimFrameFadeMode {
			None,
			Turn,
			Cross
		};

		enum class fImageReverse {
			None,
			X = 1 << 0,
			Y = 1 << 1
		};

		// アニメーション無しスプライトの作成
		static Shared<Sprite> Create( const Shared<SpriteResouce>& resource ) ;

		// アニメーション有りスプライトの作成
		static Shared<Sprite> CreateAnimation( const Shared<SpriteResouce>& resources );

		// リソースの取得
		Shared<SpriteResouce> getSpriteResource() { return resource_; }

		TNL_CALC_PROPERTY(tnl::Vector2f, Position, position_);
		TNL_CALC_PROPERTY(tnl::Vector2f, ScaleXY, scale_xy_);
		TNL_CALC_PROPERTY(float, Rotation, rotation_);
		TNL_CALC_PROPERTY(float, Alpha, alpha_);
		TNL_CALC_PROPERTY(float, Scale, scale_);
		TNL_CALC_PROPERTY(float, CameraFollowWeightX, camera_follow_weight_x_);
		TNL_CALC_PROPERTY(float, CameraFollowWeightY, camera_follow_weight_y_);

		TNL_PROPERTY(eBlendMode, BlendMode, blend_mode_);
		TNL_PROPERTY(eTransformMode, TransformMode, transform_mode_);
		TNL_SET_SELF_PROPERTY(eRectOrigin, Location, location_);
		TNL_PROPERTY(fImageReverse, ImageReverse, f_image_reverse_);
		void compImageReverse(fImageReverse flag);

		// XY 方向へ画像が反転されているか取得する
		bool isImageReverseX();
		bool isImageReverseY();

		// スプライトサイズの取得
		tnl::Vector2f getSize();

		// アニメーションのフレーム更新処理等の更新処理 ( アニメーションを再生するには毎フレーム実行する )
		void updateAnimation();
		// 一時停止中のアニメーションを停止位置から再生させる
		void playAnimation();
		// 一時停止
		void pauseAnimation();
		// 停止中のアニメーションを初めから再生させる
		void restartAnimation();
		// アニメーションを切り替える
		void changeAnimation(int32_t animation_index);
		// 次に再生するアニメーションの予約
		void reserveAnimation(int32_t animation_index);
		// 登録されたアニメーションの種類数を取得
		int32_t getAnimTypeNum();
		// 指定アニメーションのフレーム数を取得
		int32_t getAnimFrameNum(int32_t type_index);
		// 現在再生中のアニメーションを取得
		int32_t getAnimCurrentTypeIndex();
		// 現在再生中のアニメーションのフレーム番号を取得
		int32_t getAnimPlayingFrameIndex();

		// アニメーションの再生モードを設定する
		void setAnimPlayMode(tnl::SeekUnit::ePlayMode play_mode, tnl::SeekUnit::ePlayDir play_dir = tnl::SeekUnit::ePlayDir::FORWARD);

		// アニメーションの再生時間のスケーリング
		void addAnimTimeScale(float add_scale);
		void setAnimTimeScale(float time_scale);

		// アニメーションのフレーム間の表現設定
		void setAnimFrameFadeMode(eAnimFrameFadeMode frame_fade_mode );

		// アニメーションを指定のフレームに飛ばす
		void jumpAnimSeekFrame(int32_t frame);

		// デバッグ用ラインを描画するかどうか
		void setDebugLine( bool is_draw_line, int32_t color = -1 );

		// アニメーションの再生状態を取得する
		tnl::SeekUnit::eCondition getAnimCondition();

		// 描画
		void draw( const Shared<dxe::Camera>& camera = nullptr) ;


	private :
		mutable std::mutex		mutex_;
		tnl::Vector2f			position_;
		tnl::Vector2f			scale_xy_ = { 1, 1 };
		tnl::Vector2f			locate_position_ = { 0, 0 };
		float					rotation_ = 0;
		float					alpha_ = 1.0f;
		float					scale_ = 1.0f;
		float					camera_follow_weight_x_ = 1.0f;
		float					camera_follow_weight_y_ = 1.0f;
		int32_t					debug_line_color_ = -1;
		bool					is_debug_line_draw_ = false;
		eBlendMode				blend_mode_ = eBlendMode::None;
		eTransformMode			transform_mode_ = eTransformMode::RotLocateScaleXY;
		eRectOrigin				location_ = eRectOrigin::CENTER;
		fImageReverse			f_image_reverse_ = fImageReverse::None;
		Shared<SpriteResouce>   resource_;
		Shared<SpriteAnimationController> anim_ctrl_;
	};
	TNL_ENUM_CLASS_BIT_MASK_OPERATOR(Sprite::fImageReverse);


	//---------------------------------------------------------------------------------------------------------------------
	class SpriteResourceDivisionData {
	public:
		SpriteResourceDivisionData() = default;
		~SpriteResourceDivisionData() = default;
	private:
		friend class Sprite;
		friend class SpriteResouce;
		std::string file_path_;
		int32_t all_num_ = 1;
		int32_t x_num_ = 1;
		int32_t y_num_ = 1;
		int32_t x_size_ = 0;
		int32_t y_size_ = 0;
	};

	class SpriteAnimationSetting;
	class SpriteResouce final : public dxe::IResourceUnit {
	private:
		SpriteResouce() = default;
	public:
		~SpriteResouce();

		static Shared<dxe::ResourceHandle> LoadGraph(const std::string& file_path);

		static Shared<dxe::ResourceHandleArray> LoadDivGraph
		(const std::string& file_path
			, int32_t frame_num
			, int32_t x_num
			, int32_t y_num
			, int32_t x_size
			, int32_t y_size);

		static Shared<dxe::ResourceHandleArray> LoadDivGraph
		(const std::string& file_path
			, int32_t x_size
			, int32_t y_size
			, const std::vector<int32_t>& load_frame_indexs );


		static Shared<SpriteResouce> Create(const std::string& file_path);
		static Shared<SpriteResouce> Create(const Shared<dxe::Texture>& texture);
		static Shared<SpriteResouce> Create(const Shared<dxe::ResourceHandle>& resource_handle);
		static Shared<SpriteResouce> CreateAnimation();

		static Shared<SpriteResouce> CreateDivision
		(const std::string& file_path
			, int32_t frame_num
			, int32_t x_num
			, int32_t y_num
			, int32_t x_size
			, int32_t y_size);

		//static Shared<SpriteResouce> CreateDivision
		//(const Shared<dxe::Texture>& texture
		//	, int32_t frame_num
		//	, int32_t x_num
		//	, int32_t y_num
		//	, int32_t x_size
		//	, int32_t y_size);

		static Shared<SpriteResouce> CreateDivision
		(const Shared<dxe::ResourceHandleArray>& resource_handle
			, int32_t frame_num
			, int32_t x_num
			, int32_t y_num
			, int32_t x_size
			, int32_t y_size);

		static Shared<SpriteResouce> CreateDivision(const SpriteDivGraphDesc& desc);
		static Shared<SpriteResouce> CreateDivision
		(const Shared<dxe::ResourceHandleArray>& resource_handle, const SpriteDivGraphDesc& desc);

		int32_t addAnimation
		(const std::string& file_path
			, int32_t frame_num
			, int32_t x_num
			, int32_t y_num
			, int32_t x_size
			, int32_t y_size
			, float anim_total_time
			, tnl::SeekUnit::ePlayMode anim_play_mode
			, tnl::SeekUnit::ePlayDir anim_play_dir = tnl::SeekUnit::ePlayDir::FORWARD);

		int32_t addAnimation
		(const std::string& file_path
			, int32_t x_size
			, int32_t y_size
			, const std::vector<int32_t>& load_frame_indexs
			, float anim_total_time
			, tnl::SeekUnit::ePlayMode anim_play_mode
			, tnl::SeekUnit::ePlayDir anim_play_dir = tnl::SeekUnit::ePlayDir::FORWARD);

		int32_t addAnimation
		(const Shared<dxe::Texture>& texture
			, int32_t x_size
			, int32_t y_size
			, const std::vector<int32_t>& load_frame_indexs
			, float anim_total_time
			, tnl::SeekUnit::ePlayMode anim_play_mode
			, tnl::SeekUnit::ePlayDir anim_play_dir = tnl::SeekUnit::ePlayDir::FORWARD);

		int32_t addAnimation
		(const Shared<dxe::ResourceHandleArray>& resource_handle
			, int32_t frame_num
			, int32_t x_num
			, int32_t y_num
			, int32_t x_size
			, int32_t y_size
			, float anim_total_time
			, tnl::SeekUnit::ePlayMode anim_play_mode
			, tnl::SeekUnit::ePlayDir anim_play_dir = tnl::SeekUnit::ePlayDir::FORWARD);

		int32_t addAnimation
		(const Shared<dxe::ResourceHandleArray>& resource_handle
			, int32_t x_size
			, int32_t y_size
			, const std::vector<int32_t>& load_frame_indexs
			, float anim_total_time
			, tnl::SeekUnit::ePlayMode anim_play_mode
			, tnl::SeekUnit::ePlayDir anim_play_dir = tnl::SeekUnit::ePlayDir::FORWARD);


		int32_t addAnimation(const SpriteAnimationDesc& desc);
		int32_t addAnimation(const Shared<dxe::ResourceHandleArray>& resource_handle, const SpriteAnimationDesc& desc);

		//Shared<SpriteResouce> extraction( int32_t index ) ;

		bool isAnimation() { return is_animation_; }

		Shared<dxe::Texture> getDxeTexture(int32_t anim_index, int32_t frame_index) { 
			if (dxe_textures_.empty()) return nullptr;
			if (anim_index >= (int32_t)dxe_textures_.size()) return nullptr;
			if (frame_index >= dxe_textures_[anim_index].size()) return nullptr;
			return dxe_textures_[anim_index][frame_index]; 
		}

		float getAnimationPlayTime(int32_t anim_index);
		tnl::SeekUnit::ePlayMode getAnimationPlayMode(int32_t anim_index);
		tnl::SeekUnit::ePlayDir getAnimationPlayDir(int32_t anim_index);

	private:
		friend class Sprite;
		bool is_animation_ = false;
		std::vector< SpriteAnimationSetting > animation_settings_;
		std::vector< SpriteResourceDivisionData > divisions_;
		std::vector< Shared<dxe::ResourceHandleArray> > handle_array_table_;
		std::vector<std::vector< Shared<dxe::Texture> >> dxe_textures_; // [ アニメーション単位 ][ フレーム単位 ]
		void addDivisionProcess(const std::string& file_path, int32_t frame_num, int32_t x_num, int32_t y_num, int32_t x_size, int32_t y_size);
		void addDivisionProcess(const std::string& file_path, int32_t x_size, int32_t y_size, const std::vector<int32_t>& load_frame_indexs );

		void addDivisionProcess(const Shared<dxe::Texture>& texture, int32_t frame_size_w, int32_t frame_size_h);
		void addDivisionProcess(const Shared<dxe::Texture>& texture, int32_t frame_size_w, int32_t frame_size_h, const std::vector<int32_t>& load_frame_indexs);

		void addDivisionProcess(const Shared<dxe::ResourceHandleArray>& resource_handle, int32_t frame_num, int32_t x_num, int32_t y_num, int32_t x_size, int32_t y_size);
		void addDivisionProcess(const Shared<dxe::ResourceHandleArray>& resource_handle, int32_t x_size, int32_t y_size, const std::vector<int32_t>& load_frame_indexs);

	};

	class SpriteAnimationSetting {
	public:
		SpriteAnimationSetting() = default;
		~SpriteAnimationSetting() = default;
	private:
		friend class Sprite;
		friend class SpriteResouce;
		float total_anim_time_ = 1.0f;
		tnl::SeekUnit::ePlayMode anim_play_mode_ = tnl::SeekUnit::ePlayMode::REPEAT;
		tnl::SeekUnit::ePlayDir anim_play_dir_ = tnl::SeekUnit::ePlayDir::FORWARD;
	};

	class SpriteAnimationController {
	public:
		SpriteAnimationController() = default;
		~SpriteAnimationController() = default;

		bool update() ;
		void play();
		void pause();
		void restart();
		void reserve(int32_t reserve_type_index);
		void addSeekUnit(const Shared<tnl::SeekUnit>& seek);
		void setPlayMode(tnl::SeekUnit::ePlayMode play_mode, tnl::SeekUnit::ePlayDir play_dir = tnl::SeekUnit::ePlayDir::FORWARD);
		void addTimeScale(float add_scale);
		void jumpSeekFrame(int32_t frame);
		tnl::SeekUnit::eCondition getAnimCondition();
		int32_t getPlayingFrameIndex();
		int32_t getTypeNum();
		int32_t getFrameNum(int32_t type_index);

		TNL_PROPERTY(int32_t, BeforeImageHandle, before_image_handle_);
		TNL_PROPERTY(float, BeforeFadeAlpha, before_fade_alpha_);
		TNL_PROPERTY(float, CurrentFadeAlpha, current_fade_alpha_);
		TNL_PROPERTY(Sprite::eAnimFrameFadeMode, FrameFadeMode, frame_fade_mode_);
		TNL_SET_SELF_PROPERTY(float, TimeScale, time_scale_);
		TNL_SET_SELF_PROPERTY(int32_t, CurrentTypeIndex, type_index_);


	private:
		int32_t					type_index_ = 0;
		int32_t					frame_index_ = 0;
		int32_t					reserve_index_ = 0;
		int32_t					before_type_index_ = 0;
		int32_t					before_image_handle_ = 0;
		float					before_fade_alpha_ = 1.0f;
		float					current_fade_alpha_ = 0.0;
		float					time_scale_ = 1.0f;
		Sprite::eAnimFrameFadeMode	frame_fade_mode_ = Sprite::eAnimFrameFadeMode::None;
		std::vector<Shared<tnl::SeekUnit>> seeks_;
	};



}

