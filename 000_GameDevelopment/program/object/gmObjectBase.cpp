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

    // Euler角(rotation_)からクォータニオンへ変換して取得する
    // Note:
    //  gmMeshBase::render()内の変換と共通のロジック。
    //  コライダー判定などで使用
    tnl::Quaternion gmObjectBase::getRotationQuaternion() const {
        return
            tnl::Quaternion::RotationAxis({ 1, 0, 0 }, rotation_.x) *
            tnl::Quaternion::RotationAxis({ 0, 1, 0 }, rotation_.y) *
            tnl::Quaternion::RotationAxis({ 0, 0, 1 }, rotation_.z);
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