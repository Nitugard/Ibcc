/*
 *  Copyright (C) 2021 by Dragutin Sredojevic
 *  https://www.nitugard.com
 *  All Rights Reserved.
 */


#ifndef IBC_MODEL_H
#define IBC_MODEL_H

#include <array>
#include <vector>
#include <sstream>
#include <fstream>
#include <map>

class Model {
public:

    std::vector<std::array<float, 3>> vertices;
    std::vector<std::array<float, 3>> normals;
    std::vector<std::array<float, 2>> textures;
    std::vector<int> indices;
};

static inline Model loadObj(const std::string &name) {
    Model model;

    std::vector<std::array<float, 3>> temp_vertices; //v x y z
    std::vector<std::array<float, 2>> temp_textures;       //vt x y
    std::vector<std::array<float, 3>> temp_normals;   //vn x y z
    std::vector<int> faces;    //vf v1/vt1/vn1 v2/vt2/vn2 ... vN/vtN/vnN; Index can be negative one that corresponds to not provided

    std::ifstream stream;
    std::string line;

    stream.open(name);
    if (stream.fail())
        throw std::runtime_error("Failed to open obj file: " + name);

    while (std::getline(stream, line)) {
        std::istringstream iss(line);
        std::string code;
        iss >> code;

        if (code == "v") {
            std::array<float, 3> input;
            iss >> input[0] >> input[1] >> input[2];
            temp_vertices.push_back(input);
        } else if (code == "vt") {
            std::array<float, 2> input;
            iss >> input[0] >> input[1];
            temp_textures.push_back(input);
        } else if (code == "vn") {
            std::array<float, 3> input;
            iss >> input[0] >> input[1] >> input[2];
            temp_normals.push_back(input);
        } else if (code == "f") {
            while (!iss.eof()) {
                for (int i = 0; i < 3; ++i) {
                    if (iss.peek() == '\\') {
                        if (i == 0)
                            throw std::runtime_error("vertex data must be provided");

                        iss.ignore(1);
                        faces.push_back(-1);
                    }

                    int a;
                    iss >> a;
                    faces.push_back(a - 1);
                    iss.ignore(1);
                }
            }
        }
    }

    stream.close();
    std::map<std::array<float, 3>, int> vertexMap;
    int indices = 0;
    for (int i = 0; i < faces.size(); i += 3) {
        int v = faces[i];
        int vt = faces[i + 1];
        int vn = faces[i + 2];

        if (auto it = vertexMap.find(temp_vertices[v]); it != vertexMap.end()) {
            model.indices.push_back(it->second);
        } else {
            vertexMap.insert(std::pair(temp_vertices[v], indices));
            model.vertices.push_back(temp_vertices[v]);
            model.indices.push_back(indices);
            model.textures.push_back(temp_textures[vt]);
            model.normals.push_back(temp_normals[vn]);
            indices++;
        }
    }
    return model;
}

#endif //IBC_MODEL_H
