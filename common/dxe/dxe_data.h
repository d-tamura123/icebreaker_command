#pragma once
#include <string>
#include "../tnl/tnl_rect.h"

namespace dxe {

	extern std::string g_x_file_hedder_str;
	extern tnl::Rect WINDOW_RECT ;

    class IResourceUnit : public std::enable_shared_from_this<IResourceUnit> {
    public :
        virtual ~IResourceUnit() = default;
    protected:
        IResourceUnit() = default;
    private:
    };

    class ResourceHandle : public IResourceUnit {
    public :
        virtual ~ResourceHandle() {
            if (release_function_) {
                release_function_( this );
            }
        }
        ResourceHandle() = default;
        TNL_PROPERTY( int, DxlibHandle, dxlib_handle_ );
        void setReleaseFunction(const std::function<void(ResourceHandle* resource)>& function) {
            release_function_ = function;
        }
    private :
        int dxlib_handle_ = -1;
        std::function<void(ResourceHandle* resource)> release_function_;
    };

    class ResourceHandleArray : public IResourceUnit {
    public:
        virtual ~ResourceHandleArray() {
            if (release_function_) {
                release_function_( this );
            }
        }
        ResourceHandleArray() = default;
        void setReleaseFunction(const std::function<void(ResourceHandleArray* resource)>& function) {
            release_function_ = function;
        }
        std::vector<Shared<ResourceHandle>> handles_;
    private :
        std::function<void(ResourceHandleArray* resource)> release_function_;
    };

	enum class eRectOrigin { 
		LEFT_TOP
		, LEFT_CENTER
		, LEFT_BOTTOM
		, CENTER_TOP
		, CENTER
		, CENTER_BOTTOM
		, RIGHT_TOP
		, RIGHT_CENTER
		, RIGHT_BOTTOM
        , MAX
	};

	enum class eDepthStenclil {
		DEPTH_W_ON_T_ON
		, DEPTH_W_ON_T_OFF
		, DEPTH_W_OFF_T_ON
		, DEPTH_W_OFF_T_OFF
		, MAX
	};
	enum class eBlendState {
		ALPHA
		, ADD
		, SUB
		, MUL
		, NORMAL
		, MAX
	};
	enum class eSamplerState {
		ANISOTROPIC
		, BILINEAR
		, NEAREST
		, MAX
	};
	enum class eRasterizerState {
		CULL_NONE
		, CULL_FRONT
		, CULL_BACK
		, WIREFRAME
		, MAX
	};



