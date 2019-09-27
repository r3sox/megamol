#include "stdafx.h"

#include "glTFFileLoader.h"

#include "mmcore/param/FilePathParam.h"

#ifndef TINYGLTF_IMPLEMENTATION
#    define TINYGLTF_IMPLEMENTATION
#endif // !TINYGLTF_IMPLEMENTATION
#ifndef STB_IMAGE_IMPLEMENTATION
#    define STB_IMAGE_IMPLEMENTATION
#endif // !STB_IMAGE_IMPLEMENTATION
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#    define STB_IMAGE_WRITE_IMPLEMENTATION
#endif // !STB_IMAGE_WRITE_IMPLEMENTATION

#include "tiny_gltf.h"

megamol::mesh::GlTFFileLoader::GlTFFileLoader()
    : core::Module()
    , m_glTFFilename_slot("glTF filename", "The name of the gltf file to load")
    , m_gltf_slot("CallGlTFData", "The slot publishing the loaded data")
    , m_gltf_hash(0)
    , m_mesh_slot("CallMeshData", "The slot providing access to internal mesh data")
    , m_mesh_hash(0) {
    this->m_gltf_slot.SetCallback(CallGlTFData::ClassName(), "GetData", &GlTFFileLoader::getGltfDataCallback);
    this->m_gltf_slot.SetCallback(CallGlTFData::ClassName(), "GetMetaData", &GlTFFileLoader::getGltfDataCallback);
    this->MakeSlotAvailable(&this->m_gltf_slot);

    this->m_mesh_slot.SetCallback(CallMesh::ClassName(), "GetData", &GlTFFileLoader::getMeshDataCallback);
    this->m_mesh_slot.SetCallback(CallMesh::ClassName(), "GetMetaData", &GlTFFileLoader::getMeshDataCallback);
    this->MakeSlotAvailable(&this->m_mesh_slot);

    this->m_glTFFilename_slot << new core::param::FilePathParam("");
    this->MakeSlotAvailable(&this->m_glTFFilename_slot);
}

megamol::mesh::GlTFFileLoader::~GlTFFileLoader() { this->Release(); }

bool megamol::mesh::GlTFFileLoader::create(void) {

    this->m_mesh_collection = std::make_shared<MeshDataAccessCollection>();

    return true;
}

bool megamol::mesh::GlTFFileLoader::getGltfDataCallback(core::Call& caller) {
    CallGlTFData* cd = dynamic_cast<CallGlTFData*>(&caller);

    if (cd == NULL) return false;

    checkAndLoadGltfModel();
    cd->setMetaData({m_gltf_hash});
    cd->setData(m_gltf_model);

    return true;
}

//bool megamol::mesh::GlTFFileLoader::getGltfMetaDataCallback(core::Call& caller) {
//    CallGlTFData* cd = dynamic_cast<CallGlTFData*>(&caller);
//    
//    if (cd == NULL) return false;
//    
//    cd->setMetaData({m_gltf_hash});
//
//    return true;
//}

