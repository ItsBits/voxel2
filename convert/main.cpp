#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <memory>
#include <unordered_map>
#include "../src/Region.hpp"

struct Model {
    int32_t x, y, z;
    std::vector<int8_t> v;
};

Model getModel(const char * file_name) {
    std::ifstream file{ file_name, std::ifstream::in | std::ifstream::binary | std::ifstream::ate };
    if (!file.good()) return {};
    const auto length = file.tellg();
    file.seekg(0, std::ifstream::beg);
    std::vector<char> buffer;
    buffer.resize(length);
    file.read(buffer.data(), length);
    if (!file.good()) return {};
    file.close();

    // find voxel data in file
    bool valid = true;
    size_t i = 0;
    valid = valid && buffer.at(i++) == 'V';
    valid = valid && buffer.at(i++) == 'O';
    valid = valid && buffer.at(i++) == 'X';
    valid = valid && buffer.at(i++) == ' ';
    valid = valid && buffer.at(i++) == -106; 
    valid = valid && buffer.at(i++) == 0; 
    valid = valid && buffer.at(i++) == 0; 
    valid = valid && buffer.at(i++) == 0;
    valid = valid && buffer.at(i++) == 'M';
    valid = valid && buffer.at(i++) == 'A';
    valid = valid && buffer.at(i++) == 'I';
    valid = valid && buffer.at(i++) == 'N'; 
    valid = valid && buffer.at(i++) == 0; 
    valid = valid && buffer.at(i++) == 0; 
    valid = valid && buffer.at(i++) == 0; 
    valid = valid && buffer.at(i++) == 0;
    i += 4; // don't care (eventhough it might make the file technically invalid if the value is not correct)
    if (!valid) return {};

    struct ChunkMeta {
        char id[4];
        int32_t content_size;
        int32_t children_size;
    };

    auto getChunkMeta = [&buffer] (size_t & i) -> ChunkMeta {
        ChunkMeta chunk_meta;
        chunk_meta.content_size = 0;
        chunk_meta.children_size = 0;
        for (size_t j = 0; j < 4; ++j)
            chunk_meta.id[j] = buffer.at(i++);
        // little endian
        for (size_t j = 0; j < 4; ++j)
            chunk_meta.content_size |= (int32_t(buffer.at(i++)) & 0xff) << j * 8;
        for (size_t j = 0; j < 4; ++j)
            chunk_meta.children_size |= (int32_t(buffer.at(i++)) & 0xff) << j * 8;
        return chunk_meta;
    };

    while (true) {
        ChunkMeta chunk_meta = getChunkMeta(i);
        if (std::strncmp(chunk_meta.id, "SIZE", 4) != 0) {
            i += chunk_meta.content_size + chunk_meta.children_size;
        } else {
            break;
        }
    }

    // x, y, z
    int32_t model_size[3]{ 0, 0, 0 };
    for (size_t k = 0; k < 3; ++k)
        for (size_t j = 0; j < 4; ++j)
            model_size[k] |= (int32_t(buffer.at(i++)) & 0xff) << j * 8;
    const auto chunk_meta = getChunkMeta(i);
    if (std::strncmp(chunk_meta.id, "XYZI", 4) != 0)
        return {};
    int32_t number_of_voxels = 0;
    for (size_t j = 0; j < 4; ++j)
        number_of_voxels |= (int32_t(buffer.at(i++)) & 0xff) << j * 8;

    std::vector<int8_t> data;
    data.resize(model_size[0] * model_size[1] * model_size[2]);
    std::fill(std::begin(data), std::end(data), 0);
    for (int32_t j = 0; j < number_of_voxels; ++j) {
        const int8_t x = buffer.at(i++);
        const int8_t y = buffer.at(i++);
        const int8_t z = buffer.at(i++);
        const int8_t v = buffer.at(i++);
        if (x < 0 || x >= model_size[0] || y < 0 || y >= model_size[1] || z < 0 || z >= model_size[2])
            return {};
        data[z * model_size[1] * model_size[0] + y * model_size[0] + x] = v;
    }
    Model model;
    model.x = model_size[0];
    model.y = model_size[1];
    model.z = model_size[2];
    model.v = std::move(data);
    return std::move(model);
}

struct Vec { int64_t x, y, z; };
Vec floorMod(Vec a, Vec b) {
    Vec p;
    p.x = (a.x % b.x + b.x) % b.x;
    p.y = (a.y % b.y + b.y) % b.y;
    p.z = (a.z % b.z + b.z) % b.z;
    return p;
}
Vec floorDiv(Vec a, Vec b) {
    Vec p;
    p.x = (a.x + (a.x < 0)) / b.x - (a.x < 0);
    p.y = (a.y + (a.y < 0)) / b.y - (a.y < 0);
    p.z = (a.z + (a.z < 0)) / b.z - (a.z < 0);
    return p;
}
size_t toIndex(Vec a, Vec b) {
    return (a.z * b.y + a.y) * b.x + a.x;
}
size_t posToIndex(Vec a, Vec b) {
    return toIndex(floorMod(a, b), b);
}

