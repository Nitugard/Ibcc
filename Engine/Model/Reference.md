
In order to be valid, gltf asset info is specified at the start of the file


       "asset" : {
            "generator" : "Khronos glTF Blender I/O v1.4.40",
            "version" : "2.0"
        },


Array of scenes, scene index represents the starting scene whereas nodes
represent main object hierarchy in the scene

        "scene" : 0,
        "scenes" : [
            {
                "name" : "Scene",
                "nodes" : [
                    0,
                    1
                ]
            }
        ],

Array of nodes across all scenes with specified type, name and their local transformation(with the help of index ie. mesh = 0)
. They can be nested, forming child parent hierarchy. Note that node 0 has default rotation and translation,
hence specification is omitted.

    "nodes" : [
        {
            "mesh" : 0,
            "name" : "Cube"
        },
        {
            "mesh" : 1,
            "name" : "Cube.001",
            "rotation" : [
                -0.49929237365722656,
                -0.2718203067779541,
                -0.2205304354429245,
                0.7925827503204346
            ],
            "translation" : [
                -2.3817710876464844,
                2.4395575523376465,
                2.3855490684509277
            ]
        }
    ],

List of meshes, note that mesh names correspond to their respective node names. Each mesh consist
of a set of primitives(ie. draw calls). Each primitive specifies material index(if any), indices(index into accessor), and
attribute(index into accessor). Accessor tells how and from where to read required index/vertex data.

    "meshes" : [
        {
            "name" : "Cube.001",
            "primitives" : [
                {
                    "attributes" : {
                        "POSITION" : 0,
                        "NORMAL" : 1,
                        "TEXCOORD_0" : 2
                    },
                    "indices" : 3
                }
            ]
        },
        {
            "name" : "Cube.002",
            "primitives" : [
                {
                    "attributes" : {
                        "POSITION" : 4,
                        "NORMAL" : 5,
                        "TEXCOORD_0" : 6
                    },
                    "indices" : 3
                }
            ]
        }
    ],

Accessors specify buffer view index from which data is read. Component type is tied to opengl buffer type. Count is the number of 
elements to be read. Max and min specify maximum and minimum value while the type specify stored data type.

    "accessors" : [
        {
            "bufferView" : 0,
            "componentType" : 5126,
            "count" : 24,
            "max" : [
                1,
                1,
                1
            ],
            "min" : [
                -1,
                -1,
                -1
            ],
            "type" : "VEC3"
        },
        {
            "bufferView" : 1,
            "componentType" : 5126,
            "count" : 24,
            "type" : "VEC3"
        },
        {
            "bufferView" : 2,
            "componentType" : 5126,
            "count" : 24,
            "type" : "VEC2"
        },
        {
            "bufferView" : 3,
            "componentType" : 5123,
            "count" : 36,
            "type" : "SCALAR"
        },
        {
            "bufferView" : 4,
            "componentType" : 5126,
            "count" : 24,
            "max" : [
                1,
                1,
                1
            ],
            "min" : [
                -1,
                -1,
                -1
            ],
            "type" : "VEC3"
        },
        {
            "bufferView" : 5,
            "componentType" : 5126,
            "count" : 24,
            "type" : "VEC3"
        },
        {
            "bufferView" : 6,
            "componentType" : 5126,
            "count" : 24,
            "type" : "VEC2"
        }
    ],

Buffer views are contiguous 'views' into a buffer specified by buffer(index), length and offset.

    "bufferViews" : [
        {
            "buffer" : 0,
            "byteLength" : 288,
            "byteOffset" : 0
        },
        {
            "buffer" : 0,
            "byteLength" : 288,
            "byteOffset" : 288
        },
        {
            "buffer" : 0,
            "byteLength" : 192,
            "byteOffset" : 576
        },
        {
            "buffer" : 0,
            "byteLength" : 72,
            "byteOffset" : 768
        },
        {
            "buffer" : 0,
            "byteLength" : 288,
            "byteOffset" : 840
        },
        {
            "buffer" : 0,
            "byteLength" : 288,
            "byteOffset" : 1128
        },
        {
            "buffer" : 0,
            "byteLength" : 192,
            "byteOffset" : 1416
        }
    ],

