#ifndef _SP_DEBUG_H_
#define _SP_DEBUG_H_

namespace sp
{

std::ostream &operator<<(std::ostream &os, const sp::Camera &cam)
{
    auto print_vec3 = [&os](glm::vec3 a) {
        os << a[0] << ", " << a[1] << ", " << a[2];
    };
    os << "camera pos: ";
    print_vec3(cam.pos);
    os << std::endl;
    os << "camera dir: ";
    print_vec3(cam.dir);
    os << std::endl;
    os << "camera up: ";
    print_vec3(cam.up);
    os << std::endl;
    os << "camera look: ";
    print_vec3(cam.look);
    return os;
}

} // namespace sp
#endif
