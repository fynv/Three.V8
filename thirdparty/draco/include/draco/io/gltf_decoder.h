// Copyright 2018 The Draco Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
#ifndef DRACO_IO_GLTF_DECODER_H_
#define DRACO_IO_GLTF_DECODER_H_

#include "draco/draco_features.h"

#ifdef DRACO_TRANSCODER_SUPPORTED

#include <map>
#include <string>
#include <vector>

#include "draco/core/decoder_buffer.h"
#include "draco/core/status.h"
#include "draco/core/status_or.h"
#include "draco/io/file_utils.h"
#include "draco/io/texture_io.h"
#include "draco/mesh/mesh.h"
#include "draco/mesh/triangle_soup_mesh_builder.h"
#include "draco/scene/scene.h"
#include "third_party/tinygltf/tiny_gltf.h"

namespace draco {

// Decodes a glTF file and returns a draco::Mesh. All of the |mesh|'s attributes
// will be merged into one draco::Mesh
class GltfDecoder {
 public:
  GltfDecoder();

  // Decodes a glTF file stored in the input |file_name| or |buffer| to a Mesh.
  // The second form returns a vector of files used as input to the mesh during
  // the decoding process. Returns nullptr when decode fails.
  StatusOr<std::unique_ptr<Mesh>> DecodeFromFile(const std::string &file_name);
  StatusOr<std::unique_ptr<Mesh>> DecodeFromFile(
      const std::string &file_name, std::vector<std::string> *mesh_files);
  StatusOr<std::unique_ptr<Mesh>> DecodeFromBuffer(DecoderBuffer *buffer);

  // Decodes a glTF file stored in the input |file_name| or |buffer| to a Scene.
  // The second form returns a vector of files used as input to the scene during
  // the decoding process. Returns nullptr if the decode fails.
  StatusOr<std::unique_ptr<Scene>> DecodeFromFileToScene(
      const std::string &file_name);
  StatusOr<std::unique_ptr<Scene>> DecodeFromFileToScene(
      const std::string &file_name, std::vector<std::string> *scene_files);
  StatusOr<std::unique_ptr<Scene>> DecodeFromBufferToScene(
      DecoderBuffer *buffer);

 private:
  // Loads |file_name| into |gltf_model_|. Fills |input_files| with paths to all
  // input files when non-null.
  Status LoadFile(const std::string &file_name,
                  std::vector<std::string> *input_files);

  // Loads |gltf_model_| from |buffer| in GLB format.
  Status LoadBuffer(const DecoderBuffer &buffer);

  // Builds mesh from |gltf_model_|.
  StatusOr<std::unique_ptr<Mesh>> BuildMesh();

  // Checks |gltf_model_| for unsupported features. If |gltf_model_| contains
  // unsupported features then the function will return with a status code of
  // UNSUPPORTED_FEATURE.
  Status CheckUnsupportedFeatures();

  // Decodes a glTF Node as well as any child Nodes. If |node| contains a mesh
  // it will process all of the mesh's primitives.
  Status DecodeNode(int node_index, const Eigen::Matrix4d &parent_matrix);

  // Decodes the number of entries in the first attribute of a given glTF
  // |primitive|. Note that all attributes have the same entry count according
  // to glTF 2.0 spec.
  StatusOr<int> DecodePrimitiveAttributeCount(
      const tinygltf::Primitive &primitive) const;

  // Decodes the number of indices in a given glTF |primitive|. If primitive's
  // indices property is not defined, the index count is implied from the entry
  // count of a primitive attribute.
  StatusOr<int> DecodePrimitiveIndicesCount(
      const tinygltf::Primitive &primitive) const;

  // Decodes indices property of a given glTF |primitive|. If primitive's
  // indices property is not defined, the indices are generated based on entry
  // count of a primitive attribute.
  StatusOr<std::vector<uint32_t>> DecodePrimitiveIndices(
      const tinygltf::Primitive &primitive) const;

  // Decodes a glTF Primitive. All of the |primitive|'s attributes will be
  // merged into the draco::Mesh output if they are of the same type that
  // already has been decoded.
  Status DecodePrimitive(const tinygltf::Primitive &primitive,
                         const Eigen::Matrix4d &transform_matrix);

  // Sums the number of elements per attribute for |node|'s mesh and any of
  // |node|'s children. Fills out the material index map.
  Status NodeGatherAttributeAndMaterialStats(const tinygltf::Node &node);

  // Sums the number of elements per attribute for all of the meshes and
  // primitives.
  Status GatherAttributeAndMaterialStats();

  // Sums the attribute counts into total_attribute_counts_.
  void SumAttributeStats(const std::string &attribute_name, int count);

  // Checks that all the same glTF attribute types in different meshes and
  // primitives contain the same characteristics.
  Status CheckTypes(const std::string &attribute_name, int component_type,
                    int type);

  // Accumulates the number of elements per attribute for |primitive|.
  Status AccumulatePrimitiveStats(const tinygltf::Primitive &primitive);

