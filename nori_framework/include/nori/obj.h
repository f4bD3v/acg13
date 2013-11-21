/*
    XXX licence to ... 
*/

#if !defined(__OBJ_H)
#define __OBJ_H

#include <nori/mesh.h>

NORI_NAMESPACE_BEGIN

/**
 * \brief Load a .obj mesh from the specified filename
 */
extern Mesh *loadOBJFile(const QString &filename);

NORI_NAMESPACE_END

#endif /* __OBJ_H */
