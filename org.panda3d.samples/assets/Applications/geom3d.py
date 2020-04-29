from panda3d.core import (
    Vec3,
    GeomVertexFormat,
    GeomVertexData,
    GeomVertexWriter,
    GeomTriangles,
    Geom,
    GeomNode,
    NodePath,
    GeomPoints,
    loadPrcFileData,
)


def Cube(size=1.0, geom_name="CubeMaker", gvd_name="Data", gvw_name="vertex"):


    format = GeomVertexFormat.getV3()
    data = GeomVertexData(gvd_name, format, Geom.UHStatic)
    vertices = GeomVertexWriter(data, gvw_name)

    size = float(size) / 2.0
    vertices.addData3f(-size, -size, -size)
    vertices.addData3f(+size, -size, -size)
    vertices.addData3f(-size, +size, -size)
    vertices.addData3f(+size, +size, -size)
    vertices.addData3f(-size, -size, +size)
    vertices.addData3f(+size, -size, +size)
    vertices.addData3f(-size, +size, +size)
    vertices.addData3f(+size, +size, +size)

    triangles = GeomTriangles(Geom.UHStatic)

    def addQuad(v0, v1, v2, v3):
        triangles.addVertices(v0, v1, v2)
        triangles.addVertices(v0, v2, v3)
        triangles.closePrimitive()

    addQuad(4, 5, 7, 6)  # Z+
    addQuad(0, 2, 3, 1)  # Z-
    addQuad(3, 7, 5, 1)  # X+
    addQuad(4, 6, 2, 0)  # X-
    addQuad(2, 6, 7, 3)  # Y+
    addQuad(0, 1, 5, 4)  # Y+

    geom = Geom(data)
    geom.addPrimitive(triangles)

    node = GeomNode(geom_name)
    node.addGeom(geom)

    return NodePath(node)