  // Adds all of the attributes from the glTF file to a Draco mesh.
  // GatherAttributeAndMaterialStats() must be called before this function. The
  // GeometryAttribute::MATERIAL attribute will be created only if the glTF file
  // contains more than one material.
  Status AddAttributesToDracoMesh();

  // Copies the tangent attribute data from |accessor| and adds it to a Draco
  // mesh. This function will transform all of the data by |transform_matrix|
  // and then normalize before adding the data to the Draco mesh.
  // |indices_data| is the indices data from the glTF file. |att_id| is the
  // attribute id of the tangent attribute in the Draco mesh. |number_of_faces|
  // This is the number of faces this function will process. |reverse_winding|
  // if set will change the orientation of the data.
  Status AddTangentToMeshBuilder(const tinygltf::Accessor &accessor,
                                 const std::vector<uint32_t> &indices_data,
                                 int att_id, int number_of_faces,
                                 const Eigen::Matrix4d &transform_matrix,
                                 bool reverse_winding,
                                 TriangleSoupMeshBuilder *mb);

  // Copies the texture coordinate attribute data from |accessor| and adds it to
  // a Draco mesh. This function will flip the data on the horizontal axis as
  // Draco meshes store the texture coordinates differently than glTF.
  // |indices_data| is the indices data from the glTF file. |att_id| is the
  // attribute id of the texture coordinate attribute in the Draco mesh.
  // |number_of_faces| This is the number of faces this function will process.
  // |reverse_winding| if set will change the orientation of the data.
  Status AddTexCoordToMeshBuilder(const tinygltf::Accessor &accessor,
                                  const std::vector<uint32_t> &indices_data,
                                  int att_id, int number_of_faces,
                                  bool reverse_winding,
                                  TriangleSoupMeshBuilder *mb);

  // Copies the attribute data from |accessor| and adds it to a Draco mesh.
  // This function will transform all of the data by |transform_matrix| before
  // adding the data to the Draco mesh. |indices_data| is the indices data
  // from the glTF file. |att_id| is the attribute id of the attribute in the
  // Draco mesh. |number_of_faces| This is the number of faces this function
  // will process. |normalize| if set will normalize all of the vector data
  // after transformation. |reverse_winding| if set will change the orientation
  // of the data.
  Status AddTransformedDataToMeshBuilder(
      const tinygltf::Accessor &accessor,
      const std::vector<uint32_t> &indices_data, int att_id,
      int number_of_faces, const Eigen::Matrix4d &transform_matrix,
      bool normalize, bool reverse_winding, TriangleSoupMeshBuilder *mb);

  // Sets values in |data| into the mesh builder |mb| for |att_id|.
  // |reverse_winding| if set will change the orientation of the data.
  template <typename T>
  void SetValuesPerFace(const std::vector<uint32_t> &indices_data, int att_id,
                        int number_of_faces, const std::vector<T> &data,
                        bool reverse_winding, TriangleSoupMeshBuilder *mb);

  // Adds the attribute data in |accessor| to |mb| for unique attribute
  // |att_id|. |indices_data| is the mesh's indices data. |reverse_winding| if
  // set will change the orientation of the data.
  Status AddAttributeDataByTypes(const tinygltf::Accessor &accessor,
                                 const std::vector<uint32_t> &indices_data,
                                 int att_id, int number_of_faces,
                                 bool reverse_winding,
                                 TriangleSoupMeshBuilder *mb);

  // Adds the textures to |owner|.
  template <typename T>
  Status CopyTextures(T *owner);

  // Adds the materials to |mesh|.
  Status AddMaterialsToDracoMesh(Mesh *mesh);

  // Adds the material data for the GeometryAttribute::MATERIAL attribute to the
  // Draco mesh.
  template <typename T>
  Status AddMaterialDataToMeshBuilder(T material_value, int number_of_faces);

  // Checks if the glTF file contains a texture. If there is a texture, this
  // function will read the texture data and add it to the Draco |material|. If
  // there is no texture, this function will return OkStatus(). |texture_info|
  // is the data structure containing information about the texture in the glTF
  // file. |type| is the type of texture defined by Draco. This is not the same
  // as the texture coordinate attribute id.
  Status CheckAndAddTextureToDracoMaterial(
      int texture_index, int tex_coord_attribute_index,
      const tinygltf::ExtensionMap &tex_info_ext, Material *material,
      TextureMap::Type type);

  // Decode glTF file to scene.
  Status DecodeGltfToScene();

  // Decode glTF animations into a scene. All of the glTF nodes must be decoded
  // to the scene before this function is called.
  Status AddAnimationsToScene();

  // Decode glTF node into a Draco scene. |parent_index| is the index of the
  // parent node. If |node| is a root node set |parent_index| to
  // |kInvalidSceneNodeIndex|.
  Status DecodeNodeForScene(int node_index, SceneNodeIndex parent_index);

  // Decode glTF primitive into a Draco scene.
  Status DecodePrimitiveForScene(const tinygltf::Primitive &primitive,
                                 MeshGroup *mesh_group);

  // Adds an attribute of type |attribute_name| to |mb|. Returns the
  // attribute id.
  StatusOr<int> AddAttribute(const std::string &attribute_name,
                             int component_type, int type,
                             TriangleSoupMeshBuilder *mb);