bool megamol::mesh::GlTFFileLoader::getMeshDataCallback(core::Call& caller) {
    CallMesh* cm = dynamic_cast<CallMesh*>(&caller);

    if (cm == NULL) return false;

    std::shared_ptr<MeshDataAccessCollection> mesh_collection(nullptr);

    if (cm->getData() == nullptr) {
        // no incoming mesh -> use your own mesh storage
        mesh_collection = this->m_mesh_collection;
        cm->setData(mesh_collection);
    } else {
        // incoming mesh -> use it (delete local?)
        mesh_collection = cm->getData();
    }

    //m_bbox[0] = std::numeric_limits<float>::max();
    //m_bbox[1] = std::numeric_limits<float>::max();
    //m_bbox[2] = std::numeric_limits<float>::max();
    //m_bbox[3] = std::numeric_limits<float>::min();
    //m_bbox[4] = std::numeric_limits<float>::min();
    //m_bbox[5] = std::numeric_limits<float>::min();

    checkAndLoadGltfModel();
    auto model = m_gltf_model;

    if (model == nullptr) return false;

    for (size_t mesh_idx = 0; mesh_idx < model->meshes.size(); mesh_idx++) {

        auto primitive_cnt = model->meshes[mesh_idx].primitives.size();

        for (size_t primitive_idx = 0; primitive_idx < primitive_cnt; ++primitive_idx) {

            std::vector<MeshDataAccessCollection::VertexAttribute> mesh_attributes;
            MeshDataAccessCollection::IndexData mesh_indices;

            auto& indices_accessor = model->accessors[model->meshes[mesh_idx].primitives[primitive_idx].indices];
            auto& indices_bufferView = model->bufferViews[indices_accessor.bufferView];
            auto& indices_buffer = model->buffers[indices_bufferView.buffer];

            mesh_indices.byte_size = (indices_accessor.count * indices_accessor.ByteStride(indices_bufferView));
            mesh_indices.data = reinterpret_cast<uint8_t*>(
                indices_buffer.data.data() + indices_bufferView.byteOffset + indices_accessor.byteOffset);
            mesh_indices.type = MeshDataAccessCollection::covertToValueType(indices_accessor.componentType);

            auto& vertex_attributes = model->meshes[mesh_idx].primitives[primitive_idx].attributes;
            for (auto attrib : vertex_attributes) {
                auto& vertexAttrib_accessor = model->accessors[attrib.second];
                auto& vertexAttrib_bufferView = model->bufferViews[vertexAttrib_accessor.bufferView];
                auto& vertexAttrib_buffer = model->buffers[vertexAttrib_bufferView.buffer];

                MeshDataAccessCollection::AttributeSemanticType attrib_semantic;

                if (attrib.first == "POSITION"){
                    attrib_semantic = MeshDataAccessCollection::AttributeSemanticType::POSITION;
                } 
                else if (attrib.first == "NORMAL"){
                    attrib_semantic = MeshDataAccessCollection::AttributeSemanticType::NORMAL;
                } 
                else if (attrib.first == "TANGENT") {
                    attrib_semantic = MeshDataAccessCollection::AttributeSemanticType::TANGENT;
                } 
                else if (attrib.first == "TEXCOORD") {
                    attrib_semantic = MeshDataAccessCollection::AttributeSemanticType::TEXCOORD;
                }

                mesh_attributes.emplace_back(
                    MeshDataAccessCollection::VertexAttribute{
                        reinterpret_cast<uint8_t*>(vertexAttrib_buffer.data.data() + vertexAttrib_bufferView.byteOffset + vertexAttrib_accessor.byteOffset),
                        (vertexAttrib_accessor.count * vertexAttrib_accessor.ByteStride(vertexAttrib_bufferView)),
                        static_cast<unsigned int>(vertexAttrib_accessor.type),
                        MeshDataAccessCollection::covertToValueType(vertexAttrib_accessor.componentType),
                        0,
                        0,
                        attrib_semantic
                    }
                );
            }

            mesh_collection->addMesh(mesh_attributes, mesh_indices);


            auto max_data =
                model->accessors[model->meshes[mesh_idx].primitives[primitive_idx].attributes.find("POSITION")->second]
                    .maxValues;
            auto min_data =
                model->accessors[model->meshes[mesh_idx].primitives[primitive_idx].attributes.find("POSITION")->second]
                    .minValues;

            //m_bbox[0] = std::min(m_bbox[0], static_cast<float>(min_data[0]));
            //m_bbox[1] = std::min(m_bbox[1], static_cast<float>(min_data[1]));
            //m_bbox[2] = std::min(m_bbox[2], static_cast<float>(min_data[2]));
            //m_bbox[3] = std::max(m_bbox[3], static_cast<float>(max_data[0]));
            //m_bbox[4] = std::max(m_bbox[4], static_cast<float>(max_data[1]));
            //m_bbox[5] = std::max(m_bbox[5], static_cast<float>(max_data[2]));
        }
    }


    cm->setMetaData({m_mesh_hash});
    cm->setData(m_mesh_collection);

    return true;
}

//bool megamol::mesh::GlTFFileLoader::getMeshDataMetaCallback(core::Call& caller) { 
//    
//    return false;
//}

void megamol::mesh::GlTFFileLoader::checkAndLoadGltfModel() {

    if (this->m_glTFFilename_slot.IsDirty()) {
        m_glTFFilename_slot.ResetDirty();

        auto vislib_filename = m_glTFFilename_slot.Param<core::param::FilePathParam>()->Value();
        std::string filename(vislib_filename.PeekBuffer());

        m_gltf_model = std::make_shared<tinygltf::Model>();
        tinygltf::TinyGLTF loader;
        std::string err;
        std::string war;

        bool ret = loader.LoadASCIIFromFile(&*m_gltf_model, &err, &war, filename);
        if (!err.empty()) {
            vislib::sys::Log::DefaultLog.WriteError("Err: %s\n", err.c_str());
        }

        if (!ret) {
            vislib::sys::Log::DefaultLog.WriteError("Failed to parse glTF\n");
        }

        ++m_gltf_hash;
        ++m_mesh_hash;
    }
}

void megamol::mesh::GlTFFileLoader::release() {
    // intentionally empty ?
}
