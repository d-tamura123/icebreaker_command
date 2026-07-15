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
#define DXE_FONT_TEXT_LOCK_GUARD_MUTEX()
#endif

	class FontTextResouce final : public dxe::ResourceHandle {
	private:
		FontTextResouce() = default;
	public:
		~FontTextResouce() ;

		// arg1... フォントサイズ
		// arg2... フォント名 ( 省略は "ＭＳ Ｐ明朝" )
		// arg3... クオリティ設定 ( DX_FONTTYPE_**** )
		// arg4... フォントの太さ ( 省略 は デフォルトの -1 )
		// arg5... 文字枠の太さ ( 省略は枠無し )
		// ret.... dxe::FontText の std::shared_ptr
		static Shared<FontTextResouce> Create
			( int32_t font_size
			, const std::string& font_name = "ＭＳ Ｐ明朝"
			, int32_t quality = -1
			, int32_t thick = -1
			, int32_t egge_size = -1);

		int32_t font_size_;
		int32_t quality_;	/* DX_DX_FONTTYPE_ */
		int32_t thick_;
		int32_t edge_size_;
		std::string font_name_;

	};


	class FontText final {
	private:
		FontText() = default;
	public:
		~FontText();

		// アニメーション無しテキストの作成
		static Shared<FontText> Create( const Shared<FontTextResouce>& resource );

		// アニメーションテキストの作成
		static Shared<FontText> CreateAnimation( const Shared<FontTextResouce>& resource );


		TNL_CALC_PROPERTY(tnl::Vector2f, Position, position_);
		TNL_PROPERTY(eRectOrigin, Location, location_);
		TNL_PROPERTY(int32_t, Color, color_);
		TNL_PROPERTY(int32_t, EdgeColor, edge_color_);
		TNL_SET_SELF_PROPERTY(std::string, String, base_text_);
		TNL_SET_SELF_PROPERTY(tnl::Vector2f, Scale, scale_);

		void draw();

		void updateAnimation();
		void playAnimation();
		void pauseAnimation();
		void restartAnimation();
		void finishAnimation();

		void addAnimTimeScale(float add_scale);
		void setAnimTimeScale(float time_scale);
		void setAnimWaitTime(float wait_time);
		tnl::SeekUnit::eCondition getAnimCondition();
		void setAnimUpdateCallback( const std::function<void()>& callback );

		Shared<FontText> createClone(const Shared<FontText>& origine);

	private:
		mutable std::mutex		mutex_;
		std::string				base_text_;
		std::string				anim_draw_text_;
		tnl::Vector2f			position_;
		tnl::Vector2f			scale_ = { 1, 1 };
		eRectOrigin				location_ = eRectOrigin::LEFT_TOP;
		Shared<FontTextResouce> resource_;
		int32_t					color_ = -1;
		int32_t					edge_color_ = -1;
		float					anim_wait_time_ = 0.1f;
		float					anim_time_scale_ = 1.0f;
		Shared<tnl::SeekUnit>	anim_seek_;
		std::function<void()>	anim_update_callback_;
		bool					is_extend_ = false;
	};

}