  // Adds an attribute of |attribute_type| to |mb|. Returns the
  // attribute id.
  StatusOr<int> AddAttribute(GeometryAttribute::Type attribute_type,
                             int component_type, int type,
                             TriangleSoupMeshBuilder *mb);

  // Returns true if the KHR_texture_transform extension is set in |extension|.
  // If the KHR_texture_transform extension is set then the values are returned
  // in |transform|.
  StatusOr<bool> CheckKhrTextureTransform(
      const tinygltf::ExtensionMap &extension, TextureTransform *transform);

  // Adds glTF material |input_material_index| to |output_material|.
  Status AddGltfMaterial(int input_material_index, Material *output_material);

  // Adds unlit property from glTF |input_material| to |output_material|.
  void DecodeMaterialUnlitExtension(const tinygltf::Material &input_material,
                                    Material *output_material);

  // Adds sheen properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialSheenExtension(const tinygltf::Material &input_material,
                                      Material *output_material);

  // Adds transmission from glTF |input_material| to |output_material|.
  Status DecodeMaterialTransmissionExtension(
      const tinygltf::Material &input_material, Material *output_material);

  // Adds clearcoat properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialClearcoatExtension(
      const tinygltf::Material &input_material, Material *output_material);

  // Adds volume properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialVolumeExtension(const tinygltf::Material &input_material,
                                       Material *output_material);

  // Adds ior properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialIorExtension(const tinygltf::Material &input_material,
                                    Material *output_material);

  // Adds specular properties from glTF |input_material| to |output_material|.
  Status DecodeMaterialSpecularExtension(
      const tinygltf::Material &input_material, Material *output_material);

  // Decodes a float value with |name| from |object| to |value| and returns true
  // if a well-formed value with such |name| is present.
  static StatusOr<bool> DecodeFloat(const std::string &name,
                                    const tinygltf::Value::Object &object,
                                    float *value);

  // Decodes a 3D vector with |name| from |object| to |value| and returns true
  // if a well-formed vector with such |name| is present.
  static StatusOr<bool> DecodeVector3f(const std::string &name,
                                       const tinygltf::Value::Object &object,
                                       Vector3f *value);

  // Decodes a texture with |name| from |object| and adds it to |material| as a
  // texture map of |type|.
  Status DecodeTexture(const std::string &name, TextureMap::Type type,
                       const tinygltf::Value::Object &object,
                       Material *material);

  // Reads texture with |texture_name| from |container_object| into
  // |texture_info|.
  static Status ParseTextureInfo(
      const std::string &texture_name,
      const tinygltf::Value::Object &container_object,
      tinygltf::TextureInfo *texture_info);

  // Adds the materials to the scene.
  Status AddMaterialsToScene();

  // Adds the skins to the scene.
  Status AddSkinsToScene();

  // Map of glTF Mesh to Draco scene mesh group.
  std::map<int, MeshGroupIndex> gltf_mesh_to_scene_mesh_group_;

  // Data structure that stores the glTF data.
  tinygltf::Model gltf_model_;

  // Path to the glTF file.
  std::string input_file_name_;

  // Class used to build the Draco mesh.
  TriangleSoupMeshBuilder mb_;

  // Next face index used when adding attribute data to the Draco mesh.
  int next_face_id_;

  // Total number of indices from all the meshes and primitives.
  int total_indices_count_;

  // This is the id of the GeometryAttribute::MATERIAL attribute added to the
  // Draco mesh.
  int material_att_id_;

  // Map of glTF attribute name to attribute element counts.
  std::map<std::string, int> total_attribute_counts_;

  // Map of glTF attribute name to attribute component type.
  std::map<std::string, int> attribute_component_type_;

  // Map of glTF attribute name to attribute type.
  std::map<std::string, int> attribute_type_;

  // Map of glTF attribute name to Draco mesh attribute id.
  std::map<std::string, int> attribute_name_to_draco_mesh_attribute_id_;

  // Map of glTF material to Draco material index.
  std::map<int, int> gltf_primitive_material_to_draco_material_;

  // Map of glTF image to Draco textures.
  std::map<int, Texture *> gltf_image_to_draco_texture_;

  std::unique_ptr<Scene> scene_;

  // Map of glTF Node to local store order.
  std::map<int, SceneNodeIndex> gltf_node_to_scenenode_index_;

  // Functionality for deduping primitives on decode.
  struct PrimitiveSignature {
    const tinygltf::Primitive &primitive;
    explicit PrimitiveSignature(const tinygltf::Primitive &primitive)
        : primitive(primitive) {}
    bool operator==(const PrimitiveSignature &signature) const;
    struct Hash {
      size_t operator()(const PrimitiveSignature &signature) const;
    };
  };
  std::unordered_map<PrimitiveSignature, MeshIndex, PrimitiveSignature::Hash>
      gltf_primitive_to_draco_mesh_index_;
};

}  // namespace draco

#endif  // DRACO_TRANSCODER_SUPPORTED
#endif  // DRACO_IO_GLTF_DECODER_H_
