//------------------------------------------------------------------------------
//  drawState.cc
//------------------------------------------------------------------------------
#include "Pre.h"
#include "drawState.h"

namespace Oryol {
namespace _priv {

//------------------------------------------------------------------------------
drawState::drawState() :
mesh_(nullptr),
programBundle_(nullptr) {
    // empty
}

//------------------------------------------------------------------------------
drawState::~drawState() {
    o_assert(nullptr == this->mesh_);
    o_assert(nullptr == this->programBundle_);
}

//------------------------------------------------------------------------------
void
drawState::clear() {
    this->mesh_ = nullptr;
    this->programBundle_ = nullptr;
    resourceBase::clear();
}

//------------------------------------------------------------------------------
void
drawState::setMesh(mesh* msh) {
    o_assert(nullptr != msh);
    this->mesh_ = msh;
}

//------------------------------------------------------------------------------
void
drawState::setProgramBundle(programBundle* pb) {
    o_assert(nullptr != pb);
    this->programBundle_ = pb;
}

} // namespace _priv
} // namespace Oryol