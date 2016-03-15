/**
 *   Copyright 2016, Quickstep Research Group, Computer Sciences Department,
 *     University of Wisconsin—Madison.
 *
 *   Licensed under the Apache License, Version 2.0 (the "License");
 *   you may not use this file except in compliance with the License.
 *   You may obtain a copy of the License at
 *
 *       http://www.apache.org/licenses/LICENSE-2.0
 *
 *   Unless required by applicable law or agreed to in writing, software
 *   distributed under the License is distributed on an "AS IS" BASIS,
 *   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *   See the License for the specific language governing permissions and
 *   limitations under the License.
 **/

#ifndef QUICKSTEP_CATALOG_INDEX_SCHEME_HPP_
#define QUICKSTEP_CATALOG_INDEX_SCHEME_HPP_

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "catalog/Catalog.pb.h"
#include "catalog/CatalogTypedefs.hpp"
#include "storage/StorageBlockInfo.hpp"
#include "threading/SpinSharedMutex.hpp"
#include "types/TypedValue.hpp"
#include "types/operations/comparisons/Comparison.hpp"
#include "types/operations/comparisons/LessComparison.hpp"
#include "utility/Macros.hpp"

#include "glog/logging.h"

namespace quickstep {

/** \addtogroup Catalog
 *  @{
 */

/**
 * @brief The Index Scheme class which stores the information about
 *        various indicies defined for a particular relation.
 **/
class IndexScheme {
 public:
  /**
   * @brief Constructor.
   **/
  IndexScheme() {
  }

  /**
   * @brief Virtual destructor.
   **/
  ~IndexScheme() {
  }

  /**
   * @brief Reconstruct an Index Scheme from its serialized
   *        Protocol Buffer form.
   *
   * @param proto The Protocol Buffer serialization of a Index Scheme,
   *              previously produced by getProto().
   * @return The deserialied index scheme object.
   **/
  static IndexScheme* ReconstructFromProto(const serialization::IndexScheme &proto);

  /**
   * @brief Check whether a serialization::IndexScheme is fully-formed and
   *        all parts are valid.
   *
   * @param proto A serialized Protocol Buffer representation of a
   *              IndexScheme, originally generated by getProto().
   * @return Whether proto is fully-formed and valid.
   **/
  static bool ProtoIsValid(const serialization::IndexScheme &proto);

  /**
   * @brief Serialize the Index Scheme as Protocol Buffer.
   *
   * @return The Protocol Buffer representation of Index Scheme.
   **/
  serialization::IndexScheme getProto() const;

  /**
   * @brief Get the number of indices for the relation.
   *
   * @return The number of indices defined for the relation.
   **/
  inline const std::size_t getNumIndices() const {
    return index_map_.size();
  }

  /**
   * @brief Check whether an index with the given exists or not.
   *
   * @param index_name Name of the index to be checked.
   * @return Whether the index exists or not.
   **/
  bool hasIndexWithName(const std::string &index_name) const {
    return index_map_.find(index_name) != index_map_.end();
  }

  /**
   * @brief Check whether an index with the given description
   *        containing the same attribute id and index type
   *        exists or not in the index map.
   *
   * @param index_descripton Index Description to check against.
   * @return Whether a similar index description was already defined or not.
   **/
  bool hasIndexWithDescription(const IndexSubBlockDescription &index_description) const {
    // Iterate through every vector of descriptions corresponding to all keys in the index map.
    for (auto cit = index_map_.cbegin(); cit != index_map_.cend(); ++cit) {
      const std::vector<IndexSubBlockDescription> &index_descriptions = cit->second;
      for (auto index_description_it = index_descriptions.cbegin();
           index_description_it != index_descriptions.cend();
           ++index_description_it) {
        // Check if the stored description matches as the given description.
        if (areIndexDescriptionsSimilar(*index_description_it, index_description)) {
          return true;
        }
      }
    }
    return false;
  }

  /**
   * @brief Check whether two index descriptions are similar or not.
   *        Two index descriptions are similar if they have same
   *        attributes ids and same index type.
   *
   * @param desc_a First index description.
   * @param desc_b Second index description.
   * @return Whether the two index_descriptions are similar or not.
   **/
  bool areIndexDescriptionsSimilar(const IndexSubBlockDescription &desc_a,
                                   const IndexSubBlockDescription &desc_b) const {
    if (desc_a.sub_block_type() != desc_b.sub_block_type()) {
      return false;
    }
    // TODO(@ssaurabh): add condition to check for matching attribute ids for both indices
    return true;
  }

  /**
   * @brief Adds a new index entry to the index map.
   * @warning Must call before hasIndexWithName() and hasIndexWithDescription().
   *
   * @param index_name The name of index to add (key)
   * @param index_description The index description for this index (value)
   **/
  bool addIndexMapEntry(const std::string &index_name,
                        const std::vector<IndexSubBlockDescription> &index_descriptions) {
    if (index_map_.find(index_name) != index_map_.end()) {
      return false;  // index_name is already present!
    }
    // Value for this index_map key is the list of index descriptions provided.
    std::vector<IndexSubBlockDescription> index_sub_block_descriptions;
    for (std::size_t i = 0; i < index_descriptions.size(); ++i) {
      // Make a copy of the index_description before putting it in the map.
      index_sub_block_descriptions.emplace_back(index_descriptions[i]);
    }
    index_map_[index_name] = index_sub_block_descriptions;
    return true;
  }

 private:
  // A map of index names to their index description.
  std::unordered_map<std::string, std::vector<IndexSubBlockDescription>> index_map_;

 private:
  DISALLOW_COPY_AND_ASSIGN(IndexScheme);
};
/** @} */

}  // namespace quickstep

#endif  // QUICKSTEP_CATALOG_INDEX_SCHEME_HPP_
