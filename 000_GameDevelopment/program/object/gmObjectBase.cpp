#include "gmObjectBase.h"

namespace gm {

    gmObjectBase::gmObjectBase(const std::string& instanceID, const tnl::Vector3& position)
        : instanceID_(instanceID), position_(position) {
    }

    // 座標
    const tnl::Vector3& gmObjectBase::getPosition() const {
        return position_;
    }

    void gmObjectBase::setPosition(const tnl::Vector3& position) {
        position_ = position;
    }

    // スケール
    const tnl::Vector3& gmObjectBase::getScale() const {
        return scale_;
    }

    void gmObjectBase::setScale(const tnl::Vector3& scale) {
        scale_ = scale;
    }

    void gmObjectBase::update(float deltaTime) {
        // 基底では何もしない
    }

    void gmObjectBase::draw() {
        // 基底では何もしない
    }

    void gmObjectBase::render(const Shared<dxe::Camera>& camera) {
        // 基底では何もしない
    }
}