#include "ModelView.hpp"

namespace sp
{

ModelView::ModelView(const glm::vec3 &o, const glm::vec3 &s)
{
    origin = o;
    scale = s;
}

glm::mat4 ModelView::GetModel() const
{
    glm::mat4 rot_mat = glm::mat4_cast(rot);
    glm::mat4 trans_mat = glm::translate(origin);
    glm::mat4 scale_mat = glm::scale(scale);

    return trans_mat * rot_mat * scale_mat;
}

} // namespace sp