Buffers can be embedded. But can also be stored in other files.

    "buffers" : [
        {
            "byteLength" : 1608,
            "uri" : "data:application/octet-stream;base64,AACAvwAAgL8AAIA/AACAvwAAgL8AAIA/AACAvwAAgL8AAIA/AACAvwAAgD8AAIA/AACAvwAAgD8AAIA/AACAvwAAgD8AAIA/AACAvwAAgL8AAIC/AACAvwAAgL8AAIC/AACAvwAAgL8AAIC/AACAvwAAgD8AAIC/AACAvwAAgD8AAIC/AACAvwAAgD8AAIC/AACAPwAAgL8AAIA/AACAPwAAgL8AAIA/AACAPwAAgL8AAIA/AACAPwAAgD8AAIA/AACAPwAAgD8AAIA/AACAPwAAgD8AAIA/AACAPwAAgL8AAIC/AACAPwAAgL8AAIC/AACAPwAAgL8AAIC/AACAPwAAgD8AAIC/AACAPwAAgD8AAIC/AACAPwAAgD8AAIC/AACAvwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIA/AACAvwAAAAAAAACAAAAAAAAAAAAAAIA/AAAAAAAAgD8AAACAAACAvwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIC/AACAvwAAAAAAAACAAAAAAAAAAAAAAIC/AAAAAAAAgD8AAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIA/AACAPwAAAAAAAACAAAAAAAAAAAAAAIA/AAAAAAAAgD8AAACAAACAPwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIC/AACAPwAAAAAAAACAAAAAAAAAAAAAAIC/AAAAAAAAgD8AAACAAACAPwAAAAAAAACAAADAPgAAgD8AAAA+AACAPgAAwD4AAAAAAAAgPwAAgD8AACA/AAAAAAAAYD8AAIA+AADAPgAAQD8AAAA+AAAAPwAAwD4AAEA/AAAgPwAAQD8AACA/AABAPwAAYD8AAAA/AADAPgAAgD4AAMA+AACAPgAAwD4AAIA+AAAgPwAAgD4AACA/AACAPgAAID8AAIA+AADAPgAAAD8AAMA+AAAAPwAAwD4AAAA/AAAgPwAAAD8AACA/AAAAPwAAID8AAAA/AAADAAkAAAAJAAYACAAKABUACAAVABMAFAAXABEAFAARAA4ADQAPAAQADQAEAAIABwASAAwABwAMAAEAFgALAAUAFgAFABAAAACAvwAAgL8AAIA/AACAvwAAgL8AAIA/AACAvwAAgL8AAIA/AACAvwAAgD8AAIA/AACAvwAAgD8AAIA/AACAvwAAgD8AAIA/AACAvwAAgL8AAIC/AACAvwAAgL8AAIC/AACAvwAAgL8AAIC/AACAvwAAgD8AAIC/AACAvwAAgD8AAIC/AACAvwAAgD8AAIC/AACAPwAAgL8AAIA/AACAPwAAgL8AAIA/AACAPwAAgL8AAIA/AACAPwAAgD8AAIA/AACAPwAAgD8AAIA/AACAPwAAgD8AAIA/AACAPwAAgL8AAIC/AACAPwAAgL8AAIC/AACAPwAAgL8AAIC/AACAPwAAgD8AAIC/AACAPwAAgD8AAIC/AACAPwAAgD8AAIC/AACAvwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIA/AACAvwAAAAAAAACAAAAAAAAAAAAAAIA/AAAAAAAAgD8AAACAAACAvwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIC/AACAvwAAAAAAAACAAAAAAAAAAAAAAIC/AAAAAAAAgD8AAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIA/AACAPwAAAAAAAACAAAAAAAAAAAAAAIA/AAAAAAAAgD8AAACAAACAPwAAAAAAAACAAAAAAAAAgL8AAACAAAAAAAAAAAAAAIC/AACAPwAAAAAAAACAAAAAAAAAAAAAAIC/AAAAAAAAgD8AAACAAACAPwAAAAAAAACAAADAPgAAgD8AAAA+AACAPgAAwD4AAAAAAAAgPwAAgD8AACA/AAAAAAAAYD8AAIA+AADAPgAAQD8AAAA+AAAAPwAAwD4AAEA/AAAgPwAAQD8AACA/AABAPwAAYD8AAAA/AADAPgAAgD4AAMA+AACAPgAAwD4AAIA+AAAgPwAAgD4AACA/AACAPgAAID8AAIA+AADAPgAAAD8AAMA+AAAAPwAAwD4AAAA/AAAgPwAAAD8AACA/AAAAPwAAID8AAAA/"
        }
    ]
}