struct Dec { int64_t bi; int64_t ci; Vec c; Vec r; };
Dec toRegionChunkBlock(Vec p, Vec c, Vec r) {
    Dec d;
    d.bi = posToIndex(p, c);
    d.ci = posToIndex(floorDiv(p, c), r);
    d.c = floorDiv(p, c);
    d.r = floorDiv(floorDiv(p, c), r);
    return d;
}

void reverseYZ(Model & model) {
    Vec i;
    std::vector<int8_t> new_voxels;
    new_voxels.resize(model.v.size());
    if (!(model.z * model.y * model.x == model.v.size())) throw 0;
    for (i.z = 0; i.z < model.z; i.z++)
        for (i.y = 0; i.y < model.y; i.y++)
            for (i.x = 0; i.x < model.x; i.x++) {
                new_voxels.at(toIndex({ i.x, i.z, i.y }, { model.x, model.z, model.y })) =
                    model.v.at(toIndex({ i.x, i.y, i.z }, { model.x, model.y, model.z }));
            }
    const auto tmp = model.y;
    model.y = model.z;
    model.z = tmp;
    model.v = std::move(new_voxels);
}

void toRegions(const Model & model) {
    static constexpr Vec CHUNK_SIZE{ cfg::CHUNK_SIZE.x, cfg::CHUNK_SIZE.y, cfg::CHUNK_SIZE.z };
    static constexpr Vec REGION_SIZE{cfg::REGION_SIZE.x, cfg::REGION_SIZE.y, cfg::REGION_SIZE.z };
    static constexpr size_t CHUNK_VOLUME{ CHUNK_SIZE.x * CHUNK_SIZE.y * CHUNK_SIZE.z };
    static constexpr size_t REGION_VOLUME{ REGION_SIZE.x * REGION_SIZE.y * REGION_SIZE.z };
    using Block = int8_t;
    // good enough I guess
    struct KeyHash { size_t operator () (const Vec & v) const { return v.x ^ v.y ^ v.z; } };
    struct KeyEqual { bool operator () (const Vec & a, const Vec & b) const { return a.x == b.x && a.y == b.y && a.z == b.z; } };
    std::unordered_map<Vec, std::array<Block, CHUNK_VOLUME>, KeyHash, KeyEqual> chunk_map;
    Vec i;
    if (!(model.z * model.y * model.x == model.v.size())) throw 0;
    for (i.z = 0; i.z < model.z; i.z++)
        for (i.y = 0; i.y < model.y; i.y++)
            for (i.x = 0; i.x < model.x; i.x++) {
                const auto d = toRegionChunkBlock(i, CHUNK_SIZE, REGION_SIZE);
                if (chunk_map.find(d.c) == chunk_map.end()) {
                    auto j = chunk_map.insert({ d.c, {} });
                    std::fill(j.first->second.begin(), j.first->second.end(), 0);
                }
                chunk_map[d.c].at(d.bi) = model.v.at(toIndex(i, Vec{ model.x, model.y, model.z }));
            }

    std::unordered_map<Vec, Region, KeyHash, KeyEqual> region_map;
    for (const auto & chunk : chunk_map) {
        std::cout << chunk.first.x << ' ';
        std::cout << chunk.first.y << ' ';
        std::cout << chunk.first.z << std::endl;
        const auto region_position = floorDiv(chunk.first, REGION_SIZE);
        const auto chunk_index = posToIndex(chunk.first, REGION_SIZE);
            if (region_map.find(region_position) == region_map.end()) {
                auto j = region_map.emplace(region_position, glm::tvec3<cfg::Coord>{ region_position.x, region_position.y, region_position.z });
            }
            region_map.find(region_position)->second.saveChunk(chunk_index, (const cfg::Block *)chunk.second.data());
    }
}

int main(int argc, char * argv[]) {
    // open file and load to memory
    if (argc != 2) {
        std::cout << "Usage: [program] [source_file_name]" << std::endl;
        return 1;
    }
    const auto file_name = argv[1];

    try {
        auto model = getModel(file_name);
        if (model.v.size() == 0) throw 0;
        reverseYZ(model);
        toRegions(model);
    } catch (...) {
        std::cout << "Issue with file " << file_name << std::endl;
        return 1;
    }

    return 0;
}