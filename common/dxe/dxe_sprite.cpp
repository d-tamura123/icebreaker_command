#include <algorithm>
#include <dxe.h>
#include "dxe_sprite.h"

namespace dxe {

	//--------------------------------------------------------------------------------------------------------------------------
	SpriteResouce::~SpriteResouce() {

	}

	//--------------------------------------------------------------------------------------------------------------------------
	Shared<dxe::ResourceHandle> SpriteResouce::LoadGraph(const std::string& file_path) {
		Shared<dxe::ResourceHandle> handle = std::shared_ptr<dxe::ResourceHandle>(new dxe::ResourceHandle());
		int dxlib_handle = DxLib::LoadGraph(file_path.c_str());
		if (-1 == dxlib_handle) {
			throw std::runtime_error(" dxe::SpriteResouce::LoadGraph load image error ");
		}
		handle->setDxlibHandle(dxlib_handle);

		handle->setReleaseFunction([]( dxe::ResourceHandle* resource ) {
			DxLib::DeleteGraph(resource->getDxlibHandle());
		});

		return handle ;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	Shared<dxe::ResourceHandleArray> SpriteResouce::LoadDivGraph
	(const std::string& file_path
		, int32_t all_num
		, int32_t x_num
		, int32_t y_num
		, int32_t x_size
		, int32_t y_size) {

		Shared<dxe::ResourceHandleArray> handle_array = std::shared_ptr<dxe::ResourceHandleArray>(new dxe::ResourceHandleArray());
		handle_array->handles_.resize(all_num);

		int* p_buff = new int[all_num];

		int result = DxLib::LoadDivGraph(file_path.c_str(), all_num, x_num, y_num, x_size, y_size, p_buff);
		if (-1 == result) {
			throw std::runtime_error(" dxe::SpriteResouce::LoadDivGraph load image error ");
		}

		for (int i = 0; i < all_num; ++i) {
			int hdl = p_buff[i];
			handle_array->handles_[i] = std::shared_ptr<dxe::ResourceHandle>(new dxe::ResourceHandle());
			handle_array->handles_[i]->setDxlibHandle(p_buff[i]);
		}
		delete[] p_buff;

		handle_array->setReleaseFunction([](dxe::ResourceHandleArray* resource) {
			for (auto hr : resource->handles_) {
				DxLib::DeleteGraph(hr->getDxlibHandle());
			}
		});

		return handle_array ;
	}


	//--------------------------------------------------------------------------------------------------------------------------
	Shared<dxe::ResourceHandleArray> SpriteResouce::LoadDivGraph
	(const std::string& file_path
		, int32_t x_size
		, int32_t y_size
		, const std::vector<int32_t>& load_frame_indexs) {

		Shared<dxe::ResourceHandleArray> handle_array = std::shared_ptr<dxe::ResourceHandleArray>(new dxe::ResourceHandleArray());
		handle_array->handles_.resize( load_frame_indexs.size() );

		tnl::Vector2i image_size ;
		int image_handle = DxLib::LoadGraph(file_path.c_str()) ;
		GetGraphSize(image_handle, &image_size.x, &image_size.y);
		DeleteGraph(image_handle);

		tnl::Vector2i image_num ;
		image_num.x = image_size.x / x_size;
		image_num.y = image_size.y / y_size;

		int32_t all_image_num = image_num.x * image_num.y;

		int* p_buff = new int[ all_image_num ];		
		int result = DxLib::LoadDivGraph(file_path.c_str(), all_image_num, image_num.x, image_num.y, x_size, y_size, p_buff);

		if (-1 == result) {
			throw std::runtime_error(" dxe::SpriteResouce::LoadDivGraph load image error ");
		}

		for (int i = 0; i < load_frame_indexs.size() ; ++i) {
			int hdl = p_buff[ load_frame_indexs[i] ];
			handle_array->handles_[i] = std::shared_ptr<dxe::ResourceHandle>(new dxe::ResourceHandle());
			handle_array->handles_[i]->setDxlibHandle( hdl );
		}

		for (int i = 0; i < all_image_num; ++i) {
			bool is_delete_graph = true ;
			for (int k = 0; k < load_frame_indexs.size(); ++k) {
				if (p_buff[i] != p_buff[load_frame_indexs[k]]) continue;
				is_delete_graph = false ;
				break;
			}
			if (!is_delete_graph) continue;
			DeleteGraph( p_buff[i] ); 
		}

		delete[] p_buff;

		return handle_array;
	}


	//--------------------------------------------------------------------------------------------------------------------------
	Shared<SpriteResouce> SpriteResouce::Create(const std::string& file_path) {
		Shared<SpriteResouce> resource = std::shared_ptr<SpriteResouce>( new SpriteResouce() );
		Shared<dxe::ResourceHandle> handle = SpriteResouce::LoadGraph(file_path.c_str());
		SpriteResourceDivisionData division;

		division.file_path_ = file_path;
		GetGraphSize(handle->getDxlibHandle(), &division.x_size_, &division.y_size_);
		resource->divisions_.emplace_back( division );
		resource->handle_array_table_.emplace_back( std::shared_ptr<ResourceHandleArray> ( new dxe::ResourceHandleArray() ) );
		resource->handle_array_table_[0]->handles_.emplace_back( handle );
		resource->handle_array_table_[0]->setReleaseFunction( [](ResourceHandleArray* resource) {
			DeleteGraph(resource->handles_[0]->getDxlibHandle());
		});
		return resource;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	Shared<SpriteResouce> SpriteResouce::Create( const Shared<dxe::Texture>& texture ) {
		Shared<SpriteResouce> resource = std::shared_ptr<SpriteResouce>(new SpriteResouce());
		Shared<dxe::ResourceHandle> handle = std::shared_ptr<dxe::ResourceHandle>(new dxe::ResourceHandle());
		handle->setDxlibHandle( texture->getDxLibGraphHandle() );

		SpriteResourceDivisionData division;
		GetGraphSize(handle->getDxlibHandle(), &division.x_size_, &division.y_size_);
		resource->divisions_.emplace_back(division);
		resource->handle_array_table_.emplace_back(std::shared_ptr<ResourceHandleArray>(new dxe::ResourceHandleArray()));
		resource->handle_array_table_[0]->handles_.emplace_back( handle );
		std::vector<Shared<dxe::Texture>> storage;
		storage.emplace_back(texture);
		resource->dxe_textures_.emplace_back( storage );

		return resource;
	}


	//--------------------------------------------------------------------------------------------------------------------------
	Shared<SpriteResouce> SpriteResouce::Create(const Shared<dxe::ResourceHandle>& resource_handle) {
		Shared<SpriteResouce> resource = std::shared_ptr<SpriteResouce>(new SpriteResouce());
		SpriteResourceDivisionData division;
		GetGraphSize(resource_handle->getDxlibHandle(), &division.x_size_, &division.y_size_);
		resource->divisions_.emplace_back(division);
		resource->handle_array_table_.emplace_back(std::shared_ptr<ResourceHandleArray>(new dxe::ResourceHandleArray()));
		resource->handle_array_table_[0]->handles_.emplace_back(resource_handle);
		return resource;
	}


	//--------------------------------------------------------------------------------------------------------------------------
	Shared<SpriteResouce> SpriteResouce::CreateDivision
		( const std::string& file_path
		, int32_t all_num
		, int32_t x_num
		, int32_t y_num
		, int32_t x_size
		, int32_t y_size) {
		Shared<dxe::SpriteResouce> resource = std::shared_ptr<dxe::SpriteResouce>(new dxe::SpriteResouce());
		resource->addDivisionProcess(file_path, all_num, x_num, y_num, x_size, y_size);
		return resource ;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	//Shared<SpriteResouce> SpriteResouce::CreateDivision
	//	(const Shared<dxe::Texture>& texture
	//	, int32_t all_num
	//	, int32_t x_num
	//	, int32_t y_num
	//	, int32_t x_size
	//	, int32_t y_size) {
	//	Shared<dxe::SpriteResouce> resource = std::shared_ptr<dxe::SpriteResouce>(new dxe::SpriteResouce());
	//	resource->addDivisionProcess(texture, all_num, x_num, y_num, x_size, y_size);
	//	return resource;
	//}

	//--------------------------------------------------------------------------------------------------------------------------
	Shared<SpriteResouce> SpriteResouce::CreateDivision
		( const Shared<dxe::ResourceHandleArray>& resource_handle
		, int32_t all_num
		, int32_t x_num
		, int32_t y_num
		, int32_t x_size
		, int32_t y_size) {
		Shared<dxe::SpriteResouce> resource = std::shared_ptr<dxe::SpriteResouce>(new dxe::SpriteResouce());
		resource->addDivisionProcess(resource_handle, all_num, x_num, y_num, x_size, y_size);
		return resource;
	}

	Shared<SpriteResouce> SpriteResouce::CreateDivision(const SpriteDivGraphDesc& desc) {
		return CreateDivision(desc.image_file_path_, desc.all_frame_num_, desc.frame_num_.x, desc.frame_num_.y, desc.size_.x, desc.size_.y);
	}
	Shared<SpriteResouce> SpriteResouce::CreateDivision
	(const Shared<dxe::ResourceHandleArray>& resource_handle, const SpriteDivGraphDesc& desc) {
		return CreateDivision(resource_handle, desc.all_frame_num_, desc.frame_num_.x, desc.frame_num_.y, desc.size_.x, desc.size_.y);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	Shared<SpriteResouce> SpriteResouce::CreateAnimation() {
		Shared<SpriteResouce> resource = std::shared_ptr<SpriteResouce>(new SpriteResouce());
		resource->is_animation_ = true;
		return resource ;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	int32_t SpriteResouce::addAnimation
		( const std::string& file_path
		, int32_t frame_num
		, int32_t x_num
		, int32_t y_num
		, int32_t x_size
		, int32_t y_size
		, float anim_total_time
		, tnl::SeekUnit::ePlayMode anim_play_mode
		, tnl::SeekUnit::ePlayDir anim_play_dir ) {

		if (!is_animation_) {
			throw std::runtime_error(" SpriteResouce::addAnimation No Animation SpriteResouce error ");
		}
		addDivisionProcess(file_path, frame_num, x_num, y_num, x_size, y_size);

		SpriteAnimationSetting anim_setting;
		anim_setting.total_anim_time_ = anim_total_time;
		anim_setting.anim_play_mode_ = anim_play_mode;
		anim_setting.anim_play_dir_ = anim_play_dir;
		animation_settings_.emplace_back(anim_setting);
		return static_cast<int32_t>( animation_settings_.size() - 1 );
	}

	//--------------------------------------------------------------------------------------------------------------------------
	int32_t SpriteResouce::addAnimation
	(const std::string& file_path
		, int32_t x_size
		, int32_t y_size
		, const std::vector<int32_t>& load_frame_indexs
		, float anim_total_time
		, tnl::SeekUnit::ePlayMode anim_play_mode
		, tnl::SeekUnit::ePlayDir anim_play_dir) {

		if (!is_animation_) {
			throw std::runtime_error(" SpriteResouce::addAnimation No Animation SpriteResouce error ");
		}
		addDivisionProcess(file_path, x_size, y_size, load_frame_indexs);

		SpriteAnimationSetting anim_setting;
		anim_setting.total_anim_time_ = anim_total_time;
		anim_setting.anim_play_mode_ = anim_play_mode;
		anim_setting.anim_play_dir_ = anim_play_dir;
		animation_settings_.emplace_back(anim_setting);
		return static_cast<int32_t>(animation_settings_.size() - 1);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	int32_t SpriteResouce::addAnimation
		(const Shared<dxe::Texture>& texture
		, int32_t x_size
		, int32_t y_size
		, const std::vector<int32_t>& load_frame_indexs
		, float anim_total_time
		, tnl::SeekUnit::ePlayMode anim_play_mode
		, tnl::SeekUnit::ePlayDir anim_play_dir) {

		if (!is_animation_) {
			throw std::runtime_error(" SpriteResouce::addAnimation No Animation SpriteResouce error ");
		}

		addDivisionProcess(texture, x_size, y_size, load_frame_indexs);

		SpriteAnimationSetting anim_setting;
		anim_setting.total_anim_time_ = anim_total_time;
		anim_setting.anim_play_mode_ = anim_play_mode;
		anim_setting.anim_play_dir_ = anim_play_dir;
		animation_settings_.emplace_back(anim_setting);
		return static_cast<int32_t>(animation_settings_.size() - 1);
	}


	//--------------------------------------------------------------------------------------------------------------------------
	int32_t SpriteResouce::addAnimation
		(const Shared<dxe::ResourceHandleArray>& resource_handle
		, int32_t frame_num
		, int32_t x_num
		, int32_t y_num
		, int32_t x_size
		, int32_t y_size
		, float anim_total_time
		, tnl::SeekUnit::ePlayMode anim_play_mode
		, tnl::SeekUnit::ePlayDir anim_play_dir ) {

		if (!is_animation_) {
			throw std::runtime_error(" SpriteResouce::addAnimation No Animation SpriteResouce error ");
		}
		addDivisionProcess(resource_handle, frame_num, x_num, y_num, x_size, y_size);

		SpriteAnimationSetting anim_setting;
		anim_setting.total_anim_time_ = anim_total_time;
		anim_setting.anim_play_mode_ = anim_play_mode;
		anim_setting.anim_play_dir_ = anim_play_dir;
		animation_settings_.emplace_back(anim_setting);
		return static_cast<int32_t>(animation_settings_.size() - 1);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	int32_t SpriteResouce::addAnimation
		(const Shared<dxe::ResourceHandleArray>& resource_handle
			, int32_t x_size
			, int32_t y_size
			, const std::vector<int32_t>& load_frame_indexs
			, float anim_total_time
			, tnl::SeekUnit::ePlayMode anim_play_mode
			, tnl::SeekUnit::ePlayDir anim_play_dir) {

		if (!is_animation_) {
			throw std::runtime_error(" SpriteResouce::addAnimation No Animation SpriteResouce error ");
		}
		addDivisionProcess(resource_handle, x_size, y_size, load_frame_indexs);

		SpriteAnimationSetting anim_setting;
		anim_setting.total_anim_time_ = anim_total_time;
		anim_setting.anim_play_mode_ = anim_play_mode;
		anim_setting.anim_play_dir_ = anim_play_dir;
		animation_settings_.emplace_back(anim_setting);
		return static_cast<int32_t>(animation_settings_.size() - 1);

	}


	int32_t SpriteResouce::addAnimation(const SpriteAnimationDesc& desc) {
		return addAnimation(desc.image_file_path_, desc.all_frame_num_, desc.frame_num_.x, desc.frame_num_.y, desc.size_.x, desc.size_.y, desc.once_playing_time_, desc.play_mode_, desc.play_dir_);
	}
	int32_t SpriteResouce::addAnimation(const Shared<dxe::ResourceHandleArray>& resource_handle, const SpriteAnimationDesc& desc) {
		return addAnimation(resource_handle, desc.all_frame_num_, desc.frame_num_.x, desc.frame_num_.y, desc.size_.x, desc.size_.y, desc.once_playing_time_, desc.play_mode_, desc.play_dir_);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteResouce::addDivisionProcess(const std::string& file_path, int32_t all_num, int32_t x_num, int32_t y_num, int32_t x_size, int32_t y_size) {

		Shared<ResourceHandleArray> handle_array = SpriteResouce::LoadDivGraph(file_path, all_num, x_num, y_num, x_size, y_size);

		SpriteResourceDivisionData division;
		division.file_path_ = file_path;
		division.all_num_ = (int32_t)handle_array->handles_.size() ;
		division.x_num_ = all_num ;
		division.y_num_ = 1;
		division.x_size_ = x_size;
		division.y_size_ = y_size;

		divisions_.emplace_back(division);
		handle_array_table_.emplace_back(handle_array);

		handle_array->setReleaseFunction([](dxe::ResourceHandleArray* resource) {
			for (auto rh : resource->handles_) {
				DeleteGraph(rh->getDxlibHandle());
			}
		});
	}


	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteResouce::addDivisionProcess(const std::string& file_path, int32_t x_size, int32_t y_size, const std::vector<int32_t>& load_frame_indexs) {

		Shared<ResourceHandleArray> handle_array = SpriteResouce::LoadDivGraph(file_path, x_size, y_size, load_frame_indexs);

		SpriteResourceDivisionData division;
		division.file_path_ = file_path;
		division.all_num_ = (int32_t)load_frame_indexs.size() ;
		division.x_num_ = division.all_num_ ;
		division.y_num_ = 1;
		division.x_size_ = x_size;
		division.y_size_ = y_size;

		divisions_.emplace_back(division);
		handle_array_table_.emplace_back(handle_array);

		handle_array->setReleaseFunction([](dxe::ResourceHandleArray* resource) {
			for (auto rh : resource->handles_) {
				DeleteGraph(rh->getDxlibHandle());
			}
		});

	}


	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteResouce::addDivisionProcess(const Shared<dxe::Texture>& texture, int32_t frame_size_w, int32_t frame_size_h) {

		std::vector<Shared<dxe::Texture>> textures = texture->createDivPixel(frame_size_w, frame_size_h, true, true);

		int32_t all_num = (int32_t)textures.size();
		Shared<dxe::ResourceHandleArray> handle_array = std::shared_ptr<dxe::ResourceHandleArray>(new dxe::ResourceHandleArray());
		handle_array->handles_.resize(all_num);

		for (int i = 0; i < all_num; ++i) {
			int hdl = textures[i]->getDxLibGraphHandle();
			handle_array->handles_[i] = std::shared_ptr<dxe::ResourceHandle>(new dxe::ResourceHandle());
			handle_array->handles_[i]->setDxlibHandle(hdl);
		}

		SpriteResourceDivisionData division;
		division.all_num_ = all_num;
		division.x_num_ = all_num;
		division.y_num_ = 1;
		division.x_size_ = frame_size_w;
		division.y_size_ = frame_size_h;

		divisions_.emplace_back(division);
		handle_array_table_.emplace_back(handle_array);
		dxe_textures_.emplace_back(textures);

	}

	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteResouce::addDivisionProcess(const Shared<dxe::Texture>& texture, int32_t frame_size_w, int32_t frame_size_h, const std::vector<int32_t>& load_frame_indexs) {

		std::vector<Shared<dxe::Texture>> textures = texture->createDivPixel(frame_size_w, frame_size_h, true, true);
		std::vector<Shared<dxe::Texture>> sel_textures ;

		for (int i = 0; i < load_frame_indexs.size(); ++i) {
			sel_textures.emplace_back( textures[ load_frame_indexs[i] ] );
		}

		int32_t all_num = (int32_t)sel_textures.size();
		Shared<dxe::ResourceHandleArray> handle_array = std::shared_ptr<dxe::ResourceHandleArray>(new dxe::ResourceHandleArray());
		handle_array->handles_.resize( all_num );

		for (int i = 0; i < all_num; ++i) {
			int hdl = sel_textures[i]->getDxLibGraphHandle() ;
			handle_array->handles_[i] = std::shared_ptr<dxe::ResourceHandle>(new dxe::ResourceHandle());
			handle_array->handles_[i]->setDxlibHandle( hdl );
		}

		SpriteResourceDivisionData division;
		division.all_num_ = all_num;
		division.x_num_ = all_num;
		division.y_num_ = 1;
		division.x_size_ = frame_size_w;
		division.y_size_ = frame_size_h;

		divisions_.emplace_back(division);
		handle_array_table_.emplace_back(handle_array);
		dxe_textures_.emplace_back(sel_textures);

	}

	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteResouce::addDivisionProcess(const Shared<dxe::ResourceHandleArray>& resource_handle, int32_t all_num, int32_t x_num, int32_t y_num, int32_t x_size, int32_t y_size) {
		SpriteResourceDivisionData division;
		division.all_num_ = all_num;
		division.x_num_ = x_num;
		division.y_num_ = y_num;
		division.x_size_ = x_size;
		division.y_size_ = y_size;
		divisions_.emplace_back(division);
		handle_array_table_.emplace_back(resource_handle);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteResouce::addDivisionProcess(const Shared<dxe::ResourceHandleArray>& resource_handle, int32_t x_size, int32_t y_size, const std::vector<int32_t>& load_frame_indexs) {
		SpriteResourceDivisionData division;
		division.all_num_ = (int32_t)load_frame_indexs.size() ;
		division.x_num_ = division.all_num_;
		division.y_num_ = 1;
		division.x_size_ = x_size;
		division.y_size_ = y_size;
		divisions_.emplace_back(division);
		handle_array_table_.emplace_back(resource_handle);
	}


	//--------------------------------------------------------------------------------------------------------------------------
	float SpriteResouce::getAnimationPlayTime(int32_t anim_index) {
		if (animation_settings_.empty()) return 0;
		if (anim_index >= (int32_t)animation_settings_.size()) return 0;
		return animation_settings_[anim_index].total_anim_time_;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	tnl::SeekUnit::ePlayMode SpriteResouce::getAnimationPlayMode(int32_t anim_index) {
		if (animation_settings_.empty()) return tnl::SeekUnit::ePlayMode::SINGLE;
		if (anim_index >= (int32_t)animation_settings_.size()) return tnl::SeekUnit::ePlayMode::SINGLE;
		return animation_settings_[anim_index].anim_play_mode_;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	tnl::SeekUnit::ePlayDir SpriteResouce::getAnimationPlayDir(int32_t anim_index) {
		if (animation_settings_.empty()) return tnl::SeekUnit::ePlayDir::FORWARD;
		if (anim_index >= (int32_t)animation_settings_.size()) return tnl::SeekUnit::ePlayDir::FORWARD;
		return animation_settings_[anim_index].anim_play_dir_;
	}


	//--------------------------------------------------------------------------------------------------------------------------
	bool SpriteAnimationController::update() {

		bool is_animation_update = false;

		seeks_[type_index_]->update( GetDeltaTime() );

		int32_t update_frame_index = seeks_[type_index_]->getSeekFrame();
		if (update_frame_index != frame_index_) {
			frame_index_ = update_frame_index;
			is_animation_update = true;
		}

		if (Sprite::eAnimFrameFadeMode::None == frame_fade_mode_) {
			current_fade_alpha_ = 1.0f;
			return is_animation_update;
		}

		float frame_time = seeks_[type_index_]->getTotalTime() / (float)(seeks_[type_index_]->getTotalFrameNum());

		if (Sprite::eAnimFrameFadeMode::Turn == frame_fade_mode_) {
			if (!tnl::IsAlmostEqualRelative(current_fade_alpha_, 1.0f)) {
				current_fade_alpha_ = tnl::UniformLerp(0.0f, 1.0f, frame_time * 0.5f, seeks_[type_index_]->getFrameProgressTime());
			}
			else {
				float progress = seeks_[type_index_]->getFrameProgressTime() - (frame_time * 0.5f);
				progress = std::clamp( progress, 0.0f, frame_time );
				before_fade_alpha_ = tnl::UniformLerp(1.0f, 0.0f, frame_time * 0.5f, progress );
			}
		}
		else if (Sprite::eAnimFrameFadeMode::Cross == frame_fade_mode_) {
			current_fade_alpha_ = tnl::UniformLerp(0.0f, 1.0f, frame_time, seeks_[type_index_]->getFrameProgressTime());
			before_fade_alpha_ = tnl::UniformLerp(1.0f, 0.0f, frame_time, seeks_[type_index_]->getFrameProgressTime());
		}
		//DrawFormatString(0, 0, -1, "%f", frame_time);
		//DrawFormatString(0, 20, -1, "%f", current_fade_alpha_);
		return is_animation_update;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::play() {
		seeks_[type_index_]->play();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::pause() {
		seeks_[type_index_]->pause();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::restart() {
		seeks_[type_index_]->restart();
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::reserve(int32_t reserve_type_index) {
		int32_t type_index = type_index_;
		reserve_index_ = reserve_type_index;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::addSeekUnit(const Shared<tnl::SeekUnit>& seek) {
		seeks_.emplace_back(seek);
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::setPlayMode(tnl::SeekUnit::ePlayMode play_mode, tnl::SeekUnit::ePlayDir play_dir) {
		seeks_[ type_index_ ]->setPlayState(play_mode, play_dir);
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::addTimeScale(float add_scale) {
		time_scale_ += add_scale;
		time_scale_ = (0 > time_scale_) ? 0 : time_scale_;
		seeks_[ type_index_ ]->setTimeScale( time_scale_ );
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::setTimeScale(const float& time_scale) {
		time_scale_ = (0 > time_scale_) ? 0 : time_scale_;
		time_scale_ = time_scale;
		seeks_[type_index_]->setTimeScale(time_scale_);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	int32_t SpriteAnimationController::getTypeNum() {
		return (int32_t)seeks_.size() ;
	}
	//--------------------------------------------------------------------------------------------------------------------------
	int32_t SpriteAnimationController::getFrameNum(int32_t type_index) {
		if (type_index >= (int32_t)seeks_.size()) return 0;
		return (int32_t)seeks_[type_index]->getTotalFrameNum();
	}

	//--------------------------------------------------------------------------------------------------------------------------
	int32_t SpriteAnimationController::getPlayingFrameIndex() {
		return seeks_[type_index_]->getSeekFrame();
	}

	//--------------------------------------------------------------------------------------------------------------------------
	tnl::SeekUnit::eCondition SpriteAnimationController::getAnimCondition() {
		return seeks_[type_index_]->getCondition();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::jumpSeekFrame(int32_t frame) {
		seeks_[type_index_]->jumpSeekFrame( frame );
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void SpriteAnimationController::setCurrentTypeIndex(const int32_t& value) {
		seeks_[type_index_]->pause();
		before_type_index_ = type_index_;
		type_index_ = value;
		restart();
	}

	//--------------------------------------------------------------------------------------------------------------------------
	//Shared<SpriteResouce> SpriteResouce::extraction(int32_t index) {
	//	if (handles_.empty() || index >= handles_.size() ) {
	//		throw std::runtime_error(" SpriteResouce::extraction handles empty error ");
	//	}
	//	Shared<SpriteResouce> resource = std::shared_ptr<SpriteResouce>(new SpriteResouce());
	//	resource->file_path_ = file_path_;
	//	resource->all_num_ = 1;
	//	resource->x_num_ = 1;
	//	resource->y_num_ = 1;
	//	resource->x_size_ = x_size_;
	//	resource->y_size_ = y_size_;
	//	resource->handles_.emplace_back( handles_[ index ] );
	//	return resource;
	//}


	//--------------------------------------------------------------------------------------------------------------------------
	Shared<Sprite> Sprite::Create(const Shared<SpriteResouce>& resource ) {
		Shared<Sprite> sprite = std::shared_ptr<Sprite>( new Sprite() );
		sprite->resource_ = resource ;
		sprite->setLocation(sprite->location_);
		return sprite;
	}


	//--------------------------------------------------------------------------------------------------------------------------
	Shared<Sprite> Sprite::CreateAnimation( const Shared<SpriteResouce>& resources ) {
		Shared<Sprite> sprite = std::shared_ptr<Sprite>(new Sprite());
		sprite->resource_ = resources;
		sprite->setLocation(sprite->location_);

		int32_t anim_num = static_cast<int32_t>( resources->handle_array_table_.size() );

		Shared<SpriteAnimationController> ctrl = std::make_shared<SpriteAnimationController>();

		for (int i = 0; i < anim_num; ++i) {
			int32_t frame_num = resources->divisions_[i].all_num_;
			float total_time = resources->animation_settings_[i].total_anim_time_;
			tnl::SeekUnit::ePlayMode play_mode = resources->animation_settings_[i].anim_play_mode_;
			tnl::SeekUnit::ePlayDir play_dir = resources->animation_settings_[i].anim_play_dir_;
			ctrl->addSeekUnit( tnl::SeekUnit::Create(DXE_FIX_FPS, frame_num, total_time, play_mode, play_dir) );
		}
		sprite->anim_ctrl_ = ctrl ;

		return sprite;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::compImageReverse(fImageReverse flag) { f_image_reverse_ |= flag; }
	bool Sprite::isImageReverseX() { return static_cast<bool>(f_image_reverse_ & fImageReverse::X); }
	bool Sprite::isImageReverseY() { return static_cast<bool>(f_image_reverse_ & fImageReverse::Y); }

	//--------------------------------------------------------------------------------------------------------------------------
	tnl::Vector2f Sprite::getSize() {
		int32_t index = (anim_ctrl_)? anim_ctrl_->getCurrentTypeIndex() : 0;
		return { (float)resource_->divisions_[index].x_size_, (float)resource_->divisions_[index].y_size_ };
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::setLocation( const eRectOrigin& location ) {
		location_ = location;
		tnl::Vector2f offset{ 0, 0 };
		GetRectOriginLocatePosition(offset.x, offset.y, (int)(getSize().x), (int)(getSize().y), location_);
		locate_position_ = -offset ;
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::draw(const Shared<dxe::Camera>& camera) {

		float scale = scale_;
		tnl::Vector2f scale_xy = scale_xy_;
		tnl::Vector2f draw_pos = position_;
		if (camera) {
			//draw_pos.x -= camera->getPositionX() * camera_follow_weight_x_;
			//draw_pos.y -= camera->getPositionY() * camera_follow_weight_y_;
			//draw_pos.x += camera->getScreenWidth() * 0.5f;
			//draw_pos.y += camera->getScreenHeight() * 0.5f;
			scale *= camera->getZoom2D() ;
			scale_xy *= camera->getZoom2D() ;
			draw_pos = camera->convertWorldToScreenPosition2D(position_, tnl::Vector2f(camera_follow_weight_x_, camera_follow_weight_y_) );
		}

		int32_t type_index = 0;
		int32_t frame_index = 0;
		float before_alpha = 0.0f;
		float current_alpha = 1.0f;
		eAnimFrameFadeMode frame_fade_mode = eAnimFrameFadeMode::None;
		int32_t before_image_handle = 0;
		if (anim_ctrl_) {
			type_index = anim_ctrl_->getCurrentTypeIndex();
			frame_index = anim_ctrl_->getPlayingFrameIndex();
			before_alpha = anim_ctrl_->getBeforeFadeAlpha();
			current_alpha = anim_ctrl_->getCurrentFadeAlpha();
			frame_fade_mode = anim_ctrl_->getFrameFadeMode();
			before_image_handle = anim_ctrl_->getBeforeImageHandle();
		}

		int image_handle = resource_->handle_array_table_[type_index]->handles_[frame_index]->getDxlibHandle();

		bool is_rev_x = static_cast<bool>(f_image_reverse_ & Sprite::fImageReverse::X);
		bool is_rev_y = static_cast<bool>(f_image_reverse_ & Sprite::fImageReverse::Y);

		int blend_mode, blend_param;
		GetDrawBlendMode(&blend_mode, &blend_param);

		if ( frame_fade_mode != eAnimFrameFadeMode::None && blend_mode_ != eBlendMode::None ) {
			SetDrawBlendMode(static_cast<int>(blend_mode_), static_cast<int>(before_alpha * alpha_ * 255.0f));
			switch (transform_mode_) {
			case eTransformMode::Simple:
				DrawGraphF(draw_pos.x, draw_pos.y, before_image_handle, TRUE);
				break;
			case eTransformMode::RotScale:
				DrawRotaGraphFastF(draw_pos.x, draw_pos.y, scale, rotation_, before_image_handle, TRUE, is_rev_x, is_rev_y);
				break;
			case eTransformMode::RotLocateScaleXY:
				DrawRotaGraphFast3F(draw_pos.x, draw_pos.y, locate_position_.x, locate_position_.y, scale_xy.x, scale_xy.y, rotation_, before_image_handle, TRUE, is_rev_x, is_rev_y);
				break;
			}
		}

		tnl::Vector2f draw_size = getSize() ;
		tnl::Vector2f draw_scale = { 1.0f, 1.0f };

		SetDrawBlendMode(static_cast<int>(blend_mode_), static_cast<int>(current_alpha * alpha_ * 255.0f));
		switch ( transform_mode_ ) {
		case eTransformMode::Simple:
			DrawGraphF(draw_pos.x, draw_pos.y, image_handle, TRUE);
			break;
		case eTransformMode::RotScale:
			draw_size *= scale;
			draw_scale = scale;
			DrawRotaGraphFastF(draw_pos.x, draw_pos.y, scale, rotation_, image_handle, TRUE, is_rev_x, is_rev_y);
			break;
		case eTransformMode::RotLocateScaleXY:
			draw_size *= scale_xy;
			draw_scale = scale_xy;
			DrawRotaGraphFast3F(draw_pos.x, draw_pos.y, locate_position_.x, locate_position_.y, scale_xy.x, scale_xy.y, rotation_, image_handle, TRUE, is_rev_x, is_rev_y);
			break;
		}

		SetDrawBlendMode(blend_mode, blend_param);

		if (!is_debug_line_draw_) return;

		float x = -locate_position_.x * draw_scale.x;
		float y = -locate_position_.y * draw_scale.y;
		tnl::Vector3 c = { x + draw_size.x * 0.5f, y + draw_size.y * 0.5f, 0 };
		c = tnl::Vector3::TransformCoord(c, tnl::Quaternion::RotationAxis({ 0, 0, 1 }, rotation_));
		tnl::Vector2f center = draw_pos + tnl::Vector2f(c) ;
		DrawRotaBox(center, draw_size.x, draw_size.y, rotation_, debug_line_color_);
		DrawCircle((int)draw_pos.x, (int)draw_pos.y, 5, debug_line_color_, true);

		//DrawFormatString(0, 200, -1, "%s", locate_position_.toString().c_str());
	}


	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::setAnimPlayMode(tnl::SeekUnit::ePlayMode play_mode, tnl::SeekUnit::ePlayDir play_dir) {
		if (!anim_ctrl_) return;
		anim_ctrl_->setPlayMode(play_mode, play_dir);
	}


	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::updateAnimation() {
		if (!anim_ctrl_) return;
		if (anim_ctrl_->getAnimCondition() != tnl::SeekUnit::eCondition::PLAYING) return;

		int32_t type_index = anim_ctrl_->getCurrentTypeIndex();
		int32_t frame_index = anim_ctrl_->getPlayingFrameIndex();
		int before_image_handle = resource_->handle_array_table_[type_index]->handles_[frame_index]->getDxlibHandle();

		if (anim_ctrl_->update() && anim_ctrl_->getFrameFadeMode() != eAnimFrameFadeMode::None) {
			anim_ctrl_->setBeforeImageHandle(before_image_handle);
			anim_ctrl_->setCurrentFadeAlpha(0);
			anim_ctrl_->setBeforeFadeAlpha(1.0f);
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::reserveAnimation(int32_t reserve_type_index) {
		if (!anim_ctrl_) return;
		if (reserve_type_index >= resource_->handle_array_table_.size()) {
			throw std::runtime_error(" Sprite::setAnimation animation type index error ");
		}
		anim_ctrl_->reserve(reserve_type_index);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	int32_t Sprite::getAnimTypeNum() {
		if (!anim_ctrl_) return 0;
		return anim_ctrl_->getTypeNum();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	int32_t Sprite::getAnimFrameNum(int32_t type_index) {
		if (!anim_ctrl_) return 0;
		return anim_ctrl_->getFrameNum(type_index);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	int32_t Sprite::getAnimCurrentTypeIndex() {
		if (!anim_ctrl_) return 0;
		return anim_ctrl_->getCurrentTypeIndex(); 
	}
	//--------------------------------------------------------------------------------------------------------------------------
	int32_t Sprite::getAnimPlayingFrameIndex() {
		if (!anim_ctrl_) return 0;
		return anim_ctrl_->getPlayingFrameIndex();
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::changeAnimation(int32_t animation_index) {
		if (!anim_ctrl_) return;
		reserveAnimation( animation_index );
		restartAnimation();
		anim_ctrl_->setCurrentTypeIndex( animation_index );
		setLocation(location_);
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::playAnimation() {
		if (!anim_ctrl_) return;
		anim_ctrl_->play();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::pauseAnimation() {
		if (!anim_ctrl_) return;
		anim_ctrl_->pause();
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::restartAnimation() {
		if (!anim_ctrl_) return;
		anim_ctrl_->restart();
	}

	//--------------------------------------------------------------------------------------------------------------------------
	tnl::SeekUnit::eCondition Sprite::getAnimCondition() {
		if (!anim_ctrl_) return tnl::SeekUnit::eCondition::STANDBY;
		return anim_ctrl_->getAnimCondition();
	}

	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::setAnimTimeScale(float time_scale) {
		if (!anim_ctrl_) return;
		anim_ctrl_->setTimeScale(time_scale);
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::addAnimTimeScale(float add_scale) {
		if (!anim_ctrl_) return;
		anim_ctrl_->addTimeScale(add_scale);
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::setAnimFrameFadeMode(eAnimFrameFadeMode frame_fade_mode) {
		if (!anim_ctrl_) return;
		anim_ctrl_->setFrameFadeMode( frame_fade_mode );
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::jumpAnimSeekFrame(int32_t frame) {
		if (!anim_ctrl_) return;
		anim_ctrl_->jumpSeekFrame(frame);
	}
	//--------------------------------------------------------------------------------------------------------------------------
	void Sprite::setDebugLine(bool is_draw_line, int32_t color) {
		is_debug_line_draw_ = true;
		debug_line_color_ = color;
	}

}