#ifndef _SP_MODEL_VIEW_H_
#define _SP_MODEL_VIEW_H_

#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp> 

namespace sp {

struct ModelView {
    glm::vec3 origin;
    glm::vec3 scale;
    glm::quat rot;

    ModelView(const glm::vec3 &o, const glm::vec3 &s);
    glm::mat4 GetModel() const;
};

} // namespace sp
#endif