	class Colors {
	public :
        static constexpr int  AliceBlue = TNL_ARGB8(240, 248, 255, 255);
        static constexpr int  AntiqueWhite = TNL_ARGB8(250, 235, 215, 255);
        static constexpr int  Aqua = TNL_ARGB8(0, 255, 255, 255);
        static constexpr int  Aquamarine = TNL_ARGB8(127, 255, 212, 255);
        static constexpr int  Azure = TNL_ARGB8(240, 255, 255, 255);
        static constexpr int  Beige = TNL_ARGB8(245, 245, 220, 255);
        static constexpr int  Bisque = TNL_ARGB8(255, 228, 196, 255);
        static constexpr int  Black = TNL_ARGB8(0, 0, 0, 255);
        static constexpr int  BlanchedAlmond = TNL_ARGB8(255, 235, 205, 255);
        static constexpr int  Blue = TNL_ARGB8(0, 0, 255, 255);
        static constexpr int  BlueViolet = TNL_ARGB8(138, 43, 226, 255);
        static constexpr int  Brown = TNL_ARGB8(165, 42, 42, 255);
        static constexpr int  Burlywood = TNL_ARGB8(222, 184, 135, 255);
        static constexpr int  CadetBlue = TNL_ARGB8(95, 158, 160, 255);
        static constexpr int  Chartreuse = TNL_ARGB8(127, 255, 0, 255);
        static constexpr int  Chocolate = TNL_ARGB8(210, 105, 30, 255);
        static constexpr int  Coral = TNL_ARGB8(255, 127, 80, 255);
        static constexpr int  CornflowerBlue = TNL_ARGB8(100, 149, 237, 255);
        static constexpr int  Cornsilk = TNL_ARGB8(255, 248, 220, 255);
        static constexpr int  Crimson = TNL_ARGB8(220, 20, 60, 255);
        static constexpr int  Cyan = TNL_ARGB8(0, 255, 255, 255);
        static constexpr int  DarkBlue = TNL_ARGB8(0, 0, 139, 255);
        static constexpr int  DarkCyan = TNL_ARGB8(0, 139, 139, 255);
        static constexpr int  DarkGoldenrod = TNL_ARGB8(184, 134, 11, 255);
        static constexpr int  DarkGray = TNL_ARGB8(169, 169, 169, 255);
        static constexpr int  DarkGreen = TNL_ARGB8(0, 100, 0, 255);
        static constexpr int  DarkKhaki = TNL_ARGB8(189, 183, 107, 255);
        static constexpr int  DarkMagenta = TNL_ARGB8(139, 0, 139, 255);
        static constexpr int  DarkOliveGreen = TNL_ARGB8(85, 107, 47, 255);
        static constexpr int  DarkOrange = TNL_ARGB8(255, 140, 0, 255);
        static constexpr int  DarkOrchid = TNL_ARGB8(153, 50, 204, 255);
        static constexpr int  DarkRed = TNL_ARGB8(139, 0, 0, 255);
        static constexpr int  DarkSalmon = TNL_ARGB8(233, 150, 122, 255);
        static constexpr int  DarkSeaGreen = TNL_ARGB8(143, 188, 143, 255);
        static constexpr int  DarkSlateBlue = TNL_ARGB8(72, 61, 139, 255);
        static constexpr int  DarkSlateGray = TNL_ARGB8(47, 79, 79, 255);
        static constexpr int  DarkTurquoise = TNL_ARGB8(0, 206, 209, 255);
        static constexpr int  DarkViolet = TNL_ARGB8(148, 0, 211, 255);
        static constexpr int  DeepPink = TNL_ARGB8(255, 20, 147, 255);
        static constexpr int  DeepSkyBlue = TNL_ARGB8(0, 191, 255, 255);
        static constexpr int  DimGray = TNL_ARGB8(105, 105, 105, 255);
        static constexpr int  DodgerBlue = TNL_ARGB8(30, 144, 255, 255);
        static constexpr int  FireBrick = TNL_ARGB8(178, 34, 34, 255);
        static constexpr int  FloralWhite = TNL_ARGB8(255, 250, 240, 255);
        static constexpr int  ForestGreen = TNL_ARGB8(34, 139, 34, 255);
        static constexpr int  Fuchsia = TNL_ARGB8(255, 0, 255, 255);
        static constexpr int  Gainsboro = TNL_ARGB8(220, 220, 220, 255);
        static constexpr int  GhostWhite = TNL_ARGB8(248, 248, 255, 255);
        static constexpr int  Gold = TNL_ARGB8(255, 215, 0, 255);
        static constexpr int  Goldenrod = TNL_ARGB8(218, 165, 32, 255);
        static constexpr int  Gray = TNL_ARGB8(128, 128, 128, 255);
        static constexpr int  Green = TNL_ARGB8(0, 128, 0, 255);
        static constexpr int  GreenYellow = TNL_ARGB8(173, 255, 47, 255);
        static constexpr int  Honeydew = TNL_ARGB8(240, 255, 240, 255);
        static constexpr int  HotPink = TNL_ARGB8(255, 105, 180, 255);
        static constexpr int  IndianRed = TNL_ARGB8(205, 92, 92, 255);
        static constexpr int  Indigo = TNL_ARGB8(75, 0, 130, 255);
        static constexpr int  Ivory = TNL_ARGB8(255, 255, 240, 255);
        static constexpr int  Khaki = TNL_ARGB8(240, 230, 140, 255);
        static constexpr int  Lavender = TNL_ARGB8(230, 230, 250, 255);
        static constexpr int  Lavenderblush = TNL_ARGB8(255, 240, 245, 255);
        static constexpr int  LawnGreen = TNL_ARGB8(124, 252, 0, 255);
        static constexpr int  LemonChiffon = TNL_ARGB8(255, 250, 205, 255);
        static constexpr int  LightBlue = TNL_ARGB8(173, 216, 230, 255);
        static constexpr int  LightCoral = TNL_ARGB8(240, 128, 128, 255);
        static constexpr int  LightCyan = TNL_ARGB8(224, 255, 255, 255);
        static constexpr int  LightGoldenodYellow = TNL_ARGB8(250, 250, 210, 255);
        static constexpr int  LightGray = TNL_ARGB8(211, 211, 211, 255);
        static constexpr int  LightGreen = TNL_ARGB8(144, 238, 144, 255);
        static constexpr int  LightPink = TNL_ARGB8(255, 182, 193, 255);
        static constexpr int  LightSalmon = TNL_ARGB8(255, 160, 122, 255);
        static constexpr int  LightSeaGreen = TNL_ARGB8(32, 178, 170, 255);
        static constexpr int  LightSkyBlue = TNL_ARGB8(135, 206, 250, 255);
        static constexpr int  LightSlateGray = TNL_ARGB8(119, 136, 153, 255);
        static constexpr int  LightSteelBlue = TNL_ARGB8(176, 196, 222, 255);
        static constexpr int  LightYellow = TNL_ARGB8(255, 255, 224, 255);
        static constexpr int  Lime = TNL_ARGB8(0, 255, 0, 255);
        static constexpr int  LimeGreen = TNL_ARGB8(50, 205, 50, 255);
        static constexpr int  Linen = TNL_ARGB8(250, 240, 230, 255);
        static constexpr int  Magenta = TNL_ARGB8(255, 0, 255, 255);
        static constexpr int  Maroon = TNL_ARGB8(128, 0, 0, 255);
        static constexpr int  MediumAquamarine = TNL_ARGB8(102, 205, 170, 255);
        static constexpr int  MediumBlue = TNL_ARGB8(0, 0, 205, 255);
        static constexpr int  MediumOrchid = TNL_ARGB8(186, 85, 211, 255);
        static constexpr int  MediumPurple = TNL_ARGB8(147, 112, 219, 255);
        static constexpr int  MediumSeaGreen = TNL_ARGB8(60, 179, 113, 255);
        static constexpr int  MediumSlateBlue = TNL_ARGB8(123, 104, 238, 255);
        static constexpr int  MediumSpringGreen = TNL_ARGB8(0, 250, 154, 255);
        static constexpr int  MediumTurquoise = TNL_ARGB8(72, 209, 204, 255);
        static constexpr int  MediumVioletRed = TNL_ARGB8(199, 21, 133, 255);
        static constexpr int  MidnightBlue = TNL_ARGB8(25, 25, 112, 255);
        static constexpr int  Mintcream = TNL_ARGB8(245, 255, 250, 255);
        static constexpr int  MistyRose = TNL_ARGB8(255, 228, 225, 255);
        static constexpr int  Moccasin = TNL_ARGB8(255, 228, 181, 255);
        static constexpr int  NavajoWhite = TNL_ARGB8(255, 222, 173, 255);
        static constexpr int  Navy = TNL_ARGB8(0, 0, 128, 255);
        static constexpr int  OldLace = TNL_ARGB8(253, 245, 230, 255);
        static constexpr int  Olive = TNL_ARGB8(128, 128, 0, 255);
        static constexpr int  Olivedrab = TNL_ARGB8(107, 142, 35, 255);
        static constexpr int  Orange = TNL_ARGB8(255, 165, 0, 255);
        static constexpr int  Orangered = TNL_ARGB8(255, 69, 0, 255);
        static constexpr int  Orchid = TNL_ARGB8(218, 112, 214, 255);
        static constexpr int  PaleGoldenrod = TNL_ARGB8(238, 232, 170, 255);
        static constexpr int  PaleGreen = TNL_ARGB8(152, 251, 152, 255);
        static constexpr int  PaleTurquoise = TNL_ARGB8(175, 238, 238, 255);
        static constexpr int  PaleVioletred = TNL_ARGB8(219, 112, 147, 255);
        static constexpr int  PapayaWhip = TNL_ARGB8(255, 239, 213, 255);
        static constexpr int  PeachPuff = TNL_ARGB8(255, 218, 185, 255);
        static constexpr int  Peru = TNL_ARGB8(205, 133, 63, 255);
        static constexpr int  Pink = TNL_ARGB8(255, 192, 203, 255);
        static constexpr int  Plum = TNL_ARGB8(221, 160, 221, 255);
        static constexpr int  PowderBlue = TNL_ARGB8(176, 224, 230, 255);
        static constexpr int  Purple = TNL_ARGB8(128, 0, 128, 255);
        static constexpr int  Red = TNL_ARGB8(255, 0, 0, 255);
        static constexpr int  RosyBrown = TNL_ARGB8(188, 143, 143, 255);
        static constexpr int  RoyalBlue = TNL_ARGB8(65, 105, 225, 255);
        static constexpr int  SaddleBrown = TNL_ARGB8(139, 69, 19, 255);
        static constexpr int  Salmon = TNL_ARGB8(250, 128, 114, 255);
        static constexpr int  SandyBrown = TNL_ARGB8(244, 164, 96, 255);
        static constexpr int  SeaGreen = TNL_ARGB8(46, 139, 87, 255);
        static constexpr int  Seashell = TNL_ARGB8(255, 245, 238, 255);
        static constexpr int  Sienna = TNL_ARGB8(160, 82, 45, 255);
        static constexpr int  Silver = TNL_ARGB8(192, 192, 192, 255);
        static constexpr int  SkyBlue = TNL_ARGB8(135, 206, 235, 255);
        static constexpr int  SlateBlue = TNL_ARGB8(106, 90, 205, 255);
        static constexpr int  SlateGray = TNL_ARGB8(112, 128, 144, 255);
        static constexpr int  Snow = TNL_ARGB8(255, 250, 250, 255);
        static constexpr int  SpringGreen = TNL_ARGB8(0, 255, 127, 255);
        static constexpr int  SteelBlue = TNL_ARGB8(70, 130, 180, 255);
        static constexpr int  Tan = TNL_ARGB8(210, 180, 140, 255);
        static constexpr int  Teal = TNL_ARGB8(0, 128, 128, 255);
        static constexpr int  Thistle = TNL_ARGB8(216, 191, 216, 255);
        static constexpr int  Tomato = TNL_ARGB8(255, 99, 71, 255);
        static constexpr int  Turquoise = TNL_ARGB8(64, 224, 208, 255);
        static constexpr int  Violet = TNL_ARGB8(238, 130, 238, 255);
        static constexpr int  Wheat = TNL_ARGB8(245, 222, 179, 255);
        static constexpr int  White = TNL_ARGB8(255, 255, 255, 255);
        static constexpr int  WhiteSmoke = TNL_ARGB8(245, 245, 245, 255);
        static constexpr int  Yellow = TNL_ARGB8(255, 255, 0, 255);
        static constexpr int  YellowGreen = TNL_ARGB8(154, 205, 50, 255);
	};

}

