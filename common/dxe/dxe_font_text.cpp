#pragma warning(disable:4819)
#include "DxLib.h"
#include "dxe_font_text.h"
#include "dxe_prototype.h"
#include "dxe_build_setting.h"

namespace dxe {

	DXE_PROTOTYPE_GET_DELTA_TIME;
	DXE_PROTOTYPE_GET_RECT_ORIGIN_LOCATION;
	DXE_PROTOTYPE_GET_FONT_PATH_FROM_NAME;
	DXE_PROTOTYPE_IS_ENABLE_FONT_PATH;


	//--------------------------------------------------------------------------------------------------------------------------
	Shared<FontTextResouce> FontTextResouce::Create
		( int32_t font_size
		, const std::string& font_name
		, int32_t quality
		, int32_t thick
		, int32_t edge_size){

		if (!dxe::IsEnableFontPath(font_name.c_str())) {
			throw std::runtime_error(" dxe::CreateFontHandle font path not found error ");
		}

		Shared<FontTextResouce> resource = std::shared_ptr<FontTextResouce>( new FontTextResouce() ) ;
		resource->font_name_ = font_name;
		resource->font_size_ = font_size;
		resource->quality_ = quality;
		resource->thick_ = thick;
		resource->edge_size_ = edge_size;

		int handle = CreateFontToHandle(font_name.c_str(), font_size, thick, quality, -1, edge_size);

		resource->setDxlibHandle( handle );
		return resource;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	FontTextResouce::~FontTextResouce() {
		DeleteFontToHandle(getDxlibHandle());
	}

	//--------------------------------------------------------------------------------------------------------------------------
	Shared<FontText> FontText::Create(const Shared<FontTextResouce>& resource) {
		Shared<FontText> text = std::shared_ptr<FontText>(new FontText());
		text->resource_ = resource;
		return text;
	}
	//--------------------------------------------------------------------------------------------------------------------------
	Shared<FontText> FontText::CreateAnimation(const Shared<FontTextResouce>& resource) {
		Shared<FontText> text = std::shared_ptr<FontText>(new FontText());
		text->resource_ = resource;
		text->anim_seek_ = tnl::SeekUnit::Create(DXE_FIX_FPS, 0, 3.0f, tnl::SeekUnit::ePlayMode::SINGLE, tnl::SeekUnit::ePlayDir::FORWARD);
		return text;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	FontText::~FontText() {}

	//--------------------------------------------------------------------------------------------------------------------------
	Shared<FontText> FontText::createClone(const Shared<FontText>& origine) {
		Shared<FontText> text = std::shared_ptr<FontText>(new FontText());
		text->base_text_ = origine->base_text_;
		text->location_ = origine->location_;
		text->position_ = origine->position_;
		text->scale_ = origine->scale_;
		text->color_ = origine->color_;
		text->edge_color_ = origine->edge_color_;
		text->resource_ = origine->resource_;
		text->is_extend_ = origine->is_extend_;

		if (origine->anim_seek_) {
			text->anim_wait_time_ = origine->anim_wait_time_;
			text->anim_time_scale_ = origine->anim_time_scale_;
			text->anim_seek_ = tnl::SeekUnit::Create(DXE_FIX_FPS, 0, 3.0f, tnl::SeekUnit::ePlayMode::SINGLE, tnl::SeekUnit::ePlayDir::FORWARD);
			text->anim_seek_->setTimeScale(origine->anim_time_scale_);
			text->base_text_ = origine->base_text_;
			int32_t length = tnl::GetSJisLength(base_text_);
			text->anim_seek_->setTotalTime(static_cast<float>(length) * anim_wait_time_);
			text->anim_update_callback_ = origine->anim_update_callback_;
		}

		return text;
	}


	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::draw() {

		tnl::Vector2f drawpos = position_;
		int size_x, size_y, line_count;
		GetDrawStringSizeToHandle(&size_x, &size_y, &line_count, base_text_.c_str(), (int)base_text_.length(), resource_->getDxlibHandle());
		GetRectOriginLocatePosition(drawpos.x, drawpos.y, (int)(size_x * scale_.x), (int)(size_y * scale_.y), location_);

		if (anim_seek_) {
			if (is_extend_) DrawExtendStringFToHandle(drawpos.x, drawpos.y, scale_.x, scale_.y, anim_draw_text_.c_str(), color_, resource_->getDxlibHandle(), edge_color_);
			else			DrawStringFToHandle(drawpos.x, drawpos.y, anim_draw_text_.c_str(), color_, resource_->getDxlibHandle(), edge_color_);
		}
		else {
			if (is_extend_) DrawExtendStringFToHandle(drawpos.x, drawpos.y, scale_.x, scale_.y, base_text_.c_str(), color_, resource_->getDxlibHandle(), edge_color_);
			else			DrawStringFToHandle(drawpos.x, drawpos.y, base_text_.c_str(), color_, resource_->getDxlibHandle(), edge_color_);
		}

		//DrawCircle(position_.x, position_.y, 3, GetColor(255, 0, 0));
	}


	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::setString(const std::string& base_text) {
		{
			DXE_FONT_TEXT_LOCK_GUARD_MUTEX();
			base_text_ = base_text;
		}
		if (!anim_seek_) return;
		int32_t length = tnl::GetSJisLength(base_text_);
		anim_seek_->setTotalTime(static_cast<float>(length) * anim_wait_time_);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::setScale(const tnl::Vector2f& value) {
		DXE_FONT_TEXT_LOCK_GUARD_MUTEX();
		scale_ = value;
		is_extend_ = true;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::setAnimWaitTime(float wait_time) {
		if (!anim_seek_) return;
		{
			DXE_FONT_TEXT_LOCK_GUARD_MUTEX();
			anim_wait_time_ = wait_time;
		}
		if (base_text_.empty()) return;
		int32_t length = tnl::GetSJisLength(base_text_);
		anim_seek_->setTotalTime( static_cast<float>(length) * anim_wait_time_ );
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::setAnimTimeScale(float time_scale) {
		if (!anim_seek_) return;
		{
			DXE_FONT_TEXT_LOCK_GUARD_MUTEX();
			anim_time_scale_ = (0 > anim_time_scale_) ? 0 : anim_time_scale_;
			anim_time_scale_ = time_scale;
		}
		anim_seek_->setTimeScale(anim_time_scale_);
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::addAnimTimeScale(float add_scale) {
		if (!anim_seek_) return;
		{
			DXE_FONT_TEXT_LOCK_GUARD_MUTEX();
			anim_time_scale_ += add_scale;
			anim_time_scale_ = (0 > anim_time_scale_) ? 0 : anim_time_scale_;
		}
		anim_seek_->setTimeScale(anim_time_scale_);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::updateAnimation() {
		if (!anim_seek_) return;
		if (anim_seek_->getCondition() != tnl::SeekUnit::eCondition::PLAYING) return;
		anim_seek_->update(GetDeltaTime());

		int32_t length = static_cast<int32_t>( base_text_.length() );
		int32_t current = static_cast<int32_t>(static_cast<float>(length) * anim_seek_->getSeekRate());
		if (anim_draw_text_.length() > current) return;

		int32_t cutpos = 0;
		for (cutpos = 0; cutpos < current; ++cutpos) {
			unsigned char byte = base_text_[cutpos];
			if (tnl::IsSjisMultiUpbyte(byte)) {
				cutpos += 1;
				continue;
			}
		}
		std::string cutstr = base_text_.substr(0, cutpos);
		if (anim_draw_text_ == cutstr) return;
		std::string addstr = cutstr.substr(anim_draw_text_.length(), -1);

		{
			DXE_FONT_TEXT_LOCK_GUARD_MUTEX();
			anim_draw_text_ = cutstr;
		}

		if (addstr[0] == ' ') return;
		if (addstr[0] == '\n') return;
		if (addstr[addstr.length() - 1] == ' ') return;
		if (addstr[addstr.length() - 1] == '\n') return;
		if (anim_update_callback_) anim_update_callback_();

	}
	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::playAnimation() {
		if (!anim_seek_) return;
		anim_seek_->play() ;
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::pauseAnimation() {
		if (!anim_seek_) return;
		anim_seek_->pause();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::restartAnimation() {
		if (!anim_seek_) return;
		anim_seek_->restart();
		anim_draw_text_.clear();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::finishAnimation() {
		if (!anim_seek_) return;
		anim_seek_->jumpSeekRate(1.0f);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	tnl::SeekUnit::eCondition FontText::getAnimCondition() {
		if (!anim_seek_) return tnl::SeekUnit::eCondition::STANDBY;
		return anim_seek_->getCondition();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void FontText::setAnimUpdateCallback(const std::function<void()>& callback) {
		DXE_FONT_TEXT_LOCK_GUARD_MUTEX();
		anim_update_callback_ = callback;
	}

}
